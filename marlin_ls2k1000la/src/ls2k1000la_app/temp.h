
// �¶����


#ifndef __TEMP_H
#define __TEMP_H


#include <stdio.h>



// �¶ȿ��Ƶ��߳�
// ͨ����ȡ��ǰ�¶ȣ�����pid�㷨���Ƽ��ȣ�ʹ�¶Ⱥ㶨��ָ����ֵ��
void *temp_thread_fn(void *arg);

// ���ü���ͷĿ���¶�
// @target_temp ����ͷĿ���¶�
void temp_set_extruder_target_temp(float target_temp);

// ��ȡ����ͷ��ǰ�¶�
float temp_get_extruder_current_temp(void);

// ��ӡ����ͷĿ���¶�
void temp_print_extruder_target_temp(void);

// ����PIDϵ��Kp
void temp_set_Kp(float new_Kp);

// ����PIDϵ��Ki
void temp_set_Ki(float new_Ki);

// ����PIDϵ��Kd
void temp_set_Kd(float new_Kd);

// ����PIDϵ��Kc
void temp_set_Kc(float new_Kc);

// ��ӡPIDϵ��
void temp_print_Kp_Ki_Kd_Kc(void);

// ������ȴ����
void temp_cooling_fan_enable(void);

// �ر���ȴ����
void temp_cooling_fan_disable(void);



#endif

