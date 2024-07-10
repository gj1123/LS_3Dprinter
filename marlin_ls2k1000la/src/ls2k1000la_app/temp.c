// �¶����


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


#define TEMP_AD_MAX                 ((0x1<<10)-1)       // ad�����ֵ��ad��ʮλ��
#define TEMP_IS_VALID_AD(ad)        ((TEMP_AD_MAX>=(ad)) && (0<=(ad)))       // �ж�ad�������̷�Χ��
#define TEMP_BUFF_SIZE              (64)                // �����С

// �¶ȵ����ֵ����Сֵ
// PLA�ĲĵĴ�ӡ�¶�[195,230]
// ABS�ĲĵĴ�ӡ�¶�[220-250]
#define TEMP_MIN                    (0)
#define TEMP_MAX                    (270)

// ����ͷ��ɢ�ȷ��ȵ�pwm����(ms)
#define HEATER_FAN_PWM_TIME_MS              (200)   // 200ms


enum
{
    FAN_NOT_WORK = 0,       // ��ɢ��(���Ȳ�����)
    FAN_WORK = 1,           // ɢ��(������������)
};


#ifdef PID_DEBUG
#define pid_debug_printf(format,...)        printf(format, ##__VA_ARGS__);
#else
#define pid_debug_printf(format,...)
#endif



// TM7705��ͨ��
enum
{
    TM7705_CH_1 = 0,            // ͨ��1
    TM7705_CH_2 = 1             // ͨ��2
};


// ���¸���ntc������������ýű����ɵ�adcֵ���¶�һһ��Ӧ�ı��
// ���Ϊadcֵ���ұ�Ϊ�¶�(��λ:���϶�)
// ��ϸ��ο�Դ��Ŀ¼�еĽű�"createTemperatureLookup.py"
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


// һ��pwm�����ڣ�������ʱ������λms
// ���������в��ܽ��и������
// ������Ӧ�ó����н�pid��ռ�ձ�ֱ�ӻ���Ϊһ�������ڼ���ͷ��ɢ�ȵķ��ȹ�����ʱ��
typedef struct{
    unsigned int extruder_heater_ms;        // ���������ȵ�ʱ��(ms)
    unsigned int extruder_fan_ms;           // ���������ȹ�����ʱ��(ms)
}heater_fan_work_time_t;


// ����ͷ�ͷ��ȵ��豸�ļ�
int fd_heater_fan;

// pid���ں��ڴ��ĺ����¶�
// ����ͷ��Ŀ���¶�
// Ĭ��Ϊ0����ӡ��Ϻ�Ҳ������Ϊ0
float extruder_target_temp = 0;

// ����ͷ�ĵ�ǰ�¶�
float extruder_current_temp;


// pidϵ��
float Kp = DEFUALT_Kp;
float Ki = DEFAULT_Ki;
float Kd = DEFAULT_Kd;
float Kc = DEFAULT_Kc;


// ��ȡָ��ͨ����adֵ
// @channel ͨ����
// @adc_p ������adֵ
// @ret �ɹ� or ʧ��
int temp_get_ad(int channel, UINT16 *adc_p)
{
    const char ch1_path[] = {"/sys/bus/spi/drivers/TM7705/spi0.1/ch1"};
    const char ch2_path[] = {"/sys/bus/spi/drivers/TM7705/spi0.1/ch2"};
    const char *dev_file_path = NULL;
    int fd = 0;
    int ret = 0;
    unsigned int value = 0;
    char buff[TEMP_BUFF_SIZE] = {0};

    // ��ͬ��ͨ����Ӧ/sys�²�ͬ���ļ�
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
    *adc_p = value;             // ���adֵ

    return SUCCESS;
}


