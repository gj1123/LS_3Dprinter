#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include </home/yxc/Documents/marlin_ls2k1000la-master/src/ls2k1000la_driver/ls2k1000la_3dprinter_motor.h>

// 电机转动方向
typedef enum
{
    MOTOR_DIRECTION_REVERSE = 0,            // 反转
    MOTOR_DIRECTION_POSITIVE = 1,           // 正转
}motor_direction_t;

typedef unsigned char BOOL;

// This struct is used when buffering the setup for each linear movement "nominal" values are as specified in the source g-code and may never actually be reached if acceleration management is active.
typedef struct 
{
    unsigned int head_flag;                     // 固定为0xe5e5，用于判断整个结构体内容是否被破坏

    // Fields used by the bresenham algorithm for tracing the line
    // 直线轨迹设定的参数
    unsigned char direction_bits;               // The direction bit set for this block
                                                // 各轴步进电机的方向
    long steps[NUM_AXIS];                       // Step count along each axis
                                                // 各轴步进电机的步数

    BOOL s_cure_used;                           // 是否使用s曲线加减速,(TRUE or FALSE)                     
                                                // TRUE 使用s曲线加减速
                                                // FALSE 恒速运动，速度变化是跳变的形式，这种方式的速度不能太大

    unsigned long timer_period_ns;              // 进给率对应的(用于控制电机速度的)定时器周期，单位ns

    unsigned long step_event_count;             // The number of step events required to complete this block
                                                // 此block的最大的步数
    long accelerate_until_step;                 // The index of the step event on which to stop acceleration
                                                // 加速完成那个step的索引
    long decelerate_after_step;                 // The index of the step event on which to start decelerating
                                                // 开始减速那个step的索引
    long acceleration_rate;                     // The acceleration rate used for acceleration calculation


    // Fields used by the motion planner to manage acceleration
    // float speed_x, speed_y, speed_z, speed_e;         // Nominal mm/sec for each axis
    float nominal_speed_mm_s;                   // The nominal speed for this block in mm/s
                                                // 梯形曲线中匀速部分的速度，单位mm/s
    float entry_speed_mm_s;                     // Entry speed at previous-current junction in mm/s
                                                // 入口速度，刚进入梯形曲线时的速度，单位mm/s
    float max_entry_speed_mm_s;                 // Maximum allowable junction entry speed in mm/s
                                                // 允许的最大入口速度，单位mm/s
    float millimeters_mm;                       // The total travel of this block in mm
                                                // 此blockt移动的距离，单位mm
    float acceleration_mm_s2;                   // acceleration mm/s^2
                                                // 此block的加速度，单位mm/s^2
    unsigned char recalculate_flag;             // Planner flag to recalculate trapezoids on entry junction
                                                // 记录是否重新规划连接速度的flag
    unsigned char nominal_length_flag;          // Planner flag for nominal speed always reached
                                                // 记录nominal_speed是否能达到的flag
    /*
     * Settings for the trapezoid generator
     * 梯形加减速的参数
     *             nominal_rate_step_s
     *             ___________________
     *            /                   \
     *           /                     \
     *          /                       \  final_rate_step_s
     * initial_rate_step_s
     * entry_speed_mm_s
     */  
    unsigned long nominal_rate_step_s;          // The nominal step rate for this block in step_events/s
                                                // 梯形曲线中匀速部分的速度，单位step/s
    unsigned long initial_rate_step_s;          // The jerk-adjusted step rate at start of block
                                                // 梯形曲线中刚开始加速时的速度，单位step/s
    unsigned long final_rate_step_s;            // The minimal rate at exit
                                                // 梯形曲线中结束加速时的速度，单位step/s
    unsigned long acceleration_st_step_s2;      // acceleration steps/s^2
                                                // 梯形曲线中加速部分的加速度，单位step/s^2
                                                // 梯形曲线中减速部分的加速度为此值的负数

    unsigned long fan_speed;                    // 风扇速度
    unsigned char check_endstop;                // 是否检查限位开关状态

    unsigned int end_flag;                      // 固定为0xe5e5，用于判断整个结构体内容是否被破坏
} motor_cmd_block_t;


// 驱动的状态信息
typedef struct {
    // block缓存是否已满
    // 用于应用程序往驱动写block时判断缓存是否已满
    char cmd_block_fifo_full;   // true or false

    // fifo中所有step都已执行
    // fifo总没有block，并且最后一个block也已全部执行完成
    char buffered_steps_executed;   // true or false
} motor_status_t;


#ifndef TRUE
#define TRUE                    (1)
#endif

#ifndef FALSE
#define FALSE                   (0)
#endif

// 通用配置寄存器(通用配置寄存器 0，包括对管脚复用的控制，以及 HDA、USB、PCIE 的一致性、内存控制器、RTC 控制器及 LIO 控制器的配置等)
#define LS2K1000KA_GCR                             (0x1fe00420)

#define LS2K1000KA_PWM_SEL0                        (12)
#define LS2K1000KA_PWM_SEL1                        (13)
#define LS2K1000KA_PWM_SEL2                        (14)
#define LS2K1000KA_PWM_SEL3                        (15)


