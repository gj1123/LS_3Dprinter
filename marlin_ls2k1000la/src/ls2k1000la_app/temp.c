// 温度相关


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "public.h"
#include "configuration.h"


#define TEMP_AD_MAX                 ((0x1<<10)-1)       // ad的最大值，ad是十位的
#define TEMP_IS_VALID_AD(ad)        ((TEMP_AD_MAX>=(ad)) && (0<=(ad)))       // 判断ad是在量程范围内
#define TEMP_BUFF_SIZE              (64)                // 缓存大小

// 温度的最大值和最小值
// PLA耗材的打印温度[195,230]
// ABS耗材的打印温度[220-250]
#define TEMP_MIN                    (0)
#define TEMP_MAX                    (270)

// 加热头和散热风扇的pwm周期(ms)
#define HEATER_FAN_PWM_TIME_MS              (200)   // 200ms


enum
{
    FAN_NOT_WORK = 0,       // 不散热(风扇不工作)
    FAN_WORK = 1,           // 散热(风扇正常工作)
};


#ifdef PID_DEBUG
#define pid_debug_printf(format,...)        printf(format, ##__VA_ARGS__);
#else
#define pid_debug_printf(format,...)
#endif



// TM7705的通道
enum
{
    TM7705_CH_1 = 0,            // 通道1
    TM7705_CH_2 = 1             // 通道2
};


// 以下根据ntc热敏电阻参数用脚本生成的adc值与温度一一对应的表格
// 左边为adc值，右边为温度(单位:摄氏度)
// 详细请参考源码目录中的脚本"createTemperatureLookup.py"
// python createTemperatureLookup.py
// Thermistor lookup table for RepRap Temperature Sensor Boards (http://make.rrrf.org/ts)
// Made with createTemperatureLookup.py (http://svn.reprap.org/trunk/reprap/firmware/Arduino/utilities/createTemperatureLookup.py)
// ./createTemperatureLookup.py --r0=100000 --t0=25 --r1=0 --r2=4700 --beta=3950 --max-adc=1023
// r0: 100000
// t0: 25
// r1: 0
// r2: 4700
// beta: 3950
// max adc: 1023
#define NUMTEMPS 40
const short temptable[NUMTEMPS][2] = {
   {1, 938},
   {27, 326},
   {53, 269},
   {79, 239},
   {105, 219},
   {131, 204},
   {157, 192},
   {183, 182},
   {209, 174},
   {235, 166},
   {261, 160},
   {287, 153},
   {313, 148},
   {339, 143},
   {365, 138},
   {391, 133},
   {417, 129},
   {443, 125},
   {469, 120},
   {495, 116},
   {521, 113},
   {547, 109},
   {573, 105},
   {599, 101},
   {625, 98},
   {651, 94},
   {677, 90},
   {703, 86},
   {729, 82},
   {755, 78},
   {781, 74},
   {807, 70},
   {833, 65},
   {859, 60},
   {885, 54},
   {911, 48},
   {937, 41},
   {963, 31},
   {989, 18},
   {1015, -8}
};


// 一个pwm周期内，工作的时长，单位ms
// 由于驱动中不能进行浮点计算
// 所以在应用程序中将pid的占空比直接换算为一个周期内加热头和散热的风扇工作的时长
typedef struct{
    unsigned int extruder_heater_ms;        // 挤出机加热的时长(ms)
    unsigned int extruder_fan_ms;           // 挤出机风扇工作的时长(ms)
}heater_fan_work_time_t;


// 加热头和风扇的设备文件
int fd_heater_fan;

// pid调节后期待的恒温温度
// 挤出头的目标温度
// 默认为0，打印完毕后也会设置为0
float extruder_target_temp = 0;

// 挤出头的当前温度
float extruder_current_temp;


// pid系数
float Kp = DEFUALT_Kp;
float Ki = DEFAULT_Ki;
float Kd = DEFAULT_Kd;
float Kc = DEFAULT_Kc;


