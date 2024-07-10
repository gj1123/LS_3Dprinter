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

// ���ת������
typedef enum
{
    MOTOR_DIRECTION_REVERSE = 0,            // ��ת
    MOTOR_DIRECTION_POSITIVE = 1,           // ��ת
}motor_direction_t;

typedef unsigned char BOOL;

// This struct is used when buffering the setup for each linear movement "nominal" values are as specified in the source g-code and may never actually be reached if acceleration management is active.
typedef struct 
{
    unsigned int head_flag;                     // �̶�Ϊ0xe5e5�������ж������ṹ�������Ƿ��ƻ�

    // Fields used by the bresenham algorithm for tracing the line
    // ֱ�߹켣�趨�Ĳ���
    unsigned char direction_bits;               // The direction bit set for this block
                                                // ���Ჽ������ķ���
    long steps[NUM_AXIS];                       // Step count along each axis
                                                // ���Ჽ������Ĳ���

    BOOL s_cure_used;                           // �Ƿ�ʹ��s���߼Ӽ���,(TRUE or FALSE)                     
                                                // TRUE ʹ��s���߼Ӽ���
                                                // FALSE �����˶����ٶȱ仯���������ʽ�����ַ�ʽ���ٶȲ���̫��

    unsigned long timer_period_ns;              // �����ʶ�Ӧ��(���ڿ��Ƶ���ٶȵ�)��ʱ�����ڣ���λns

    unsigned long step_event_count;             // The number of step events required to complete this block
                                                // ��block�����Ĳ���
    long accelerate_until_step;                 // The index of the step event on which to stop acceleration
                                                // ��������Ǹ�step������
    long decelerate_after_step;                 // The index of the step event on which to start decelerating
                                                // ��ʼ�����Ǹ�step������
    long acceleration_rate;                     // The acceleration rate used for acceleration calculation


    // Fields used by the motion planner to manage acceleration
    // float speed_x, speed_y, speed_z, speed_e;         // Nominal mm/sec for each axis
    float nominal_speed_mm_s;                   // The nominal speed for this block in mm/s
                                                // �������������ٲ��ֵ��ٶȣ���λmm/s
    float entry_speed_mm_s;                     // Entry speed at previous-current junction in mm/s
                                                // ����ٶȣ��ս�����������ʱ���ٶȣ���λmm/s
    float max_entry_speed_mm_s;                 // Maximum allowable junction entry speed in mm/s
                                                // ������������ٶȣ���λmm/s
    float millimeters_mm;                       // The total travel of this block in mm
                                                // ��blockt�ƶ��ľ��룬��λmm
    float acceleration_mm_s2;                   // acceleration mm/s^2
                                                // ��block�ļ��ٶȣ���λmm/s^2
    unsigned char recalculate_flag;             // Planner flag to recalculate trapezoids on entry junction
                                                // ��¼�Ƿ����¹滮�����ٶȵ�flag
    unsigned char nominal_length_flag;          // Planner flag for nominal speed always reached
                                                // ��¼nominal_speed�Ƿ��ܴﵽ��flag
    /*
     * Settings for the trapezoid generator
     * ���μӼ��ٵĲ���
     *             nominal_rate_step_s
     *             ___________________
     *            /                   \
     *           /                     \
     *          /                       \  final_rate_step_s
     * initial_rate_step_s
     * entry_speed_mm_s
     */  
    unsigned long nominal_rate_step_s;          // The nominal step rate for this block in step_events/s
                                                // �������������ٲ��ֵ��ٶȣ���λstep/s
    unsigned long initial_rate_step_s;          // The jerk-adjusted step rate at start of block
                                                // ���������иտ�ʼ����ʱ���ٶȣ���λstep/s
    unsigned long final_rate_step_s;            // The minimal rate at exit
                                                // ���������н�������ʱ���ٶȣ���λstep/s
    unsigned long acceleration_st_step_s2;      // acceleration steps/s^2
                                                // ���������м��ٲ��ֵļ��ٶȣ���λstep/s^2
                                                // ���������м��ٲ��ֵļ��ٶ�Ϊ��ֵ�ĸ���

    unsigned long fan_speed;                    // �����ٶ�
    unsigned char check_endstop;                // �Ƿ�����λ����״̬

    unsigned int end_flag;                      // �̶�Ϊ0xe5e5�������ж������ṹ�������Ƿ��ƻ�
} motor_cmd_block_t;