// PWNn所在gpio
#define LS2K1000LA_PWM0_GPIO20                        (20)
#define LS2K1000LA_PWM1_GPIO21                        (21)
#define LS2K1000LA_PWM2_GPIO22                        (22)
#define LS2K1000LA_PWM3_GPIO23                        (23)


// PWM控制寄存器位定义
#define LS2K1000LA_PWM_CNTR_RST                 (7)     // CNTR计数器清零
#define LS2K1000LA_PWM_INT_SR                   (6)     // 中断状态位
#define LS2K1000LA_PWM_INTEN                    (5)     // 中断使能位
#define LS2K1000LA_PWM_SINGLE                   (4)     // 单脉冲控制位
#define LS2K1000LA_PWM_OE                       (3)     // 脉冲输出使能控制位
#define LS2K1000LA_PWM_CNT_EN                   (0)     // CNTR使能位


// 寄存器偏移
#define REG_PWM_CNTR	        0x00
#define REG_PWM_Low_buffer		0x4
#define REG_PWM_Full_buffer		0x8
#define REG_PWM_CTRL        	0xC

// 脉冲宽度
#define PWM_PULSE_HIGH_WIDTH_NS                 (2*1000)    // 高电平2us
#define PWM_PULSE_LOW_WIDTH_NS                  (2*1000)    // 低电平2us


// 定时器最小定时时长
// 定时时间到后至少需要发送一个高低电平脉冲给步进电机驱动芯片
// 目前，步进电机驱动芯片使用的是a4988，其脉冲的高电平和低电平时间都至少是1us
// 为了留出足够的时间，这里选择定时时间最小为50us
#define MOTOR_TIMER_MIN_TIME_NS                 (50*1000)           // 50us
#define MOTOR_TIMER_MAX_TIME_NS                 (100*1000*1000)     // 100ms
// 42步进电机的空载启动频率为1.5khz，这里选1khz，T=1/1000s=1ms=1000us=1000 000ns
#define MOTOR_TIMER_DEFAULT_TIME_NS             (1000*1000)         // 1ms = 1000us = 1000 000ns

// gcode指令对应block的fifo的大小
// marlin中是16个block，每个block大小为84字节，这里取2的n次幂，所有这里选择2k
#define MOTOR_CMD_BLOCK_FIFO_SIZE               (2*1024)    // 2k
// 结构体中首末两个成员固定为此值，用于判断结构体内容是否被破坏
#define STRUCT_CHECK_FLAG                       (0xe5e5)



// Macros for bit masks
#define BV(n)               (1<<(n))
#define TEST_BIT(n,b)       (((n)&BV(b))!=0)
#define SBI(n,b)            ((n) |= BV(b))
#define CBI(n,b)            ((n) &= ~BV(b))
#define SET_BIT(n,b,value)  (n) ^= ((-value)^(n)) & (BV(b))



static struct platform_3dprinter_motor_data *motor_data = NULL;
//pwm\n  -->  0x1fe220n0
static void __iomem *motor_pwm_base = NULL;
static unsigned long long motor_pwm_clk_rate;       // pwm时钟频率
static struct workqueue_struct *motor_queue;
static struct work_struct motor_work;
static struct kfifo_rec_ptr_1 motor_cmd_block_fifo;     // 存放gcode指令对应的block的fifo
static DEFINE_MUTEX(motor_write_lock);
static volatile int motor_current_block_complete = TRUE;    // 定时器中断下半部用于判断当前block是否执行完成的标记
static volatile int motor_buffered_steps_executed = TRUE;   // fifo中所有step都已执行完成，取值为TRUE or FALSE
// Mechanical endstop with COM to ground and NC to Signal uses "false" here (most common setup).
static unsigned char motor_endstop_inverting[NUM_AXIS] = {FALSE,FALSE,FALSE,TRUE};   // set to true to invert the logic of the endstop.
static unsigned char motor_dir_inverting[NUM_AXIS] = {FALSE, FALSE, FALSE, TRUE};   // 设置TRUE使逻辑反转，比如挤出机E的电机正反转方向与XYZ的相反


// 设置所有电机的方向
// @current_block_p 当前block控制块
static void motor_set_all_direction(motor_cmd_block_t *current_block_p)
{
    #define MOTOR_SET_ONE_DIR(axis) \
        if (motor_dir_inverting[axis] != TEST_BIT(current_block_p->direction_bits, axis)) \
        { \
            gpio_set_value(motor_data[axis].direction_gpio, MOTOR_DIRECTION_POSITIVE); \
        } \
        else \
        { \
            gpio_set_value(motor_data[axis].direction_gpio, MOTOR_DIRECTION_REVERSE); \
        }
    MOTOR_SET_ONE_DIR(X_AXIS);
    MOTOR_SET_ONE_DIR(Y_AXIS);
    MOTOR_SET_ONE_DIR(Z_AXIS);
    MOTOR_SET_ONE_DIR(E_AXIS);

    return ;
}


// 读取指定电机的限位开关的状态
// @motor_index 电机索引
static int motor_get_endstop_status(int motor_index)
{
    // 读取状态
    return gpio_get_value(motor_data[motor_index].endstop_gpio);
}


// 打印所有限位器的状态
static void motor_print_all_endstop_status(void)
{
    printk(KERN_DEBUG "[%s] endstop status, X=%d, Y=%d, Z=%d\n", 
            __FUNCTION__, 
            motor_get_endstop_status(X_AXIS),
            motor_get_endstop_status(Y_AXIS),
            motor_get_endstop_status(Z_AXIS));
}