// 读取指定通道的ad值
// @channel 通道号
// @adc_p 读到的ad值
// @ret 成功 or 失败
int temp_get_ad(int channel, UINT16 *adc_p)
{
    const char ch1_path[] = {"/sys/bus/spi/drivers/TM7705/spi0.1/ch1"};
    const char ch2_path[] = {"/sys/bus/spi/drivers/TM7705/spi0.1/ch2"};
    const char *dev_file_path = NULL;
    int fd = 0;
    int ret = 0;
    unsigned int value = 0;
    char buff[TEMP_BUFF_SIZE] = {0};

    // 不同的通道对应/sys下不同的文件
    if (TM7705_CH_1 == channel)
    {
        dev_file_path = ch1_path;
    }
    else
    {
        dev_file_path = ch2_path;
    }
    
    fd = open(dev_file_path, O_RDONLY);
    if (-1 == fd)
    {
        printf("[%s] open device file fail.\n", __FUNCTION__);
        return ERROR;
    }
    
    memset(buff, 0, TEMP_BUFF_SIZE);
    ret = read(fd, buff, TEMP_BUFF_SIZE-1);
    if (0 > ret)
    {
        printf("[%s] not read data. ret=%d\n", __FUNCTION__, ret);
        close(fd);
        return ERROR;
    }
    sscanf(buff, "%u\n", &value);
    value = value >> 6;
//    printf("[%s] value=%u, buff=%s\n", __FUNCTION__, value, buff);

    close(fd);

    if (!TEMP_IS_VALID_AD(value))
    {
        printf("[%s] adc convert fail. ad=%u\n", __FUNCTION__, value);
        return ERROR;
    }
    *adc_p = value;             // 输出ad值

    return SUCCESS;
}


// 根据adc值计算温度值
// ntc热敏电阻的阻值温度曲线被分为n段，每段可以近似为直线，
// 所以温度值的计算就转变为查表再计算
// @ad ad值(取值范围为0-1023)
// @temp_p 温度值，单位摄氏度
// @ret 成功 or 失败
int temp_calc_from_ad(UINT16 ad, float *temp_p)
{
    float celsius = 0.0;        // 温度值，单位摄氏度
    int i = 0;
    
    // 判断adc值是否在量程范围内
    if (!TEMP_IS_VALID_AD(ad))
    {
        return ERROR;
    }

    // 判断是否在表格所表示的范围内
    if (ad < temptable[0][0])               // 小于表格的最小adc
    {
        *temp_p = temptable[0][1];          // 取最小值
        return SUCCESS;
    }
    if (ad > temptable[NUMTEMPS-1][0])      // 大于表格的最大adc
    {
        *temp_p = temptable[NUMTEMPS-1][1]; // 取最大值
        return SUCCESS;
    }

    // 查表
    // 这里是从adc由低到高，逐个区间进行比较，没有采用折半查找
    for (i=1; i<NUMTEMPS; i++)              // 注意，这里是从1开始的，巧妙之处就在这里
    {
        if (ad < temptable[i][0])           // 判断是否在这个区间
        {
            // t = t0 + (adc-adc0)*k
            celsius = temptable[i-1][1] +                   // t0
                      (ad - temptable[i-1][0]) *            // adc-adc0
                      ((float)(temptable[i][1]-temptable[i-1][1]) / (float)(temptable[i][0]-temptable[i-1][0]));   // k
//            printf("[%s] adc=%u, celsius=%f\n", __FUNCTION__, ad, celsius);
            *temp_p = celsius;
            return SUCCESS;
        }
    }

    return ERROR;
}


// 从传感器获取温度值
// @temp_p 温度值，单位摄氏度
// @ret 成功 or 失败
int temp_get_from_sensor(float *temp_p)
{
    UINT16 ad = 0;
    int ret = ERROR;

    // 获取ad值
    ret = temp_get_ad(TM7705_CH_1, &ad);
    if (SUCCESS != ret)
    {
        printf("[%s] get channel 1's ad fail.\n", __FUNCTION__);
        return ret;
    }
//    printf("[%s] ad=%u\n", __FUNCTION__, ad);

    // 根据ad计算温度值
    return temp_calc_from_ad(ad, temp_p);
}


