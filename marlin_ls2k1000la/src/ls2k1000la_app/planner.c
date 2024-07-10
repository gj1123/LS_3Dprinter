#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "public.h"
#include "planner.h"
#include "printer.h"
#include "configuration.h"
#include "motor.h"


#define ONE_SECOND_TO_NS                    (1*1000*1000*1000)      // 1sת��Ϊns
#define TIMER_MIN_PERIOD_NS                 (333*1000)              // ��ʱ������С��ʱʱ��(��Ӧ��󲽽�����ٶ�)

// �����Ĳ�����λ
// �ڵ�������ƶ�1mm�Ĳ���
float axis_steps_per_unit[NUM_AXIS];
// ���������Ӧ�����ڵѿ�������ϵ��z����������Ӧ�Ĳ���
long current_step[NUM_AXIS];


// �����ٶ�
UINT32 fanSpeeds = 0;
// Min Feed Rate (mm/s)
FLOAT minimumfeedrate_mm_sec = 0.0;
// Min Travel Feed Rate (mm/s)
FLOAT mintravelfeedrate_mm_sec = 0.0;
// max feedrate in mm per second
FLOAT max_feedrate_mm_sec[NUM_AXIS];

//everything with less than this number of steps will be ignored as move and joined with the next movement
const UINT32 dropsegments = 5;

// Normal acceleration mm/s^2  DEFAULT ACCELERATION for all printing moves. M204 SXXXX
FLOAT acceleration_mm_s2;
// Retract acceleration mm/s^2 filament pull-back and push-forward while standing still in the other axes M204 TXXX
FLOAT retract_acceleration_mm_s2;
// Travel (non printing) acceleration mm/s^2  DEFAULT ACCELERATION for all NON printing moves. M204 MXXXX
FLOAT travel_acceleration_mm_s2;

UINT32 axis_steps_per_sqr_second[NUM_AXIS];


int plan_init(void)
{
    float tmp1[] = DEFAULT_AXIS_STEPS_PER_UNIT;
    int axis;

    for (axis=X_AXIS; axis<=E_AXIS; axis++)
    {
        current_step[axis] = 0;
        axis_steps_per_unit[axis] = tmp1[axis];
    }

    return SUCCESS;
}


/*
 * �ҳ����������Ǹ����
 * @x_steps, @y_steps, @z_steps, @e_steps ÿ������Ĳ���
 * @ret ���������Ǹ����
 */
int plan_get_max_steps_axis(long x_steps, long y_steps, long z_steps, long e_steps)
{
    long max_steps;
    int max_axis;

    max_steps = MAX(x_steps,
                    MAX(y_steps,
                        MAX(z_steps, e_steps)));

    if (max_steps == x_steps)
    {
        max_axis = X_AXIS;
    }
    else if (max_steps == y_steps)
    {
        max_axis = Y_AXIS;
    }
    else if (max_steps == z_steps)
    {
        max_axis = Z_AXIS;
    }
    else
    {
        max_axis = E_AXIS;
    }

    return max_axis;
}


