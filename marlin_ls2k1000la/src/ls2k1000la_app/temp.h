
// 温度相关


#ifndef __TEMP_H
#define __TEMP_H


#include <stdio.h>



// 温度控制的线程
// 通过读取当前温度，再用pid算法控制加热，使温度恒定在指定的值上
void *temp_thread_fn(void *arg);

// 设置挤出头目标温度
// @target_temp 挤出头目标温度
void temp_set_extruder_target_temp(float target_temp);

// 获取挤出头当前温度
float temp_get_extruder_current_temp(void);

// 打印挤出头目标温度
void temp_print_extruder_target_temp(void);

// 设置PID系数Kp
void temp_set_Kp(float new_Kp);

// 设置PID系数Ki
void temp_set_Ki(float new_Ki);

// 设置PID系数Kd
void temp_set_Kd(float new_Kd);

// 设置PID系数Kc
void temp_set_Kc(float new_Kc);

// 打印PID系数
void temp_print_Kp_Ki_Kd_Kc(void);

// 开启冷却风扇
void temp_cooling_fan_enable(void);

// 关闭冷却风扇
void temp_cooling_fan_disable(void);



#endif