// ����adcֵ�����¶�ֵ
// ntc�����������ֵ�¶����߱���Ϊn�Σ�ÿ�ο��Խ���Ϊֱ�ߣ�
// �����¶�ֵ�ļ����ת��Ϊ����ټ���
// @ad adֵ(ȡֵ��ΧΪ0-1023)
// @temp_p �¶�ֵ����λ���϶�
// @ret �ɹ� or ʧ��
int temp_calc_from_ad(UINT16 ad, float *temp_p)
{
    float celsius = 0.0;        // �¶�ֵ����λ���϶�
    int i = 0;
    
    // �ж�adcֵ�Ƿ������̷�Χ��
    if (!TEMP_IS_VALID_AD(ad))
    {
        return ERROR;
    }

    // �ж��Ƿ��ڱ������ʾ�ķ�Χ��
    if (ad < temptable[0][0])               // С�ڱ�����Сadc
    {
        *temp_p = temptable[0][1];          // ȡ��Сֵ
        return SUCCESS;
    }
    if (ad > temptable[NUMTEMPS-1][0])      // ���ڱ������adc
    {
        *temp_p = temptable[NUMTEMPS-1][1]; // ȡ���ֵ
        return SUCCESS;
    }

    // ���
    // �����Ǵ�adc�ɵ͵��ߣ����������бȽϣ�û�в����۰����
    for (i=1; i<NUMTEMPS; i++)              // ע�⣬�����Ǵ�1��ʼ�ģ�����֮����������
    {
        if (ad < temptable[i][0])           // �ж��Ƿ����������
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


// �Ӵ�������ȡ�¶�ֵ
// @temp_p �¶�ֵ����λ���϶�
// @ret �ɹ� or ʧ��
int temp_get_from_sensor(float *temp_p)
{
    UINT16 ad = 0;
    int ret = ERROR;

    // ��ȡadֵ
    ret = temp_get_ad(TM7705_CH_1, &ad);
    if (SUCCESS != ret)
    {
        printf("[%s] get channel 1's ad fail.\n", __FUNCTION__);
        return ret;
    }
//    printf("[%s] ad=%u\n", __FUNCTION__, ad);

    // ����ad�����¶�ֵ
    return temp_calc_from_ad(ad, temp_p);
}


// ��ʼ��
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


// ֹͣ����
void temp_stop_heater(void)
{
    heater_fan_work_time_t work_time = {0};
    
    work_time.extruder_heater_ms = 0;
    work_time.extruder_fan_ms = HEATER_FAN_PWM_TIME_MS;
    write(fd_heater_fan, &work_time, sizeof(heater_fan_work_time_t));

    return ;
}


// ���ݵ�ǰ�¶ȼ���pidֵ
// @current_temp ��ǰ�¶�
// @ret pidֵ��ȡֵ��Χ[0,1]
float temp_get_pid(float current_temp)
{
    float current_diff;                     // ��ǰ�¶���Ŀ���¶ȵĲ�ֵ
    float target_temp;                      // Ŀ���¶�
    static float last_diff = 0.0;           // ��һ�ε� ��ǰ�¶���Ŀ���¶ȵĲ�ֵ
    float pid;                              // ����õ���pidֵ
    float p_term = 0.0;                     // ������
    static float i_term = 0.0;              // ������
    static float d_term = 0.0;              // ΢����
    float c_term = 0.0;                     // ά�ֹ�����
    static BOOL i_reset = FALSE;            // �Ƿ�λ���ּ������
    static float diff_sum = 0.0;            // ƫ���ۼӺ�
    float diff_sum_min = PID_MIN;           // ƫ���ۼӺ͵�����
    float diff_sum_max = PID_MAX/Ki;        // ƫ���ۼӺ͵�����

    // ����ƫ��
    target_temp = extruder_target_temp;
    current_diff = target_temp - current_temp;

    // ����΢���ͬʱ��΢����ƽ���˲�
    // ��ƽ���˲��Ļ�������d_term = Kd * (current_diff - last_diff)
    d_term = PID_SMOOTHING_FACTOR * d_term +(1.0-PID_SMOOTHING_FACTOR) * Kd * (current_diff - last_diff);
    last_diff = current_diff;       // ����

    // ��ǰ�¶Ȼ�Զ����Ŀ���¶�ʱ��ȫ�ټ���
    if (PID_FUNCTIONAL_RANGE < current_diff)
    {
        pid = PID_LONG_TIME_ACTION;
        i_reset = TRUE;
        pid_debug_printf("[%s] target_temp=%f, current_temp=%f, pid=%f, current temp is too low, heater all work\n",
               __FUNCTION__, target_temp, current_temp, pid);
        return pid;
    }
    // ��ǰ�¶�Զ����Ŀ���¶�ʱ��������
    else if (-PID_FUNCTIONAL_RANGE > current_diff || 0 == target_temp)
    {
        pid = PID_MIN;
        i_reset = TRUE;
        pid_debug_printf("[%s] target_temp=%f, current_temp=%f, pid=%f, current temp is too high, heater not work\n",
               __FUNCTION__, target_temp, current_temp, pid);
        return pid;
    }
    // ��ǰ�¶���Ŀ���¶ȸ���ʱ��ʹ��pid�㷨�����¶�
    else
    {
        // �ж��Ƿ�λ���ּ������
        // ���¶�ƫ��������[-PID_FUNCTIONAL_RANGE, PID_FUNCTIONAL_RANGE]�ڲŻ���
        if (TRUE == i_reset)
        {
            i_reset = FALSE;
            diff_sum = 0.0;             // ��λƫ���ۼӺ�
        }
        
        // ���������
        p_term = Kp * current_diff;

        // ���������
        diff_sum += current_diff;
        diff_sum = constrain_float(diff_sum, diff_sum_min, diff_sum_max);   // ���ƻ�����ķ�Χ
        i_term = Ki * diff_sum;

        // ����ά�ֹ�����
        // ���¶Ȳ�ͬʱ��ɢ�ȿ���Ҳ��ͬ�����������ɢ��ά���¶ȵ����ã�������ϵͳ�ȶ���
        c_term = Kc * target_temp;

        // ����pid
        pid = p_term + i_term + d_term + c_term;

        // ���ַ���
        if (((PID_MAX < pid) && (0 < current_diff))
            || ((PID_MIN > pid) && (0 > current_diff)))
        {
            diff_sum -= current_diff;   // ȡ����ǰ�Ļ��֣�������һ�εĻ���
        }

        // ����pid�ķ�Χ
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


// ��pidת��Ϊpwmֵ������������ִ��
// @pid pidֵ��ȡֵ��Χ[0,1]
void temp_pid_to_pwm(float pid)
{
    heater_fan_work_time_t work_time = {0};

    work_time.extruder_heater_ms    = HEATER_FAN_PWM_TIME_MS*pid;
    work_time.extruder_fan_ms = HEATER_FAN_PWM_TIME_MS;
    write(fd_heater_fan, &work_time, sizeof(heater_fan_work_time_t));

    return ;
}


// �¶ȿ��Ƶ��߳�
// ͨ����ȡ��ǰ�¶ȣ�����pid�㷨���Ƽ��ȣ�ʹ�¶Ⱥ㶨��ָ����ֵ��
void *temp_thread_fn(void *arg)
{
    float temp = 0.0;   // �ɼ����¶�ֵ
    float pid = 0.0;    // �����¶ȼ���õ���pidֵ��ȡֵ��Χ[0,1]
    int ret = ERROR;
    
    if (SUCCESS != temp_init())
    {
        return NULL;
    }
    
    while (1)
    {
        usleep(500*1000);       // 500ms

        // �ɼ��¶�
        ret = temp_get_from_sensor(&temp);
        if (SUCCESS != ret)
        {
            // �ɼ�ʧ��
            temp_stop_heater();         // ֹͣ���ȣ�����ɢ�ȷ���
            printf("[%s] temp_get fail. ret=%d\n", __FUNCTION__, ret);
            continue;
        }
        extruder_current_temp = temp;
        printf("[%s] current_temp=%f\n", __FUNCTION__, temp);

        // ����pid
        pid = temp_get_pid(temp);

        // ��pidת��Ϊpwmֵ������������ִ��
        temp_pid_to_pwm(pid);
    }
}



// ��ȡ����ͷ��ǰ�¶�
float temp_get_extruder_current_temp(void)
{
    return extruder_current_temp;
}


// ���ü���ͷĿ���¶�
// @target_temp ����ͷĿ���¶�
void temp_set_extruder_target_temp(float target_temp)
{
    // �ж�Ŀ���¶��Ƿ��ں���Χ��
    if ((TEMP_MIN>target_temp) || (TEMP_MAX<target_temp))
    {
        printf("[%s] target_temp=%f is invalid.\n", __FUNCTION__, target_temp);
        return ;
    }
    
    extruder_target_temp = target_temp;
    return ;
}


// ��ӡ����ͷĿ���¶�
void temp_print_extruder_target_temp(void)
{
    printf("extruder target temp=%f\n", extruder_target_temp);
    return ;
}


// ����PIDϵ��Kp
void temp_set_Kp(float new_Kp)
{
    Kp = new_Kp;
    return ;
}


// ����PIDϵ��Ki
void temp_set_Ki(float new_Ki)
{
    Ki = new_Ki;
    return ;
}

// ����PIDϵ��Kd
void temp_set_Kd(float new_Kd)
{
    Kd = new_Kd;
    return ;
}

// ����PIDϵ��Kc
void temp_set_Kc(float new_Kc)
{
    Kc = new_Kc;
    return ;
}

// ��ӡPIDϵ��
void temp_print_Kp_Ki_Kd_Kc(void)
{
    printf("PID para: Kp=%f, Ki=%f, Kd=%f, Kc=%f\n", Kp, Ki, Kd, Kc);
    return ;
}


// ������ȴ����
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


// �ر���ȴ����
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