// ������״̬��Ϣ
typedef struct {
    // block�����Ƿ�����
    // ����Ӧ�ó���������дblockʱ�жϻ����Ƿ�����
    char cmd_block_fifo_full;   // true or false

    // fifo������step����ִ��
    // fifo��û��block���������һ��blockҲ��ȫ��ִ�����
    char buffered_steps_executed;   // true or false
} motor_status_t;


#ifndef TRUE
#define TRUE                    (1)
#endif

#ifndef FALSE
#define FALSE                   (0)
#endif

// ͨ�����üĴ���(ͨ�����üĴ��� 0�������ԹܽŸ��õĿ��ƣ��Լ� HDA��USB��PCIE ��һ���ԡ��ڴ��������RTC �������� LIO �����������õ�)
#define LS2K1000KA_GCR                             (0x1fe00420)

#define LS2K1000KA_PWM_SEL0                        (12)
#define LS2K1000KA_PWM_SEL1                        (13)
#define LS2K1000KA_PWM_SEL2                        (14)
#define LS2K1000KA_PWM_SEL3                        (15)


// PWNn����gpio
#define LS2K1000LA_PWM0_GPIO20                        (20)
#define LS2K1000LA_PWM1_GPIO21                        (21)
#define LS2K1000LA_PWM2_GPIO22                        (22)
#define LS2K1000LA_PWM3_GPIO23                        (23)


// PWM���ƼĴ���λ����
#define LS2K1000LA_PWM_CNTR_RST                 (7)     // CNTR����������
#define LS2K1000LA_PWM_INT_SR                   (6)     // �ж�״̬λ
#define LS2K1000LA_PWM_INTEN                    (5)     // �ж�ʹ��λ
#define LS2K1000LA_PWM_SINGLE                   (4)     // ���������λ
#define LS2K1000LA_PWM_OE                       (3)     // �������ʹ�ܿ���λ
#define LS2K1000LA_PWM_CNT_EN                   (0)     // CNTRʹ��λ


// �Ĵ���ƫ��
#define REG_PWM_CNTR	        0x00
#define REG_PWM_Low_buffer		0x4
#define REG_PWM_Full_buffer		0x8
#define REG_PWM_CTRL        	0xC

// ������
#define PWM_PULSE_HIGH_WIDTH_NS                 (2*1000)    // �ߵ�ƽ2us
#define PWM_PULSE_LOW_WIDTH_NS                  (2*1000)    // �͵�ƽ2us


// ��ʱ����С��ʱʱ��
// ��ʱʱ�䵽��������Ҫ����һ���ߵ͵�ƽ����������������оƬ
// Ŀǰ�������������оƬʹ�õ���a4988��������ĸߵ�ƽ�͵͵�ƽʱ�䶼������1us
// Ϊ�������㹻��ʱ�䣬����ѡ��ʱʱ����СΪ50us
#define MOTOR_TIMER_MIN_TIME_NS                 (50*1000)           // 50us
#define MOTOR_TIMER_MAX_TIME_NS                 (100*1000*1000)     // 100ms
// 42��������Ŀ�������Ƶ��Ϊ1.5khz������ѡ1khz��T=1/1000s=1ms=1000us=1000 000ns
#define MOTOR_TIMER_DEFAULT_TIME_NS             (1000*1000)         // 1ms = 1000us = 1000 000ns

// gcodeָ���Ӧblock��fifo�Ĵ�С
// marlin����16��block��ÿ��block��СΪ84�ֽڣ�����ȡ2��n���ݣ���������ѡ��2k
#define MOTOR_CMD_BLOCK_FIFO_SIZE               (2*1024)    // 2k
// �ṹ������ĩ������Ա�̶�Ϊ��ֵ�������жϽṹ�������Ƿ��ƻ�
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
static unsigned long long motor_pwm_clk_rate;       // pwmʱ��Ƶ��
static struct workqueue_struct *motor_queue;
static struct work_struct motor_work;
static struct kfifo_rec_ptr_1 motor_cmd_block_fifo;     // ���gcodeָ���Ӧ��block��fifo
static DEFINE_MUTEX(motor_write_lock);
static volatile int motor_current_block_complete = TRUE;    // ��ʱ���ж��°벿�����жϵ�ǰblock�Ƿ�ִ����ɵı��
static volatile int motor_buffered_steps_executed = TRUE;   // fifo������step����ִ����ɣ�ȡֵΪTRUE or FALSE
// Mechanical endstop with COM to ground and NC to Signal uses "false" here (most common setup).
static unsigned char motor_endstop_inverting[NUM_AXIS] = {FALSE,FALSE,FALSE,TRUE};   // set to true to invert the logic of the endstop.
static unsigned char motor_dir_inverting[NUM_AXIS] = {FALSE, FALSE, FALSE, TRUE};   // ����TRUEʹ�߼���ת�����缷����E�ĵ������ת������XYZ���෴