// 初始化
int temp_init(void)
{
    fd_heater_fan = open("/dev/3dPrinter_heater_fan", O_WRONLY);
    if (-1 == fd_heater_fan)
    {
        printf("[%s] open file /dev/3dPrinter_heater_fan fail.\n", __FUNCTION__);
        return ERROR;
    }

    return SUCCESS;
}


// 停止加热
void temp_stop_heater(void)
{
    heater_fan_work_time_t work_time = {0};
    
    work_time.extruder_heater_ms = 0;
    work_time.extruder_fan_ms = HEATER_FAN_PWM_TIME_MS;
    write(fd_heater_fan, &work_time, sizeof(heater_fan_work_time_t));

    return ;
}


// 根据当前温度计算pid值
// @current_temp 当前温度
// @ret pid值，取值范围[0,1]
float temp_get_pid(float current_temp)
{
    float current_diff;                     // 当前温度与目标温度的差值
    float target_temp;                      // 目标温度
    static float last_diff = 0.0;           // 上一次的 当前温度与目标温度的差值
    float pid;                              // 计算得到的pid值
    float p_term = 0.0;                     // 比例项
    static float i_term = 0.0;              // 积分项
    static float d_term = 0.0;              // 微分项
    float c_term = 0.0;                     // 维持功率项
    static BOOL i_reset = FALSE;            // 是否复位积分计算过程
    static float diff_sum = 0.0;            // 偏差累加和
    float diff_sum_min = PID_MIN;           // 偏差累加和的下限
    float diff_sum_max = PID_MAX/Ki;        // 偏差累加和的上限

    // 计算偏差
    target_temp = extruder_target_temp;
    current_diff = target_temp - current_temp;

    // 计算微分项，同时对微分项平滑滤波
    // 不平滑滤波的话，就是d_term = Kd * (current_diff - last_diff)
    d_term = PID_SMOOTHING_FACTOR * d_term +(1.0-PID_SMOOTHING_FACTOR) * Kd * (current_diff - last_diff);
    last_diff = current_diff;       // 更新

    // 当前温度还远低于目标温度时，全速加热
    if (PID_FUNCTIONAL_RANGE < current_diff)
    {
        pid = PID_LONG_TIME_ACTION;
        i_reset = TRUE;
        pid_debug_printf("[%s] target_temp=%f, current_temp=%f, pid=%f, current temp is too low, heater all work\n",
               __FUNCTION__, target_temp, current_temp, pid);
        return pid;
    }
    // 当前温度远高于目标温度时，不加热
    else if (-PID_FUNCTIONAL_RANGE > current_diff || 0 == target_temp)
    {
        pid = PID_MIN;
        i_reset = TRUE;
        pid_debug_printf("[%s] target_temp=%f, current_temp=%f, pid=%f, current temp is too high, heater not work\n",
               __FUNCTION__, target_temp, current_temp, pid);
        return pid;
    }
    // 当前温度在目标温度附近时，使用pid算法控制温度
    else
    {
        // 判断是否复位积分计算过程
        // 当温度偏差在区间[-PID_FUNCTIONAL_RANGE, PID_FUNCTIONAL_RANGE]内才积分
        if (TRUE == i_reset)
        {
            i_reset = FALSE;
            diff_sum = 0.0;             // 复位偏差累加和
        }
        
        // 计算比例项
        p_term = Kp * current_diff;

        // 计算积分项
        diff_sum += current_diff;
        diff_sum = constrain_float(diff_sum, diff_sum_min, diff_sum_max);   // 限制积分项的范围
        i_term = Ki * diff_sum;

        // 计算维持功率项
        // 当温度不同时，散热快慢也不同，此项起抵消散热维持温度的作用，可增加系统稳定性
        c_term = Kc * target_temp;

        // 计算pid
        pid = p_term + i_term + d_term + c_term;

        // 积分分离
        if (((PID_MAX < pid) && (0 < current_diff))
            || ((PID_MIN > pid) && (0 > current_diff)))
        {
            diff_sum -= current_diff;   // 取消当前的积分，采用上一次的积分
        }

        // 限制pid的范围
        pid = constrain_float(pid, PID_MIN, PID_MAX);
        if (PID_ACTION_MIN > pid)
        {
            pid = 0;
        }

        pid_debug_printf("[%s] target_temp=%f, current_temp=%f, pid=%f, Kp=%f, Ki=%f, Kd=%f, Kc=%f, p_term=%f, i_term=%f, d_term=%f, c_term=%f\n",
               __FUNCTION__, target_temp, current_temp, pid, Kp, Ki, Kd, Kc, p_term, i_term, d_term, c_term);
        
        return pid;
    }
    
}