// Add a new linear movement to the buffer. steps[X_AXIS], _y and _z is the absolute position in
// mm. Microseconds specify how many microseconds the move should take to perform. To aid acceleration
// calculation the caller must also provide the physical length of the line in millimeters.
// �滮·��
// x,y����˶������ٶȱ仯�ǳ�Ƶ����������������˶���
// �ٶȵ�ͻȻ�仯�����·�����ܴ�ĳ����Ӱ��������ȶ��ԡ�
// ���������ԭ���;������㷨��Ӧ�ò��ü��ټ��ٵ��˶��㷨��
// ·���滮����ζ�ţ�������ִ�в�������Ķ���֮ǰ��
// ���Ѿ���������������̵��ٶ����ߡ������ֻ��Stepperģ����ʵ��ִ�С�
// @dx_position, @dy_position, @dz_position ���������Ӧ�����ڵѿ�������ϵz��ֵ���������꣬��λmm
// @de_position ������Ҫ�����ĳ��ȣ���λmm
// @feedrate_mm_sec ���Ͻ����ʣ���λmm/s
// @check_endstop �Ƿ���Ҫ�����λ����״̬
void plan_buffer_line(FLOAT dx_position, 
                      FLOAT dy_position, 
                      FLOAT dz_position, 
                      FLOAT de_position, 
                      FLOAT feedrate_mm_sec,
                      unsigned char check_endstop)
{
    INT32 dx_step = 0;      // x�����Ҫ�ƶ��Ĳ���
    INT32 dy_step = 0;      // y�����Ҫ�ƶ��Ĳ���
    INT32 dz_step = 0;      // z�����Ҫ�ƶ��Ĳ���
    INT32 de_step = 0;      // ����ͷ�����Ҫ�ƶ��Ĳ���
    FLOAT delta_mm[NUM_AXIS] = {0.0};   // ���������Ҫ�ƶ��ľ��룬��λmm
    FLOAT inverse_millimeters = 0.0;    // ��block�ƶ��ľ���ĵ���
    FLOAT inverse_second = 0.0;         // ��block�ƶ���ʱ��ĵ���
    FLOAT current_speed_mm_sec[NUM_AXIS] = {0.0};   // ��ǰ�ٶȣ���λmm/sec
    FLOAT cs = 1.0;                     // ��ǰ�ٶȵ���ʱ����
    FLOAT mf = 1.0;                     // ���feedrate����ʱ����
    FLOAT speed_factor = 1.0;           //factor <=1 do decrease speed
    FLOAT steps_per_mm = 0.0;           // 
    UINT8 db = 0;       // ����bits
    INT32 i = 0;
    INT32 bsx, bsy, bsz, bse;               // block��x,y,z,e�Ĳ���
    UINT32 acc_step_s2;                     // ���ٶȣ���λstep/s^2
    UINT32 xsteps, ysteps, zsteps, esteps;  // ÿ�����step/s^2
    UINT32 allsteps;                        // ��block�е�step_event_count
    motor_cmd_block_t block = {0};
    int ret = 0;
    int max_steps_axis;                     // XYZE�в��������Ǹ�

    block.head_flag = STRUCT_CHECK_FLAG;
    block.end_flag = STRUCT_CHECK_FLAG;
    block.check_endstop = check_endstop;

    // The target position of the tool in absolute steps
    // Calculate target position in absolute steps
    //this should be done after the wait, because otherwise a M92 code within the gcode disrupts this calculation somehow
    // �����������Ӧ�����ڵѿ�������ϵ��z���������ת��Ϊ���Բ���
    long target_step[NUM_AXIS];
    target_step[X_AXIS] = lround(dx_position * axis_steps_per_unit[X_AXIS]);
    target_step[Y_AXIS] = lround(dy_position * axis_steps_per_unit[Y_AXIS]);
    target_step[Z_AXIS] = lround(dz_position * axis_steps_per_unit[Z_AXIS]);
    target_step[E_AXIS] = lround(de_position * axis_steps_per_unit[E_AXIS]);

    // ��������Ҫ�ƶ��Ĳ���(��Բ���)
    dx_step = target_step[X_AXIS] - current_step[X_AXIS];
    dy_step = target_step[Y_AXIS] - current_step[Y_AXIS];
    dz_step = target_step[Z_AXIS] - current_step[Z_AXIS];
    de_step = target_step[E_AXIS] - current_step[E_AXIS];
    
    // Number of steps for each axis
    // ÿ��(���)��Ĳ���
    block.steps[X_AXIS] = abs(dx_step);
    block.steps[Y_AXIS] = abs(dy_step);
    block.steps[Z_AXIS] = abs(dz_step);
    block.steps[E_AXIS] = abs(de_step);
    block.step_event_count = MAX(block.steps[X_AXIS], 
                                 MAX(block.steps[Y_AXIS], 
                                     MAX(block.steps[Z_AXIS], block.steps[E_AXIS])));
    // Bail if this is a zero-length block
    if (dropsegments >= block.step_event_count)
    {
        return ;
    }

    block.fan_speed = fanSpeeds;

    // Compute direction bits for this block
    if (0 < dx_step) SBI(db, X_AXIS);
    if (0 < dy_step) SBI(db, Y_AXIS);
    if (0 < dz_step) SBI(db, Z_AXIS);
    if (0 < de_step) SBI(db, E_AXIS);
    block.direction_bits = db;

    // �����feedrate_mm_sec���м��
    if (block.steps[E_AXIS])
    {
        NOLESS(feedrate_mm_sec, minimumfeedrate_mm_sec);
    }
    else
    {
        NOLESS(feedrate_mm_sec, mintravelfeedrate_mm_sec);
    }

    /**
     * This part of the code calculates the total length of the movement.
     * For cartesian bots, the X_AXIS is the real X movement and same for Y_AXIS.
     * But for corexy bots, that is not true. The "X_AXIS" and "Y_AXIS" motors (that should be named to A_AXIS
     * and B_AXIS) cannot be used for X and Y length, because A=X+Y and B=X-Y.
     * So we need to create other 2 "AXIS", named X_HEAD and Y_HEAD, meaning the real displacement of the Head.
     * Having the real displacement of the head, we can calculate the total movement length and apply the desired speed.
     */
    delta_mm[X_AXIS] = dx_step / axis_steps_per_unit[X_AXIS];
    delta_mm[Y_AXIS] = dy_step / axis_steps_per_unit[Y_AXIS];
    delta_mm[Z_AXIS] = dz_step / axis_steps_per_unit[Z_AXIS];
    delta_mm[E_AXIS] = de_step / axis_steps_per_unit[E_AXIS];
    if ((dropsegments >= block.steps[X_AXIS]) 
        && (dropsegments >= block.steps[Y_AXIS])
        && (dropsegments >= block.steps[Z_AXIS]))
    {
        block.millimeters_mm = fabs(delta_mm[E_AXIS]);
    }
    else
    {
        block.millimeters_mm = sqrt(SQUARE(delta_mm[X_AXIS]) 
                                     + SQUARE(delta_mm[Y_AXIS]) 
                                     + SQUARE(delta_mm[Z_AXIS]));
    }
    inverse_millimeters = 1.0 / block.millimeters_mm;      // Inverse millimeters to remove multiple divides

    // ʹ�ú����˶���û�мӼ��٣����������ʽ�ı��ٶȣ��������ٶȱȽ�Сʱ
    block.s_cure_used = FALSE;
    max_steps_axis = plan_get_max_steps_axis(block.steps[X_AXIS], block.steps[Y_AXIS], block.steps[Z_AXIS], block.steps[E_AXIS]);          // �ҳ����������Ǹ����
    // T=1/f, f=feedrate_mm_sec*axis_steps_per_unit������T=1/f=1/(feedrate_mm_sec*axis_steps_per_unit)
    block.timer_period_ns = ONE_SECOND_TO_NS/(feedrate_mm_sec*axis_steps_per_unit[max_steps_axis]);
    NOLESS(block.timer_period_ns, TIMER_MIN_PERIOD_NS);     // �������ٴ�ӡ������ٶ�Ϊ30mm/s
//    printf("[%s] max_steps_axis=%d, timer_period_ns=%ld\n", __FUNCTION__, max_steps_axis, block.timer_period_ns);


// -------�ݲ����ǼӼ���-------


    // Calculate moves/second for this move. No divide by zero due to previous checks.
    inverse_second = feedrate_mm_sec * inverse_millimeters;
    block.nominal_speed_mm_s = block.millimeters_mm * inverse_second;       // (mm/sec) Always > 0
    block.nominal_rate_step_s = ceil(block.step_event_count * inverse_second);     // (step/sec) Always > 0

    // Calculate and limit speed in mm/sec for each axis
    for (i=0; i<NUM_AXIS; i++)
    {
        current_speed_mm_sec[i] = delta_mm[i] * inverse_second;
        cs = fabs(current_speed_mm_sec[i]);
        mf = max_feedrate_mm_sec[i];
        if (cs > mf)
        {
            speed_factor = MIN(speed_factor, mf / cs);
        }
    }    
    // Correct the speed
    if (1.0 > speed_factor)
    {
        for (i=0; i<NUM_AXIS; i++) current_speed_mm_sec[i] *= speed_factor;
        block.nominal_speed_mm_s *= speed_factor;
        block.nominal_rate_step_s *= speed_factor;
    }

    // Compute and limit the acceleration rate for the trapezoid generator.
    steps_per_mm = block.step_event_count / block.millimeters_mm;
    bsx = block.steps[X_AXIS];
    bsy = block.steps[Y_AXIS];
    bsz = block.steps[Z_AXIS];
    bse = block.steps[E_AXIS];
    if ((0==bsx) && (0==bsy) && (0==bsz))
    {
        block.acceleration_st_step_s2 = ceil(retract_acceleration_mm_s2 * steps_per_mm);  // convert to: acceleration steps/sec^2
    }
    else if (0 == bse)
    {
        block.acceleration_st_step_s2 = ceil(travel_acceleration_mm_s2 * steps_per_mm);   // convert to: acceleration steps/sec^2
    }
    else
    {
        block.acceleration_st_step_s2 = ceil(acceleration_mm_s2 * steps_per_mm);  // convert to: acceleration steps/sec^2
    }
    // Limit acceleration per axis
    acc_step_s2 = block.acceleration_st_step_s2;
    xsteps = axis_steps_per_sqr_second[X_AXIS];
    ysteps = axis_steps_per_sqr_second[Y_AXIS];
    zsteps = axis_steps_per_sqr_second[Z_AXIS];
    esteps = axis_steps_per_sqr_second[E_AXIS];
    allsteps = block.step_event_count;
    if (xsteps < (acc_step_s2 * bsx) / allsteps) acc_step_s2 = (xsteps * allsteps) / bsx;
    if (ysteps < (acc_step_s2 * bsy) / allsteps) acc_step_s2 = (ysteps * allsteps) / bsy;
    if (zsteps < (acc_step_s2 * bsz) / allsteps) acc_step_s2 = (zsteps * allsteps) / bsz;
    if (esteps < (acc_step_s2 * bse) / allsteps) acc_step_s2 = (esteps * allsteps) / bse;

    block.acceleration_st_step_s2 = acc_step_s2;
    block.acceleration_mm_s2 = acc_step_s2 / steps_per_mm;
//    block.acceleration_rate = (INT32)(acc_step_s2 * 16777216.0 / ());


    // Update position
    for (i=0; i<NUM_AXIS; i++)
    {
        current_step[i] = target_step[i];
    }

    // �ж��Ƿ��пռ������block
    while (TRUE == motor_block_fifo_is_full())
    {
        // û�У���ȴ�
        printf("[%s] block buff is full\n", __FUNCTION__);
        usleep(10);
    }

    // ��������ִ��
    ret = motor_write_block(&block);
    if (SUCCESS != ret)
    {
        return ;
    }
    return ;
}


// ���������xyze���������޸���ر���
void plan_set_position(float x_abs_coordinate_mm,
                       float y_abs_coordinate_mm,
                       float z_abs_coordinate_mm,
                       float e_abs_coordinate_mm)
{
    current_step[X_AXIS] = lround(x_abs_coordinate_mm * axis_steps_per_unit[X_AXIS]);
    current_step[Y_AXIS] = lround(y_abs_coordinate_mm * axis_steps_per_unit[Y_AXIS]);
    current_step[Z_AXIS] = lround(z_abs_coordinate_mm * axis_steps_per_unit[Z_AXIS]);
    current_step[E_AXIS] = lround(e_abs_coordinate_mm * axis_steps_per_unit[E_AXIS]);

    return ;
}

