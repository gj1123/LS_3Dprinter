// �ṩ���������ؽӿ�


#ifndef __MOTOR_H
#define __MOTOR_H


#include <stdio.h>
#include "printer.h"


// This struct is used when buffering the setup for each linear movement "nominal" values are as specified in
// the source g-code and may never actually be reached if acceleration management is active.
typedef struct {
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


// �ṹ����ĳ����Ա�̶�Ϊ��ֵ�������жϽṹ�������Ƿ��ƻ�
#define STRUCT_CHECK_FLAG                       (0xe5e5)



// �����ʼ��
int motor_init(void);

// �ж������е�block�����Ƿ��Ѿ�����
// @ret TRUE or FALSE
int motor_block_fifo_is_full(void);

// fifo������step����ִ��
// fifo��û��block���������һ��blockҲ��ȫ��ִ�����
// @ret TRUE or FALSE
int motor_buffered_steps_is_executed(void);


// Block until all buffered steps are executed
void motor_synchronize(void);


// ������д��block
int motor_write_block(motor_cmd_block_t *block_p);


void motor_test(void);


#endif