// �������е���ķ���
// @current_block_p ��ǰblock���ƿ�
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


// ��ȡָ���������λ���ص�״̬
// @motor_index �������
static int motor_get_endstop_status(int motor_index)
{
    // ��ȡ״̬
    return gpio_get_value(motor_data[motor_index].endstop_gpio);
}


// ��ӡ������λ����״̬
static void motor_print_all_endstop_status(void)
{
    printk(KERN_DEBUG "[%s] endstop status, X=%d, Y=%d, Z=%d\n", 
            __FUNCTION__, 
            motor_get_endstop_status(X_AXIS),
            motor_get_endstop_status(Y_AXIS),
            motor_get_endstop_status(Z_AXIS));
}


// �����λ����״̬
// @current_block_p ��ǰִ�е�block
// @step_events_completed_p ��ǰblock����ɵ�step��
static void motor_update_endstops(motor_cmd_block_t *current_block_p, unsigned long *step_events_completed_p)
{
    unsigned char current_endstop_bits= 0;

    // �жϻ����Ƿ����ϻ���
    #define CARRIAGE_MOVE_UP(axis)      (TEST_BIT(current_block_p->direction_bits, (axis)) && (0<current_block_p->steps[axis]))
    // �жϻ����Ƿ����»���
    #define CARRIAGE_MOVE_DOWN(axis)    (!TEST_BIT(current_block_p->direction_bits, (axis)) && (0<current_block_p->steps[axis]))

    /*
     * Ŀǰ���ϵ�������3D��ӡ����xyz�����Ϸ�װ����λ��
     * xyz�����ϵ���λ����Ҫ���ڵ������
     *
     * ����
     * ��ȡ��λ�����������ŵ�״̬
     * �жϻ������з����Ƿ����ϣ������Ѿ��Ӵ�����λ����
     */
    #define UPDATE_ENDSTOPS_XYZ(axis)   do { \
        SET_BIT(current_endstop_bits, (axis), (motor_endstop_inverting[axis]!=motor_get_endstop_status(axis))); \
        if (CARRIAGE_MOVE_UP(axis) && TEST_BIT(current_endstop_bits, (axis))) \
        { \
            *step_events_completed_p = current_block_p->step_event_count; \
            return ;\
        } \
    } while(0)
        
    // ���xyz�����λ����
    UPDATE_ENDSTOPS_XYZ(X_AXIS);  // ramps1.4��������λ���صĽӿڽ����ţ�����ͬʱ��4����λ���ؽ���ȡ������ѡȡy��e���ڲ���
    UPDATE_ENDSTOPS_XYZ(Y_AXIS);
    UPDATE_ENDSTOPS_XYZ(Z_AXIS);

    return ;
}


// �ж�����block�������Ƿ��ƻ������ṹ��ǰ��ı���Ƿ���ȷ
// ����0Ϊû�б��ƻ�
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


// �ж�block fifo�Ƿ�����
// ����0Ϊû������������������һ��block
static char motor_cmd_block_fifo_is_full(void)
{
    unsigned int available;
    
    // �ж�fifo�Ƿ����㹻�Ŀ���ռ�
    // ��֤ÿ��д���ܰ�����blockһ��д��
    available = kfifo_avail(&motor_cmd_block_fifo);
    if (available < sizeof(motor_cmd_block_t))
    {
        return TRUE;
    }

    return FALSE;
}


// ��ȡ״̬
static void motor_get_status(motor_status_t *status)
{    
    status->cmd_block_fifo_full         = motor_cmd_block_fifo_is_full();
    status->buffered_steps_executed     = motor_buffered_steps_executed;

    return ;
}