// 检查限位开关状态
// @current_block_p 当前执行的block
// @step_events_completed_p 当前block已完成的step数
static void motor_update_endstops(motor_cmd_block_t *current_block_p, unsigned long *step_events_completed_p)
{
    unsigned char current_endstop_bits= 0;

    // 判断滑车是否向上滑动
    #define CARRIAGE_MOVE_UP(axis)      (TEST_BIT(current_block_p->direction_bits, (axis)) && (0<current_block_p->steps[axis]))
    // 判断滑车是否向下滑动
    #define CARRIAGE_MOVE_DOWN(axis)    (!TEST_BIT(current_block_p->direction_bits, (axis)) && (0<current_block_p->steps[axis]))

    /*
     * 目前手上的三角洲3D打印机的xyz导轨上方装有限位块
     * xyz导轨上的限位块主要用于电机归零
     *
     * 步骤
     * 读取限位开关所在引脚的状态
     * 判断滑块运行方向是否向上，并且已经接触到限位块了
     */
    #define UPDATE_ENDSTOPS_XYZ(axis)   do { \
        SET_BIT(current_endstop_bits, (axis), (motor_endstop_inverting[axis]!=motor_get_endstop_status(axis))); \
        if (CARRIAGE_MOVE_UP(axis) && TEST_BIT(current_endstop_bits, (axis))) \
        { \
            *step_events_completed_p = current_block_p->step_event_count; \
            return ;\
        } \
    } while(0)
        
    // 检查xyz轴的限位开关
    UPDATE_ENDSTOPS_XYZ(X_AXIS);  // ramps1.4板子上限位开关的接口紧挨着，不能同时把4个限位开关接上取，这里选取y和e用于测试
    UPDATE_ENDSTOPS_XYZ(Y_AXIS);
    UPDATE_ENDSTOPS_XYZ(Z_AXIS);

    return ;
}