// 将pid转换为pwm值，并传给驱动执行
// @pid pid值，取值范围[0,1]
void temp_pid_to_pwm(float pid)
{
    heater_fan_work_time_t work_time = {0};

    work_time.extruder_heater_ms    = HEATER_FAN_PWM_TIME_MS*pid;
    work_time.extruder_fan_ms = HEATER_FAN_PWM_TIME_MS;
    write(fd_heater_fan, &work_time, sizeof(heater_fan_work_time_t));

    return ;
}


// 温度控制的线程
// 通过读取当前温度，再用pid算法控制加热，使温度恒定在指定的值上
void *temp_thread_fn(void *arg)
{
    float temp = 0.0;   // 采集的温度值
    float pid = 0.0;    // 根据温度计算得到的pid值，取值范围[0,1]
    int ret = ERROR;
    
    if (SUCCESS != temp_init())
    {
        return NULL;
    }
    
    while (1)
    {
        usleep(500*1000);       // 500ms

        // 采集温度
        ret = temp_get_from_sensor(&temp);
        if (SUCCESS != ret)
        {
            // 采集失败
            temp_stop_heater();         // 停止加热，开启散热风扇
            printf("[%s] temp_get fail. ret=%d\n", __FUNCTION__, ret);
            continue;
        }
        extruder_current_temp = temp;
        printf("[%s] current_temp=%f\n", __FUNCTION__, temp);

        // 计算pid
        pid = temp_get_pid(temp);

        // 将pid转换为pwm值，并传给驱动执行
        temp_pid_to_pwm(pid);
    }
}



// 获取挤出头当前温度
float temp_get_extruder_current_temp(void)
{
    return extruder_current_temp;
}


// 设置挤出头目标温度
// @target_temp 挤出头目标温度
void temp_set_extruder_target_temp(float target_temp)
{
    // 判断目标温度是否在合理范围内
    if ((TEMP_MIN>target_temp) || (TEMP_MAX<target_temp))
    {
        printf("[%s] target_temp=%f is invalid.\n", __FUNCTION__, target_temp);
        return ;
    }
    
    extruder_target_temp = target_temp;
    return ;
}


// 打印挤出头目标温度
void temp_print_extruder_target_temp(void)
{
    printf("extruder target temp=%f\n", extruder_target_temp);
    return ;
}


// 设置PID系数Kp
void temp_set_Kp(float new_Kp)
{
    Kp = new_Kp;
    return ;
}


// 设置PID系数Ki
void temp_set_Ki(float new_Ki)
{
    Ki = new_Ki;
    return ;
}

// 设置PID系数Kd
void temp_set_Kd(float new_Kd)
{
    Kd = new_Kd;
    return ;
}

// 设置PID系数Kc
void temp_set_Kc(float new_Kc)
{
    Kc = new_Kc;
    return ;
}

// 打印PID系数
void temp_print_Kp_Ki_Kd_Kc(void)
{
    printf("PID para: Kp=%f, Ki=%f, Kd=%f, Kc=%f\n", Kp, Ki, Kd, Kc);
    return ;
}


// 开启冷却风扇
void temp_cooling_fan_enable(void)
{
    int arg = 0;
    
    if (0 > ioctl(fd_heater_fan, FAN_WORK, &arg))
    {
        printf("[%s] enable cooling fan fail.\n", __FUNCTION__);
        return ;
    }

    return ;
}


// 关闭冷却风扇
void temp_cooling_fan_disable(void)
{
    int arg = 0;

    if (0 > ioctl(fd_heater_fan, FAN_NOT_WORK, &arg))
    {
        printf("[%s] disable cooling fan fail.\n", __FUNCTION__);
        return ;
    }

    return ;
}