// ��ʼ�����ڲ������������pwm0, pwm1, pwm2
static void motor_pulse_PWMn_init(int PWMn)
{
    unsigned long long tmp = 0;
    unsigned long pulse_high_width_ns = PWM_PULSE_HIGH_WIDTH_NS;
    unsigned long pulse_low_width_ns = PWM_PULSE_LOW_WIDTH_NS;
    unsigned int cntr_reg_data = 0;     // д����ƼĴ���������
    unsigned int data = 0;
    void __iomem *reg_base = NULL;
    void __iomem *addr = NULL;

    // ����GPIOΪPWM
    switch (PWMn)
    {
        case LS2K1000LA_PWM_0:
        // ��ʹ���� ioremap �������������ַӳ�䵽�ں������ַ�ռ䡣������Ϊֱ�ӷ��������ַ���ں���ͨ���ǲ�������ģ�������ȷ��֪��������ʲô��������ͨ�����ڷǳ��ײ�Ĵ����У���ioremap ��Ϊ�㴦����Щϸ�ڡ�
        // ������� volatile �ؼ��������� addr ָ�롣������Ϊ���ǿ��ܻ�ͨ�����ָ�������ʿ�����ʱ��Ӳ�����ĵ��ڴ�λ�á���� volatile ����ȷ����������������λ�õķ��ʽ����Ż����Ӷ�ȷ�����ǵĴ����ܹ���ȷ����Ӳ��������
        // ������˴���������� ioremap �Ƿ�ɹ������ӳ��ʧ�ܣ����磬��Ϊ�ڴ治�������ԭ�򣩣�����Ӧ���ܹ���⵽��һ�㲢��ȡ��Ӧ���ж���
        // ����ɶԼĴ����Ķ�д��������ʹ���� iounmap ���ͷ�֮ǰͨ�� ioremap ��õ�ӳ�䡣����һ����ϰ�ߣ���Ϊ�����Է�ֹ�ڴ�й©������Ǳ�����⡣Ȼ������������������������������������ж��������ӳ�䣨���磬�������һ��ģ���ȫ�ַ�Χ�ڱ�������������ô����ܲ���Ҫ���� iounmap�����ǣ���ע�⣬����ܻ������ڴ�ʹ�õķ��գ������ܵ����������⡣

        // ֱ�ӷ��������ַ���ں����ǲ�������ģ�ʹ��ioremapӳ�䵽�����ַ
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

    // ��·����������ַ����
    // pwm\n  -->  0x1fe220n0
    reg_base = motor_pwm_base + (PWMn << 4);
    
    writel(0, reg_base + REG_PWM_CNTR);

    // �͵�ƽʱ�䣨2000ns��
    tmp = motor_pwm_clk_rate * pulse_low_width_ns;
    do_div(tmp, 1000000000);
    writel(--tmp, reg_base + REG_PWM_Low_buffer);

    // �ߵ�ƽʱ��(2000ns)
    tmp = motor_pwm_clk_rate * (pulse_high_width_ns + pulse_low_width_ns);
    do_div(tmp, 1000000000);
    writel(--tmp, reg_base + REG_PWM_Full_buffer);

    // д���ƼĴ���
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


// ��PWMn�����ϲ���һ������
static void motor_pulse(int axis)
{
    unsigned int cntr_reg_data = 0;     // д����ƼĴ���������
    void __iomem *reg_base = NULL;
    int PWMn;

    // ��axisת��Ϊpwmn
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
        // �����ڵ�ֱ�ӷ���
        default:
            return ; 
    }
    
    reg_base = motor_pwm_base + (PWMn << 4);
    
    // д��������
    writel(0, reg_base + REG_PWM_CNTR);
    
    // д���ƼĴ���
    cntr_reg_data = (0 << LS2K1000LA_PWM_CNTR_RST)
                    | (0 << LS2K1000LA_PWM_INT_SR)
                    | (0 << LS2K1000LA_PWM_INTEN)
                    | (1 << LS2K1000LA_PWM_SINGLE)
                    | (0 << LS2K1000LA_PWM_OE)
                    | (1 << LS2K1000LA_PWM_CNT_EN);
    writel(cntr_reg_data, reg_base+REG_PWM_CTRL);
    return ;
}

    
// ���ö�ʱ��ʱ�䣬��λns��ͨ������pwm3�ļĴ���LRC��
static void motor_timer_set_time(unsigned long period_ns)
{
    unsigned long long tmp = 0;

    // �ж�����Ƿ�Ϸ�
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


// ��ʱ����ʼ��
// ��ʱ�����ڿ��Ʋ���������ٶ�
// ��pwm3������ʱ��
// ����Ҫ��
static void motor_timer_init(void)
{
    writel(0x011, motor_pwm_base + (MOTOR_TIMER_PWM << 4) + REG_PWM_CTRL);
}

// ֹͣ��ʱ��
static void motor_timer_stop(void)
{
    writel(0x010, motor_pwm_base + (MOTOR_TIMER_PWM << 4) + REG_PWM_CTRL);
}


/*
 * ����������ʱ��
 * pwm��ʱ���жϺ���������˺���������ʱ����������ϵͳ����
 * @period_ns ��ʱʱ�䣬��λns
 */
static void motor_timer_restart(unsigned long period_ns)
{
    motor_timer_set_time(period_ns);
    writel(0, motor_pwm_base + (MOTOR_TIMER_PWM << 4) + REG_PWM_CNTR);        // ����������
    writel(0x111, motor_pwm_base + (MOTOR_TIMER_PWM << 4) + REG_PWM_CTRL);
}

static irqreturn_t motor_timer_irq_handler(int irq, void *devid)
{
    static motor_cmd_block_t current_block;
    static unsigned long step_events_completed = 0;
    static long counter_x, counter_y, counter_z, counter_e;
    int ret = 0;
    
    // �����ֹͣ��������ʱ����������ϵͳ����
    motor_timer_stop();

    // �жϵ�ǰblock�Ƿ������
    if (TRUE == motor_current_block_complete)
    {
        // �Ѿ�������ɣ������´�fifoȡ��һ��            
        // �ж�fifo�Ƿ�Ϊ��
        if (kfifo_is_empty(&motor_cmd_block_fifo))
        {
            // Ϊ�գ���ֱ�ӷ���
            motor_buffered_steps_executed = TRUE;
            goto fail_timer_restart;
        }
        motor_buffered_steps_executed = FALSE;

        // ��fifoȡ��һ��
        ret = kfifo_out(&motor_cmd_block_fifo, &current_block, sizeof(motor_cmd_block_t));
        if (sizeof(motor_cmd_block_t) != ret)
        {
            goto fail_timer_restart;
        }

        // �ж�block�����Ƿ��ƻ�
        if (TRUE == motor_cmd_block_is_err(&current_block))
        {
            goto fail_timer_restart;
        }

        motor_current_block_complete = FALSE;

        // ���³�ʼ��bresenham�㷨��Ҫ�õ��ı���
        counter_x = -(current_block.step_event_count>>1);
        counter_y = counter_z = counter_e = counter_x;
        step_events_completed = 0;

        // �������е���ķ���
        motor_set_all_direction(&current_block);       
    }

    // ������λ����״̬
    if (current_block.check_endstop)
    {
        motor_update_endstops(&current_block, &step_events_completed);
    }

    // bresenham�㷨��ֱ��

    // XYZ��������Ĳ���������Ӳ��pwm����
    #define STEP_ADD_XYZ(counter, axis) \
        counter += current_block.steps[axis]; \
        if (0 < counter) \
        { \
            motor_pulse(axis); \
            counter -= current_block.step_event_count; \
        }

    // E����Ĳ������������ģ��
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

    // �ж��Ƿ�ִ����һ��block������step
    // �ĸ�������в�������һ��������block�Ŵ������
    if (step_events_completed >= current_block.step_event_count)
    {
        motor_current_block_complete = TRUE;
    }
    
    // �ж��Ƿ�ʹ��s���߼Ӽ���
    if (FALSE == current_block.s_cure_used)
    {
        // û��ʹ��s���ߣ�ʹ�õ������٣����Ӽ��٣��ٶȱ仯��˲�������
        motor_timer_restart(current_block.timer_period_ns);
        return IRQ_HANDLED;
    }

fail_timer_restart:
    // ����������ʱ��
    motor_timer_restart(MOTOR_TIMER_DEFAULT_TIME_NS);

    return IRQ_HANDLED;
}


// ��ʱ���жϵ��°벿(��������)
static void motor_work_queue(struct work_struct *work)
{
    // �����ģ��E����Ĳ�������
    gpio_set_value(motor_data[E_AXIS].step_gpio, GPIO_LEVEL_HIGH);
    udelay(2);
    gpio_set_value(motor_data[E_AXIS].step_gpio, GPIO_LEVEL_LOW);
    udelay(2);
    
    return ;
}


// gpio��ʼ��
static int motor_gpio_init(void)
{
    int i=0;
    int ret = 0;
    for (i=0; i<NUM_AXIS; i++)
    {        
        // gpio_reqestʹ�ܽ�
        // ���е������һ��ʹ�ܽ�
        ret = gpio_request(motor_data[i].enable_gpio,  "ls2k1000la_3dprinter_motor");
        if (0 > ret)
        {
            printk(KERN_ERR "gpio_request motor %d's enable gpio %d fail.\n", i, motor_data[i].enable_gpio);
            return ret;
        }
        gpio_direction_output(motor_data[i].enable_gpio, GPIO_MOTOR_DISABLE);

        // gpio_reqest�����
        ret = gpio_request(motor_data[i].direction_gpio, "ls2k1000la_3dprinter_motor");
        if (0 > ret)
        {
            printk(KERN_ERR "gpio_request motor %d's direction gpio %d fail.\n", i, motor_data[i].direction_gpio);
            return ret;
        }
        gpio_direction_output(motor_data[i].direction_gpio, MOTOR_DIRECTION_POSITIVE);

        // xyz�����step��Ӳ��pwm������e�����step�������ģ�����������
        if (E_AXIS == i)
        {
            // gpio_reqest������
            ret = gpio_request(motor_data[i].step_gpio, "ls2k1000la_3dprinter_motor");
            if (0 > ret)
            {
                printk(KERN_ERR "gpio_request motor %d's step gpio %d fail.\n", i, motor_data[i].step_gpio);
                return ret;
            }
            gpio_direction_output(motor_data[i].step_gpio, GPIO_LEVEL_LOW);
        }
        
        // gpio_reqest��λ����
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
    // ʹ�����е��
    for (i=0; i<NUM_AXIS; i++)
    {
        gpio_direction_output(motor_data[i].enable_gpio, GPIO_MOTOR_ENABLE);
    }
    // ��ӡ������λ����״̬
    motor_print_all_endstop_status();
    // ���ö�ʱ����������
    motor_timer_restart(MOTOR_TIMER_DEFAULT_TIME_NS);
    
    return 0;
}


static int motor_close(struct inode *inode, struct file *filp)
{
    int i;
    // ֹͣ��ʱ��
    motor_timer_stop();
    // ���fifo�����ñ��
    kfifo_reset(&motor_cmd_block_fifo);
    motor_current_block_complete = TRUE;
    // �������е��
    for (i=0; i<NUM_AXIS; i++)
    {
        gpio_direction_output(motor_data[i].enable_gpio, GPIO_MOTOR_DISABLE);
    }
    return 0;
}


static ssize_t motor_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
    motor_status_t status;
    
    // ��ȡ״̬
    motor_get_status(&status);

    // ���ݸ�Ӧ�ó���
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

    // �ж�д�����ݵĳ����Ƿ�Ϊblock��С��������
    if ((0!=count%sizeof(motor_cmd_block_t)) || (0>=count))
    {
        printk(KERN_ERR "[%s] write size is not block size.\n", __FUNCTION__);
        return -1;
    }

    // �ж�fifo�Ƿ����㹻�Ŀ���ռ�
    // ��֤ÿ��д���ܰ�����blockһ��д��
    if (TRUE == motor_cmd_block_fifo_is_full())
    {
        return -ENOSPC;
    }
    
    if (mutex_lock_interruptible(&motor_write_lock))
    {
        return -ERESTARTSYS;
    }
    // д��һ��block
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

// �ṹ��pdev������/include/linux/platform_device.h
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
    //dev�ǽṹ��pdev��һ����Ա��Ҳ��һ���ṹ�壨������/include/linux/device.h��
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

//�� Linux �ں��У�request_mem_region �� ioremap �ڲ�ͬ�İ汾���в�ͬ��ʹ�÷�ʽ���Ƽ��̶ȡ��ڽϾɵ��ں˰汾�У�request_mem_region ͨ�����������������ڴ����򣬷�ֹ��������������ں��������ط�������Ȼ������ Linux 2.6.x �汾��ʼ��request_mem_region ��ʼ����Ϊ�ǹ�ʱ�ģ����ڽ��µ��ں˰汾�б�������ȡ����֮���� request_mem_region ���豸����Device Tree����ƽ̨���ݣ�Platform Data�����Ʒ��
//�����Ĵ����У�������ʹ�� request_mem_region �������ڴ���Դ��Ȼ��ʹ�� ioremap ��ӳ�����Դ���ں������ַ�ռ䡣���ǣ����� request_mem_region �Ѿ���������������һЩ���µ��ں˰汾�п��ܲ����ڣ����ʹ�������ܲ������ʵ����
//��������ں˰汾�� 4.19.190���ҽ���������ʹ�� request_mem_region����ֱ��ʹ�� ioremap������õ��� devm_ioremap_resource����ӳ�������ڴ档devm_ioremap_resource ���������Զ������ڴ�ӳ��ķ�����ͷţ����������������豸����������ص���Դ�ͷš�
//�����ȷʵ��Ҫȷ��û����������������ں�������ʸ��ڴ����򣨾��������ִ� Linux ϵͳ��ͨ�����Ǳ�Ҫ�ģ���Ϊ�ڴ汣������Ӳ�����ڴ����Ԫ��MMU���ṩ�ģ��������Կ���ʹ���豸����������������Ǻ�Ԥ�����ڴ�����
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

    //cat /proc/iomem�۲��Ƿ���ռ�ã�����pwm0��pwm1�Ѿ���ռ��
    motor_pwm_base = devm_ioremap_resource(&pdev->dev, res);  
    //���Ƽ�ʹ��Ӳ����ķ�����������ʱû�������취
     
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
    // gpio��ʼ��
    ret = motor_gpio_init();
    if (0 > ret)
    {
        goto fail_destroy_workqueue;
    }
    printk(KERN_INFO "wocao123 ls2k1000la 3dprinter motor driver's build start\n");

    // ��ȡ�ж���Դ  
    // res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);  
    // if (!res) {  
    //     dev_err(&pdev->dev, "Failed to get IRQ resource\n");  
    //     return -ENODEV;  
    // }  
    // ��ȡ�жϺ�  
    // irq = res->start; 
    // // �����ж�  
    // if (request_irq(irq, my_interrupt_handler, IRQF_TRIGGER_LOW, "my_driver", pdev) < 0) {  
    //     dev_err(&pdev->dev, "Failed to request IRQ %d\n", irq);  
    //     return -EBUSY;  
    // }  
     //   ��ȡpwm��������ʱ��Ƶ��
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

    // ����gcodeָ���Ӧ��block fifo
    ret = kfifo_alloc(&motor_cmd_block_fifo, MOTOR_CMD_BLOCK_FIFO_SIZE, GFP_KERNEL);
    if (ret)
    {
        printk(KERN_ERR "[%s] kfifo_alloc fail.\n", __FUNCTION__);
        goto fail_destroy_workqueue;
    }
    printk(KERN_INFO "fifo over!");

    printk(KERN_INFO "timer init start\n");

    // ��ʼ�����ڲ������������pwm0��pwm1��pwm2
    motor_pulse_PWMn_init(MOTOR_X_PULSE_PWM);
    motor_pulse_PWMn_init(MOTOR_Y_PULSE_PWM);
    motor_pulse_PWMn_init(MOTOR_Z_PULSE_PWM);
    
    // ��ʼ��������ʱ����pwm3
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
        gpio_free(motor_data[i].enable_gpio);   // 4�������ʹ�ܽ��ǹ��õ�
        gpio_free(motor_data[i].direction_gpio);

        // xyz�����step��Ӳ��pwm������e�����step�������ģ�����������
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
  
/* ���������е��豸���� */  
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
    //����
    //�������һ���޷�����probe���г�ʼ��
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

MODULE_AUTHOR("��о��ӡ");
MODULE_DESCRIPTION("ls2k1000la 3dprinter motor driver");
MODULE_LICENSE("GPL");