// 判断命令block的内容是否被破坏，即结构体前后的标记是否正确
// 返回0为没有被破坏
static int motor_cmd_block_is_err(motor_cmd_block_t *block)
{
    if ((STRUCT_CHECK_FLAG == block->head_flag) && (STRUCT_CHECK_FLAG == block->end_flag))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


// 判断block fifo是否已满
// 返回0为没满，还可以填入至少一个block
static char motor_cmd_block_fifo_is_full(void)
{
    unsigned int available;
    
    // 判断fifo是否有足够的空余空间
    // 保证每次写入能把整个block一次写入
    available = kfifo_avail(&motor_cmd_block_fifo);
    if (available < sizeof(motor_cmd_block_t))
    {
        return TRUE;
    }

    return FALSE;
}


// 获取状态
static void motor_get_status(motor_status_t *status)
{    
    status->cmd_block_fifo_full         = motor_cmd_block_fifo_is_full();
    status->buffered_steps_executed     = motor_buffered_steps_executed;

    return ;
}


// 初始化用于产生步进脉冲的pwm0, pwm1, pwm2
static void motor_pulse_PWMn_init(int PWMn)
{
    unsigned long long tmp = 0;
    unsigned long pulse_high_width_ns = PWM_PULSE_HIGH_WIDTH_NS;
    unsigned long pulse_low_width_ns = PWM_PULSE_LOW_WIDTH_NS;
    unsigned int cntr_reg_data = 0;     // 写入控制寄存器的数据
    unsigned int data = 0;
    void __iomem *reg_base = NULL;
    void __iomem *addr = NULL;

    // 复用GPIO为PWM
    switch (PWMn)
    {
        case LS2K1000LA_PWM_0:
        // 我使用了 ioremap 函数来将物理地址映射到内核虚拟地址空间。这是因为直接访问物理地址在内核中通常是不被允许的，除非你确切知道你在做什么（并且这通常是在非常底层的代码中）。ioremap 会为你处理这些细节。
        // 我添加了 volatile 关键字来声明 addr 指针。这是因为我们可能会通过这个指针来访问可能随时被硬件更改的内存位置。添加 volatile 可以确保编译器不会对这个位置的访问进行优化，从而确保我们的代码能够正确地与硬件交互。
        // 我添加了错误处理来检查 ioremap 是否成功。如果映射失败（例如，因为内存不足或其他原因），我们应该能够检测到这一点并采取相应的行动。
        // 在完成对寄存器的读写操作后，我使用了 iounmap 来释放之前通过 ioremap 获得的映射。这是一个好习惯，因为它可以防止内存泄漏和其他潜在问题。然而，如果你打算在整个驱动程序的生命周期中都保留这个映射（例如，如果你在一个模块的全局范围内保留了它），那么你可能不需要调用 iounmap。但是，请注意，这可能会增加内存使用的风险，并可能导致其他问题。

        // 直接访问物理地址在内核中是不被允许的，使用ioremap映射到虚拟地址
            addr = (void *)ioremap(LS2K1000KA_GCR, sizeof(unsigned int));  
            // addr = (void *)LS2K1000KA_GCR;
            data = readl(addr);
            data |= (1<<LS2K1000KA_PWM_SEL0);
            writel(data, addr);
            printk(KERN_INFO "yxc1\n");
            break;
        case LS2K1000LA_PWM_1:
            addr = (void *)ioremap(LS2K1000KA_GCR, sizeof(unsigned int));  
            data = readl(addr);
            data |= (1<<LS2K1000KA_PWM_SEL1);
            writel(data, addr);
            printk(KERN_INFO "yxc2\n");
            break;
        case LS2K1000LA_PWM_2:
            addr = (void *)ioremap(LS2K1000KA_GCR, sizeof(unsigned int));  
            data = readl(addr);
            data |= (1<<LS2K1000KA_PWM_SEL2);
            writel(data, addr);
            printk(KERN_INFO "yxc3\n");
            break;
        // case LS2K1000LA_PWM_3:
        //     addr = (void *)LS2K1000KA_GCR;
        //     data = readl(addr);
        //     data |= (1<<LS2K1000KA_PWM_SEL3);
        //     writel(data, addr);
        //     break;
    }

    // 四路控制器基地址计算
    // pwm\n  -->  0x1fe220n0
    reg_base = motor_pwm_base + (PWMn << 4);
    
    writel(0, reg_base + REG_PWM_CNTR);

    // 低电平时间（2000ns）
    tmp = motor_pwm_clk_rate * pulse_low_width_ns;
    do_div(tmp, 1000000000);
    writel(--tmp, reg_base + REG_PWM_Low_buffer);

    // 高电平时间(2000ns)
    tmp = motor_pwm_clk_rate * (pulse_high_width_ns + pulse_low_width_ns);
    do_div(tmp, 1000000000);
    writel(--tmp, reg_base + REG_PWM_Full_buffer);

    // 写控制寄存器
    cntr_reg_data = (0 << LS2K1000LA_PWM_CNTR_RST)
                    | (0 << LS2K1000LA_PWM_INT_SR)
                    | (0 << LS2K1000LA_PWM_INTEN)
                    | (1 << LS2K1000LA_PWM_SINGLE)
                    | (0 << LS2K1000LA_PWM_OE)
                    | (0 << LS2K1000LA_PWM_CNT_EN);
    addr = reg_base+REG_PWM_CTRL;
    writel(cntr_reg_data, addr);
    return ;
}


// 在PWMn引脚上产生一个脉冲
static void motor_pulse(int axis)
{
    unsigned int cntr_reg_data = 0;     // 写入控制寄存器的数据
    void __iomem *reg_base = NULL;
    int PWMn;

    // 将axis转换为pwmn
    switch (axis)
    {
        case X_AXIS:
            PWMn = MOTOR_X_PULSE_PWM;
            break;

        case Y_AXIS:
            PWMn = MOTOR_Y_PULSE_PWM;
            break;

        case Z_AXIS:
            PWMn = MOTOR_Z_PULSE_PWM;
            break;
        // 不存在的直接返回
        default:
            return ; 
    }
    
    reg_base = motor_pwm_base + (PWMn << 4);
    
    // 写主计数器
    writel(0, reg_base + REG_PWM_CNTR);
    
    // 写控制寄存器
    cntr_reg_data = (0 << LS2K1000LA_PWM_CNTR_RST)
                    | (0 << LS2K1000LA_PWM_INT_SR)
                    | (0 << LS2K1000LA_PWM_INTEN)
                    | (1 << LS2K1000LA_PWM_SINGLE)
                    | (0 << LS2K1000LA_PWM_OE)
                    | (1 << LS2K1000LA_PWM_CNT_EN);
    writel(cntr_reg_data, reg_base+REG_PWM_CTRL);
    return ;
}

    
// 设置定时器时间，单位ns（通过设置pwm3的寄存器LRC）
static void motor_timer_set_time(unsigned long period_ns)
{
    unsigned long long tmp = 0;

    // 判断入参是否合法
    if (MOTOR_TIMER_MIN_TIME_NS > period_ns)
    {
        period_ns = MOTOR_TIMER_MIN_TIME_NS;
    }
    if (MOTOR_TIMER_MAX_TIME_NS < period_ns)
    {
        period_ns = MOTOR_TIMER_MAX_TIME_NS;
    }

    tmp = motor_pwm_clk_rate * period_ns;
    do_div(tmp, 1000000000);
    writel(tmp, motor_pwm_base + (MOTOR_TIMER_PWM << 4) + REG_PWM_Full_buffer);
    return ;
}


// 定时器初始化
// 定时器用于控制步进电机的速度
// 把pwm3用作定时器
// 这里要改
static void motor_timer_init(void)
{
    writel(0x011, motor_pwm_base + (MOTOR_TIMER_PWM << 4) + REG_PWM_CTRL);
}

// 停止定时器
static void motor_timer_stop(void)
{
    writel(0x010, motor_pwm_base + (MOTOR_TIMER_PWM << 4) + REG_PWM_CTRL);
}


/*
 * 重新启动定时器
 * pwm定时器中断后，如果不调此函数重启定时器，则整个系统卡死
 * @period_ns 定时时间，单位ns
 */
static void motor_timer_restart(unsigned long period_ns)
{
    motor_timer_set_time(period_ns);
    writel(0, motor_pwm_base + (MOTOR_TIMER_PWM << 4) + REG_PWM_CNTR);        // 计数器清零
    writel(0x111, motor_pwm_base + (MOTOR_TIMER_PWM << 4) + REG_PWM_CTRL);
}

static irqreturn_t motor_timer_irq_handler(int irq, void *devid)
{
    static motor_cmd_block_t current_block;
    static unsigned long step_events_completed = 0;
    static long counter_x, counter_y, counter_z, counter_e;
    int ret = 0;
    
    // 如果不停止或重启定时器，则整个系统卡死
    motor_timer_stop();

    // 判断当前block是否处理完毕
    if (TRUE == motor_current_block_complete)
    {
        // 已经处理完成，则重新从fifo取出一个            
        // 判断fifo是否为空
        if (kfifo_is_empty(&motor_cmd_block_fifo))
        {
            // 为空，则直接返回
            motor_buffered_steps_executed = TRUE;
            goto fail_timer_restart;
        }
        motor_buffered_steps_executed = FALSE;

        // 从fifo取出一个
        ret = kfifo_out(&motor_cmd_block_fifo, &current_block, sizeof(motor_cmd_block_t));
        if (sizeof(motor_cmd_block_t) != ret)
        {
            goto fail_timer_restart;
        }

        // 判断block内容是否被破坏
        if (TRUE == motor_cmd_block_is_err(&current_block))
        {
            goto fail_timer_restart;
        }

        motor_current_block_complete = FALSE;

        // 重新初始化bresenham算法需要用到的变量
        counter_x = -(current_block.step_event_count>>1);
        counter_y = counter_z = counter_e = counter_x;
        step_events_completed = 0;

        // 设置所有电机的方向
        motor_set_all_direction(&current_block);       
    }

    // 更新限位开关状态
    if (current_block.check_endstop)
    {
        motor_update_endstops(&current_block, &step_events_completed);
    }

    // bresenham算法画直线

    // XYZ三个电机的步进脉冲用硬件pwm产生
    #define STEP_ADD_XYZ(counter, axis) \
        counter += current_block.steps[axis]; \
        if (0 < counter) \
        { \
            motor_pulse(axis); \
            counter -= current_block.step_event_count; \
        }

    // E电机的步进脉冲用软件模拟
    #define STEP_ADD_E() \
        counter_e += current_block.steps[E_AXIS]; \
        if (0 < counter_e) \
        { \
            queue_work(motor_queue, &motor_work); \
            counter_e -= current_block.step_event_count; \
        }
    
    STEP_ADD_XYZ(counter_x, X_AXIS);
    STEP_ADD_XYZ(counter_y, Y_AXIS);
    STEP_ADD_XYZ(counter_z, Z_AXIS);
    STEP_ADD_E();

    step_events_completed++;

    // 判断是否执行完一个block中所有step
    // 四个电机其中步数最大的一个结束此block才处理完成
    if (step_events_completed >= current_block.step_event_count)
    {
        motor_current_block_complete = TRUE;
    }
    
    // 判断是否使用s曲线加减速
    if (FALSE == current_block.s_cure_used)
    {
        // 没有使用s曲线，使用的是匀速，不加减速，速度变化是瞬间跳变的
        motor_timer_restart(current_block.timer_period_ns);
        return IRQ_HANDLED;
    }

fail_timer_restart:
    // 重新启动定时器
    motor_timer_restart(MOTOR_TIMER_DEFAULT_TIME_NS);

    return IRQ_HANDLED;
}


// 定时器中断的下半部(工作队列)
static void motor_work_queue(struct work_struct *work)
{
    // 用软件模拟E电机的步进脉冲
    gpio_set_value(motor_data[E_AXIS].step_gpio, GPIO_LEVEL_HIGH);
    udelay(2);
    gpio_set_value(motor_data[E_AXIS].step_gpio, GPIO_LEVEL_LOW);
    udelay(2);
    
    return ;
}


// gpio初始化
static int motor_gpio_init(void)
{
    int i=0;
    int ret = 0;
    for (i=0; i<NUM_AXIS; i++)
    {        
        // gpio_reqest使能脚
        // 所有电机共用一个使能脚
        ret = gpio_request(motor_data[i].enable_gpio,  "ls2k1000la_3dprinter_motor");
        if (0 > ret)
        {
            printk(KERN_ERR "gpio_request motor %d's enable gpio %d fail.\n", i, motor_data[i].enable_gpio);
            return ret;
        }
        gpio_direction_output(motor_data[i].enable_gpio, GPIO_MOTOR_DISABLE);

        // gpio_reqest方向脚
        ret = gpio_request(motor_data[i].direction_gpio, "ls2k1000la_3dprinter_motor");
        if (0 > ret)
        {
            printk(KERN_ERR "gpio_request motor %d's direction gpio %d fail.\n", i, motor_data[i].direction_gpio);
            return ret;
        }
        gpio_direction_output(motor_data[i].direction_gpio, MOTOR_DIRECTION_POSITIVE);

        // xyz电机的step由硬件pwm产生，e电机的step才由软件模拟产生单脉冲
        if (E_AXIS == i)
        {
            // gpio_reqest步进脚
            ret = gpio_request(motor_data[i].step_gpio, "ls2k1000la_3dprinter_motor");
            if (0 > ret)
            {
                printk(KERN_ERR "gpio_request motor %d's step gpio %d fail.\n", i, motor_data[i].step_gpio);
                return ret;
            }
            gpio_direction_output(motor_data[i].step_gpio, GPIO_LEVEL_LOW);
        }
        
        // gpio_reqest限位开关
        ret = gpio_request(motor_data[i].endstop_gpio, "ls2k1000la_3dprinter_motor");
        if (0 > ret)
        {
            printk(KERN_ERR "gpio_request motor %d's endstop gpio %d fail.\n", i, motor_data[i].endstop_gpio);
            return ret;
        }
        gpio_direction_input(motor_data[i].endstop_gpio);
    }

    return 0;
}



static int motor_open(struct inode *inode, struct file *filep)
{
    int i;
    // 使能所有电机
    for (i=0; i<NUM_AXIS; i++)
    {
        gpio_direction_output(motor_data[i].enable_gpio, GPIO_MOTOR_ENABLE);
    }
    // 打印所有限位器的状态
    motor_print_all_endstop_status();
    // 配置定时器，并启动
    motor_timer_restart(MOTOR_TIMER_DEFAULT_TIME_NS);
    
    return 0;
}


static int motor_close(struct inode *inode, struct file *filp)
{
    int i;
    // 停止定时器
    motor_timer_stop();
    // 清空fifo，重置标记
    kfifo_reset(&motor_cmd_block_fifo);
    motor_current_block_complete = TRUE;
    // 禁用所有电机
    for (i=0; i<NUM_AXIS; i++)
    {
        gpio_direction_output(motor_data[i].enable_gpio, GPIO_MOTOR_DISABLE);
    }
    return 0;
}


static ssize_t motor_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
    motor_status_t status;
    
    // 获取状态
    motor_get_status(&status);

    // 传递给应用程序
    if (copy_to_user(buf, &status, sizeof(motor_status_t)))
    {
        return -EFAULT;
    }
    return sizeof(motor_status_t);
}


