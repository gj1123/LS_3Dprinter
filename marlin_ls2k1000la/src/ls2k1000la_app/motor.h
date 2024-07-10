// 提供电机驱动相关接口


#ifndef __MOTOR_H
#define __MOTOR_H


#include <stdio.h>
#include "printer.h"


// This struct is used when buffering the setup for each linear movement "nominal" values are as specified in
// the source g-code and may never actually be reached if acceleration management is active.
typedef struct {
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


// 结构体中某个成员固定为此值，用于判断结构体内容是否被破坏
#define STRUCT_CHECK_FLAG                       (0xe5e5)



// 电机初始化
int motor_init(void);

// 判断驱动中的block缓存是否已经满了
// @ret TRUE or FALSE
int motor_block_fifo_is_full(void);

// fifo中所有step都已执行
// fifo总没有block，并且最后一个block也已全部执行完成
// @ret TRUE or FALSE
int motor_buffered_steps_is_executed(void);


// Block until all buffered steps are executed
void motor_synchronize(void);


// 向驱动写入block
int motor_write_block(motor_cmd_block_t *block_p);


void motor_test(void);


#endif