static ssize_t motor_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
    int ret;
    unsigned int copied = 0;

    // 判断写入数据的长度是否为block大小的整数倍
    if ((0!=count%sizeof(motor_cmd_block_t)) || (0>=count))
    {
        printk(KERN_ERR "[%s] write size is not block size.\n", __FUNCTION__);
        return -1;
    }

    // 判断fifo是否有足够的空余空间
    // 保证每次写入能把整个block一次写入
    if (TRUE == motor_cmd_block_fifo_is_full())
    {
        return -ENOSPC;
    }
    
    if (mutex_lock_interruptible(&motor_write_lock))
    {
        return -ERESTARTSYS;
    }
    // 写入一个block
    ret = kfifo_from_user(&motor_cmd_block_fifo, buf, count, &copied);
    mutex_unlock(&motor_write_lock);

    motor_buffered_steps_executed = FALSE;

    return ret ? ret : copied;
}


static struct file_operations ls2k1000la_motor_ops = {
    .owner      = THIS_MODULE,
    .open       = motor_open,
    .release    = motor_close,
    .read       = motor_read,
    .write      = motor_write,
};


static struct miscdevice ls2k1000la_motor_miscdev = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "ls2k1000la_3dprinter_motor",
    .fops   = &ls2k1000la_motor_ops,
};

// 结构体pdev定义在/include/linux/platform_device.h
static int motor_probe(struct platform_device *pdev)
{   
    int ret = 0;
    struct resource *res = NULL;
    int irq = 0;
    struct clk *pwm_clk = NULL;
    // void __iomem *addr = NULL;
    //     unsigned int data = 0;

    printk(KERN_INFO "here!!! irq is coming !!!ls2k1000la 3dprinter motor driver's build start\n");
    // addr = (void *)ioremap(0x1fe0141b, sizeof(unsigned int));  
    // // addr = (void *)LS2K1000KA_GCR;
    // data = readl(addr);
    // printk("%d", data);
    // // 2302720
    //dev是结构体pdev的一个成员，也是一个结构体（定义在/include/linux/device.h）
    motor_data = pdev->dev.platform_data;
    if (!motor_data)
    {
        dev_err(&pdev->dev, "failed to find platform data.\n");
        return -EINVAL;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (NULL == res)
    {
        dev_err(&pdev->dev, "no IO memory resource defined.\n");
        return -ENODEV;
    }

//在 Linux 内核中，request_mem_region 和 ioremap 在不同的版本中有不同的使用方式和推荐程度。在较旧的内核版本中，request_mem_region 通常被用来锁定物理内存区域，防止其他驱动程序或内核组件错误地访问它。然而，从 Linux 2.6.x 版本开始，request_mem_region 开始被认为是过时的，并在较新的内核版本中被废弃，取而代之的是 request_mem_region 的设备树（Device Tree）或平台数据（Platform Data）替代品。
//在您的代码中，您首先使用 request_mem_region 来请求内存资源，然后使用 ioremap 来映射该资源到内核虚拟地址空间。但是，由于 request_mem_region 已经被废弃，并且在一些较新的内核版本中可能不存在，因此使用它可能不是最佳实践。
//如果您的内核版本是 4.19.190，我建议您避免使用 request_mem_region，并直接使用 ioremap（或更好的是 devm_ioremap_resource）来映射物理内存。devm_ioremap_resource 函数不仅自动处理内存映射的分配和释放，而且它还处理与设备生命周期相关的资源释放。
//如果您确实需要确保没有其他驱动程序或内核组件访问该内存区域（尽管这在现代 Linux 系统中通常不是必要的，因为内存保护是由硬件和内存管理单元（MMU）提供的），您可以考虑使用设备树或其他机制来标记和预留该内存区域。
    // res = request_mem_region(res->start, resource_size(res), pdev->name);
    // if (NULL == res)
    // {
    //     dev_err(&pdev->dev, "failed to request memory resource.\n");
    //     return -EBUSY;
    // }

    // motor_pwm_base = ioremap(res->start, resource_size(res));
    // if (NULL == motor_pwm_base)
    // {
    //     dev_err(&pdev->dev, "ioremap() fail.\n");
    //     goto fail_free_res;
    // }

    //cat /proc/iomem观察是否有占用，发现pwm0和pwm1已经被占用
    motor_pwm_base = devm_ioremap_resource(&pdev->dev, res);  
    //不推荐使用硬编码的方法，这里暂时没有其他办法
     
    motor_pwm_base = (void *)ioremap(0x1fe22000, sizeof(unsigned int));
    if (NULL == motor_pwm_base)
    {
        dev_err(&pdev->dev, "ioremap() fail.\n");
        printk(KERN_INFO "123 incorrect ls2k1000la 3dprinter motor driver's build start\n");

        goto fail_free_res;
    }
 //   printk(KERN_INFO "%d\n", motor_pwm_base);
//


    motor_queue = create_workqueue("motor_queue");
    if (!motor_queue)
    {
        dev_err(&pdev->dev, "failed to create workqueue.\n");
        goto fail_free_irq;
    }
    INIT_WORK(&motor_work, motor_work_queue);
    printk(KERN_INFO "here!!!123123     ls2k1000la 3dprinter motor driver's build start\n");
    // gpio初始化
    ret = motor_gpio_init();
    if (0 > ret)
    {
        goto fail_destroy_workqueue;
    }
    printk(KERN_INFO "wocao123 ls2k1000la 3dprinter motor driver's build start\n");

    // 获取中断资源  
    // res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);  
    // if (!res) {  
    //     dev_err(&pdev->dev, "Failed to get IRQ resource\n");  
    //     return -ENODEV;  
    // }  
    // 提取中断号  
    // irq = res->start; 
    // // 请求中断  
    // if (request_irq(irq, my_interrupt_handler, IRQF_TRIGGER_LOW, "my_driver", pdev) < 0) {  
    //     dev_err(&pdev->dev, "Failed to request IRQ %d\n", irq);  
    //     return -EBUSY;  
    // }  
     //   获取pwm计数器的时钟频率
    pwm_clk = clk_get(NULL, "apb");
    if (IS_ERR(pwm_clk))
    {
        ret = PTR_ERR(pwm_clk);
        pwm_clk = NULL;
        printk(KERN_ERR "[%s] clk_get() fail.\n", __FUNCTION__);
        goto fail_destroy_workqueue;
    }
    motor_pwm_clk_rate = (unsigned long long)clk_get_rate(pwm_clk);
    clk_put(pwm_clk);

    // 申请gcode指令对应的block fifo
    ret = kfifo_alloc(&motor_cmd_block_fifo, MOTOR_CMD_BLOCK_FIFO_SIZE, GFP_KERNEL);
    if (ret)
    {
        printk(KERN_ERR "[%s] kfifo_alloc fail.\n", __FUNCTION__);
        goto fail_destroy_workqueue;
    }
    printk(KERN_INFO "fifo over!");

    printk(KERN_INFO "timer init start\n");

    // 初始化用于产生步进脉冲的pwm0、pwm1和pwm2
    motor_pulse_PWMn_init(MOTOR_X_PULSE_PWM);
    motor_pulse_PWMn_init(MOTOR_Y_PULSE_PWM);
    motor_pulse_PWMn_init(MOTOR_Z_PULSE_PWM);
    
    // 初始化用作定时器的pwm3
    motor_timer_init();
    printk(KERN_INFO "timer init over\n");

    irq = platform_get_irq(pdev, 0);
    if (0 > irq)
    {
        dev_err(&pdev->dev, "no IRQ resource defined.\n");
        return -ENXIO;
    }
    if (irq == 107)
    {
        printk(KERN_INFO "irq is 107\n");
    }
    else
    {
        printk(KERN_INFO "ERROR");
    }
    
    // ret = motor_gpio_init();
    // if (0 > ret)
    // {
    //     goto fail_destroy_workqueue;
    //     printk(KERN_INFO "su!!!");
    // }
    // unsigned int data = 0;
    // void __iomem *addr = NULL;
    // addr = (void *)0x1fe0141b;
    // data = readl(addr);
    // printk(KERN_INFO "%d\n", data);
    // motor_timer_init();
    // ret = request_irq(107, motor_timer_irq_handler, IRQF_SHARED, pdev->name, pdev);
    ret = request_irq(irq, motor_timer_irq_handler, UMH_DISABLED, pdev->name, NULL);
   // ret = request_threaded_irq(27, motor_timer_irq_handler, motor_timer_irq_handler,  UMH_DISABLED, pdev->name, pdev);  
    if (0 > ret)
    {
        dev_err(&pdev->dev, "failed to request IRQ.\n");
        goto fail_free_io;
    }

    return 0;
fail_destroy_workqueue:
    destroy_workqueue(motor_queue);
fail_free_irq:
    free_irq(irq, NULL);
fail_free_io:
    iounmap(motor_pwm_base);
fail_free_res:
    release_mem_region(res->start, resource_size(res));

    return ret;
}


static int motor_remove(struct platform_device *pdev)
{
    int i;
    int irq;
    struct resource *res = NULL;

    kfifo_free(&motor_cmd_block_fifo);
    flush_workqueue(motor_queue);
    destroy_workqueue(motor_queue);
    
    irq = platform_get_irq(pdev, 0);
    if (0 <= irq)
    {
        free_irq(irq, NULL);
    }

    iounmap(motor_pwm_base);
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (NULL != res)
    {
        release_mem_region(res->start, resource_size(res));
    }
    
    for (i=0; i<NUM_AXIS; i++)
    {
        gpio_free(motor_data[i].enable_gpio);   // 4个电机的使能脚是共用的
        gpio_free(motor_data[i].direction_gpio);

        // xyz电机的step由硬件pwm产生，e电机的step才由软件模拟产生单脉冲
        if (E_AXIS==i)
        {
            gpio_free(motor_data[i].step_gpio);
        }
        
        gpio_free(motor_data[i].endstop_gpio);
    }
    
    return 0;
}

static struct resource ls2k1000la_3dprinter_motor_resources[] = {
    {
        .start  = 0x1fe22020,
        .end    = 0x1fe22030 - 1,
        .flags  = IORESOURCE_MEM,
    },{
        .start  = 230,
        .end    = 230,
        .flags  = IORESOURCE_IRQ,
    }
};


static struct platform_3dprinter_motor_data ls2k1000la_3dprinter_motor_data[] = {
    [X_AXIS] = {
        .enable_gpio    = PRINTER_MOTOR_X_ENABLE_PIN,
        .direction_gpio = PRINTER_MOTOR_X_DIRECTION_PIN,
        .step_gpio      = PRINTER_MOTOR_X_STEP_PIN,
        .endstop_gpio   = PRINTER_MOTOR_X_ENDSTOP_PIN,
    },
    [Y_AXIS] = {
        .enable_gpio    = PRINTER_MOTOR_Y_ENABLE_PIN,
        .direction_gpio = PRINTER_MOTOR_Y_DIRECTOR_PIN,
        .step_gpio      = PRINTER_MOTOR_Y_STEP_PIN,
        .endstop_gpio   = PRINTER_MOTOR_Y_ENDSTOP_PIN,
    },
    [Z_AXIS] = {
        .enable_gpio    = PRINTER_MOTOR_Z_ENABLE_PIN,
        .direction_gpio = PRINTER_MOTOR_Z_DIRECTOR_PIN,
        .step_gpio      = PRINTER_MOTOR_Z_STEP_PIN,
        .endstop_gpio   = PRINTER_MOTOR_Z_ENDSTOP_PIN,
    },
    [E_AXIS] = {
        .enable_gpio    = PRINTER_MOTOR_E_ENABLE_PIN,
        .direction_gpio = PRINTER_MOTOR_E_DIRECTOR_PIN,
        .step_gpio      = PRINTER_MOTOR_E_STEP_PIN,
        .endstop_gpio   = PRINTER_MOTOR_E_ENDSTOP_PIN,
    },
};


static struct platform_device ls2k1000la_3dprinter_motor = 
{
    .name   = "ls2k1000la_3dprinter_motor",
    .dev    = {
        .platform_data = ls2k1000la_3dprinter_motor_data,
    },
    .resource       = ls2k1000la_3dprinter_motor_resources,
    .num_resources  = ARRAY_SIZE(ls2k1000la_3dprinter_motor_resources),
};

static struct platform_device *platform_devs[] = {  
    &ls2k1000la_3dprinter_motor,  
};  
  
/* 定义数组中的设备数量 */  
static int num_platform_devs = ARRAY_SIZE(platform_devs);  

static struct platform_driver ls2k1000la_motor_driver = {
    .driver = {
        .name = "ls2k1000la_3dprinter_motor",
        .owner = THIS_MODULE,
    },
    .probe  = motor_probe,
    .remove = motor_remove,
};


static int __init motor_init(void)
{
    if (misc_register(&ls2k1000la_motor_miscdev))
    {
        printk(KERN_ERR "could not register 3dprinter motor driver!\n");
        return -EBUSY;
    }
    //调试
    //不添加这一行无法进入probe进行初始化
    platform_add_devices(platform_devs, num_platform_devs);  
    printk(KERN_INFO "yxc ls2k1000la 3dprinter motor driver's build start\n");
    printk(KERN_ERR "yxc could not register 3dprinter motor driver!\n");
    return platform_driver_register(&ls2k1000la_motor_driver);
}


static void __exit motor_exit(void)
{
    misc_deregister(&ls2k1000la_motor_miscdev);
    platform_driver_unregister(&ls2k1000la_motor_driver);
}


module_init(motor_init);
module_exit(motor_exit);

MODULE_AUTHOR("龙芯创印");
MODULE_DESCRIPTION("ls2k1000la 3dprinter motor driver");
MODULE_LICENSE("GPL");
