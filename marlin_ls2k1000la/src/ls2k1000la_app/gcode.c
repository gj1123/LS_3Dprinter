#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include "public.h"
#include "printer.h"
#include "configuration.h"
#include "planner.h"
#include "motor.h"
#include "temp.h"


#define TOWER_1 X_AXIS
#define TOWER_2 Y_AXIS
#define TOWER_3 Z_AXIS


#define SIN_60 0.8660254037844386
#define COS_60 0.5

#define DELTA_RADIUS_TRIM_TOWER_1 0.0
#define DELTA_RADIUS_TRIM_TOWER_2 0.0
#define DELTA_RADIUS_TRIM_TOWER_3 0.0
#define DELTA_DIAGONAL_ROD_TRIM_TOWER_1 0.0
#define DELTA_DIAGONAL_ROD_TRIM_TOWER_2 0.0
#define DELTA_DIAGONAL_ROD_TRIM_TOWER_3 0.0

#define AXIS_RELATIVE_MODES {FALSE, FALSE, FALSE, FALSE}


// Actual temperature must be close to target for this long before M109 returns success
#define TEMP_RESIDENCY_TIME 60  // (seconds)
#define TEMP_WINDOW     1       // (degC) Window around target to start the residency timer x degC early.


/* xyz��������ᰴ��ʱ�뷽�����У�����
 *    z
 *  x   y
 * ��Ϊxyz����ṹ��һ���ȱ������Σ������������޴�ӡ����ת����Եõ����£���Ҳ�ǿ��Ե�
 *    x
 *  y   z
 * �����Ǹ���xyz����ʱ��ֲ���ԭ��
 * �ѿ����ռ��xyz�����е�x����xy�����ƽ�У����Ҿ����ȱ�������Բ��
 */
 
// ��ǰ���ĵ�����ڵѿ�������ϵ��xֵ
float delta_tower1_x;       // front left tower
// ��ǰ���ĵ�����ڵѿ�������ϵ��yֵ
float delta_tower1_y;
// ��ǰ���ĵ�����ڵѿ�������ϵ��xֵ
float delta_tower2_x;       // front right tower
// ��ǰ���ĵ�����ڵѿ�������ϵ��yֵ
float delta_tower2_y;
// �����м���Ǹ�������ڵѿ�������ϵ��xֵ
float delta_tower3_x;       // back middle tower
// �����м���Ǹ�������ڵѿ�������ϵ��yֵ
float delta_tower3_y;

// these are the default values, can be overriden with M665
float delta_radius = DELTA_RADIUS;

// �Ƹ˳�
float delta_diagonal_rod = DELTA_DIAGONAL_ROD;
float delta_diagonal_rod_trim_tower_1 = DELTA_DIAGONAL_ROD_TRIM_TOWER_1;
float delta_diagonal_rod_trim_tower_2 = DELTA_DIAGONAL_ROD_TRIM_TOWER_2;
float delta_diagonal_rod_trim_tower_3 = DELTA_DIAGONAL_ROD_TRIM_TOWER_3;
float delta_diagonal_rod_2_tower_1;
float delta_diagonal_rod_2_tower_2;
float delta_diagonal_rod_2_tower_3;


const char axis_codes[NUM_AXIS] = {'X', 'Y', 'Z', 'E'};
BOOL axis_relative_modes[] = AXIS_RELATIVE_MODES;
/* 
 * Determines Absolute or Relative Coordinates
 * FALSE:��������ģʽ, TRUE:�������ģʽ
 */
BOOL relative_mode = FALSE;  

float min_pos[3] = { X_MIN_POS, Y_MIN_POS, Z_MIN_POS };
float max_pos[3] = { X_MAX_POS, Y_MAX_POS, Z_MAX_POS };
float z_max_height = Z_MAX_POS-Z_MIN_POS;               // ���Ĵ�ӡ�߶�
float endstop_adj[3] = {0.0};

float delta_segments_per_second = DELTA_SEGMENTS_PER_SECOND;

char* seen_pointer; ///< A pointer to find chars in the command string (X, Y, Z, E, etc.)


// ��ǰ����
// �ѿ����ռ�������꣬��λmm
// �����ʼ�˶��ˣ�current_abs_coordinate_mm����
// ��һ��prepare_move()ѭ��ִ�к���һ�ε�destination_abs_coordinate_mm��ֵ
float current_abs_coordinate_mm[NUM_AXIS] = { 0.0};
// ��ָ��G1�н���������Ŀ������
// �ѿ����ռ�������꣬��λmm
float destination_abs_coordinate_mm[NUM_AXIS] = { 0.0 };
// ���������Ӧ�����ڵѿ�������ϵz��ֵ
// �������꣬��λmm
float carriage_abs_z_coordinate_mm[3] = { 0 };


// ��ָ��G1�н��������Ĵ�ӡ�ٶȣ���λmm/min
float feedrate_mm_min = HOME_FEEDRATE_MM_MIN;

// ��ǰ����Ĳ���
char *gcode_current_cmd_args = NULL;


void gcode_M114(void);
void gcode_parse_one_cmd(char *cmd_ptr);


// ���¼���
void recalc_delta_settings(void)
{
    delta_tower1_x = -SIN_60 * (delta_radius + DELTA_RADIUS_TRIM_TOWER_1);  // front left tower
    delta_tower1_y = -COS_60 * (delta_radius + DELTA_RADIUS_TRIM_TOWER_1);
    delta_tower2_x =  SIN_60 * (delta_radius + DELTA_RADIUS_TRIM_TOWER_2);  // front right tower
    delta_tower2_y = -COS_60 * (delta_radius + DELTA_RADIUS_TRIM_TOWER_2);
    delta_tower3_x = 0;     // back middle tower
    delta_tower3_y = (delta_radius + DELTA_RADIUS_TRIM_TOWER_3);

    delta_diagonal_rod_2_tower_1 = SQUARE(delta_diagonal_rod + delta_diagonal_rod_trim_tower_1);
    delta_diagonal_rod_2_tower_2 = SQUARE(delta_diagonal_rod + delta_diagonal_rod_trim_tower_2);
    delta_diagonal_rod_2_tower_3 = SQUARE(delta_diagonal_rod + delta_diagonal_rod_trim_tower_3);

    return ;
}


// �������ļ��м���������Ϣ
void gcode_load_setting(void)
{
    // �ֶ���ƽ�õ���ֵ
    delta_radius = 97;

    endstop_adj[X_AXIS] = -4.7;
    endstop_adj[Y_AXIS] = -12.8;
    endstop_adj[Z_AXIS] = -1.7;
}


// ��ʼ��
void gcode_init(void)
{
    // ����������Ϣ
    gcode_load_setting();

    // ���¼����������
    recalc_delta_settings();

    return ;
}


// �жϵ�ǰ����������Ƿ��йؼ���X,Y,Z,E,F......
BOOL code_seen(char code)
{
    seen_pointer = strchr(gcode_current_cmd_args, code);

    if (NULL != seen_pointer)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


// ��ȡ��ǰ��������йؼ���X,Y,Z,E,F...��Ӧ������
float code_value(void)
{
    float ret = 0.0;

    ret = strtod(seen_pointer+1, NULL);

    return ret;
}


// ���µ�ǰ����
void set_current_to_destination(void)
{
    memcpy(current_abs_coordinate_mm, destination_abs_coordinate_mm, sizeof(current_abs_coordinate_mm));

    return ;
}


/**
 * Set XYZE destination and feedrate from the current GCode command
 *
 *  - Set destination from included axis codes
 *  - Set to current for missing axis codes
 *  - Set the feedrate, if included
 */
void gcode_get_destination(void)
{
    int i=0;
    float next_feedrate = 0.0;

    // X Y Z E
    for (i=0; i<NUM_AXIS; i++)
    {
        if (TRUE == code_seen(axis_codes[i]))
        {
            destination_abs_coordinate_mm[i] = code_value() + (axis_relative_modes[i] || relative_mode ? current_abs_coordinate_mm[i] : 0);
        }
        else
        {
            destination_abs_coordinate_mm[i] = current_abs_coordinate_mm[i];
        }
    }

    // F
    if (code_seen('F'))
    {
        next_feedrate = code_value();
        if (0.0 < next_feedrate)
        {
            feedrate_mm_min = next_feedrate;
        }
    }

    return ;
}


// ��xyzĿ�����������ڴ�ӡ����ķ�Χ��
void clamp_to_software_endstops(float target[3])
{
    NOLESS(target[X_AXIS], min_pos[X_AXIS]);
    NOLESS(target[Y_AXIS], min_pos[Y_AXIS]);
//    NOLESS(target[Z_AXIS], min_pos[Z_AXIS]);      // �ֶ���ƽʱ��������z<0

    NOMORE(target[X_AXIS], max_pos[X_AXIS]);
    NOMORE(target[Y_AXIS], max_pos[Y_AXIS]);
    NOMORE(target[Z_AXIS], max_pos[Z_AXIS]);

    return ;
}


// ����ӡ���ĵѿ�����������ת��Ϊxyz�������˶���
// @cartesian �ѿ�����������xyz��ֵ
void calculate_delta(float cartesian[3])
{
    carriage_abs_z_coordinate_mm[TOWER_1] = sqrt(delta_diagonal_rod_2_tower_1
                                              - SQUARE(delta_tower1_x - cartesian[X_AXIS])
                                              - SQUARE(delta_tower1_y - cartesian[Y_AXIS])
                                             ) + cartesian[Z_AXIS];
    carriage_abs_z_coordinate_mm[TOWER_2] = sqrt(delta_diagonal_rod_2_tower_2
                                              - SQUARE(delta_tower2_x - cartesian[X_AXIS])
                                              - SQUARE(delta_tower2_y - cartesian[Y_AXIS])
                                             ) + cartesian[Z_AXIS];
    carriage_abs_z_coordinate_mm[TOWER_3] = sqrt(delta_diagonal_rod_2_tower_3
                                              - SQUARE(delta_tower3_x - cartesian[X_AXIS])
                                              - SQUARE(delta_tower3_y - cartesian[Y_AXIS])
                                             ) + cartesian[Z_AXIS];

    return ;
}


// delta�ṹ��prepare_move
// @target_abs �ѿ����ռ�Ŀ������(��������)
// ����������ϸ������ٶ�"Delta3D��ӡ���������������ĵ�"
BOOL prepare_move_delta(float target_abs[NUM_AXIS])
{
    float difference[NUM_AXIS];     // Ŀ�������뵱ǰ����Ĳ�ֵ����λmm
    float cartesian_mm = 0;         // Ŀ�������뵱ǰ����ĵѿ�������
    float seconds = 0.0;            // �ӵ�ǰ�����˶���Ŀ��������Ҫ��ʱ��
    INT32 segments = 0;             // �ӵ�ǰ���굽Ŀ����������·�����ָ��segments��
    UINT8 i = 0;
    UINT32 s = 0;
    float fraction = 0.0;

    // ����Ŀ������͵�������ĵѿ�������
    for (i=0; i<NUM_AXIS; i++)
    {
        difference[i] = target_abs[i] - current_abs_coordinate_mm[i];            // �������λ��ֵ(�����)
    }
    cartesian_mm = sqrt(SQUARE(difference[X_AXIS])                  // ����ѿ����ռ����
                        + SQUARE(difference[Y_AXIS]) 
                        + SQUARE(difference[Z_AXIS]));
    if (0.000001 > cartesian_mm) 
        cartesian_mm = abs(difference[E_AXIS]);    // ���XYZ��ľ����С������E��
    if (0.000001 > cartesian_mm)
        return FALSE;

    // ���ݾ�����ٶȼ���ִ��ʱ��
    // cartesian_mm ��λmm
    // feedrate_mm_min ��λmm/min
    // feedrate_multiplier_percent ��λ�ٷֱ�
    seconds = cartesian_mm / (feedrate_mm_min/60);
    // ������Ҫ�Ĳ���
    // 1���step��Ϊ���ϸ��ִ��
    segments = MAX(1, (UINT32)(delta_segments_per_second*seconds));

    // ���δ���ÿһ��
    for (s=1; s<=segments; s++)
    {
        // ֱ�߲�ֵ
        fraction = (float)s / (float)segments;
        for (i=0; i<NUM_AXIS; i++)
        {
            target_abs[i] = current_abs_coordinate_mm[i] + difference[i] * fraction;
        }

        // ����ӡ���ĵѿ�����������ת��������ֱ�ĵ������˶�����
        calculate_delta(target_abs);

        // �滮·��
        // x,y����˶������ٶȱ仯�ǳ�Ƶ����������������˶���
        // �ٶȵ�ͻȻ�仯�����·�����ܴ�ĳ����Ӱ��������ȶ��ԡ�
        // ���������ԭ���;������㷨��Ӧ�ò��ü��ټ��ٵ��˶��㷨��
        // ·���滮����ζ�ţ�������ִ�в�������Ķ���֮ǰ��
        // ���Ѿ���������������̵��ٶ����ߡ������ֻ��Stepperģ����ʵ��ִ�С�
        plan_buffer_line(carriage_abs_z_coordinate_mm[X_AXIS], 
                         carriage_abs_z_coordinate_mm[Y_AXIS], 
                         carriage_abs_z_coordinate_mm[Z_AXIS], 
                         target_abs[E_AXIS], 
                         feedrate_mm_min/60,    // �����ʵ�λmm/sec
                         TRUE);
    }

    return TRUE;
}


/**
 * Prepare a single move and get ready for the next one
 *
 * (This may call plan_buffer_line several times to put
 *  smaller moves into the planner for DELTA or SCARA.)
 */
void prepare_move(void)
{
    // ��xyzĿ�����������ڴ�ӡ����ķ�Χ��
    // destination�Ǿ���ֵ����λmm
    clamp_to_software_endstops(destination_abs_coordinate_mm);

    // delta�ṹ��prepare_move
    if (TRUE != prepare_move_delta(destination_abs_coordinate_mm))
    {
        return ;
    }

    // ���µ�ǰ����
    set_current_to_destination();

    return ;
}


// G0,G1: Coordinated movement of X Y Z E axes
void gcode_G0_G1(void)
{
    gcode_get_destination();        // For X Y Z E F
#if 0
    printf("[%s] ", __FUNCTION__);
    if (code_seen('X'))
        printf("X=%fmm, ", destination_abs_coordinate_mm[0]);
    if (code_seen('Y'))
        printf("Y=%fmm, ", destination_abs_coordinate_mm[1]);
    if (code_seen('Z'))
        printf("Z=%fmm, ", destination_abs_coordinate_mm[2]);
    if (code_seen('E'))
        printf("E=%fmm, ", destination_abs_coordinate_mm[3]);
    if (code_seen('F'))
        printf("F=%fmm/min", feedrate_mm_min);
    printf("\n");
#endif
    prepare_move();

    return ;
}


// ͬ���޸�plan���λ��
void sync_plan_position(void)
{
    plan_set_position(current_abs_coordinate_mm[X_AXIS],
                      current_abs_coordinate_mm[Y_AXIS],
                      current_abs_coordinate_mm[Z_AXIS],
                      current_abs_coordinate_mm[E_AXIS]);

    return ;
}


void line_to_destination(unsigned char check_endstop)
{
    plan_buffer_line(destination_abs_coordinate_mm[X_AXIS],
                     destination_abs_coordinate_mm[Y_AXIS],
                     destination_abs_coordinate_mm[Z_AXIS],
                     destination_abs_coordinate_mm[E_AXIS],
                     feedrate_mm_min/60,    // �����ʵ�λmm/sec
                     check_endstop);

    return ;
}


void set_homing_bump_feedrate(enum AxisEnum axis)
{
    int homing_bump_divisor[] = HOMING_BUMP_DIVISOR;
    int hbd = homing_bump_divisor[axis];

    if (1 > hbd)
    {
        hbd = 10;
        printf("[%s] Warning: Homing Bump Divisor < 1\n", __FUNCTION__);
    }
    feedrate_mm_min = HOME_FEEDRATE_MM_MIN/hbd;

    return ;
}


void set_axis_is_at_home(enum AxisEnum axis)
{
    // ������3D��ӡ������������Ϊ(0,0,z_max_height)
    if ((X_AXIS==axis) || (Y_AXIS==axis))
    {
        current_abs_coordinate_mm[axis] = 0;
    }
    else if (Z_AXIS==axis)
    {
        current_abs_coordinate_mm[Z_AXIS] = z_max_height;
    }
    else
    {
        // E_AXIS 
        // ��������������
    }

    return ;
}


/**
 * Home an individual axis
 * ����ĳһ�������
 */
void homeaxis(enum AxisEnum axis)
{
    // Set the axis position as setup for the move
    current_abs_coordinate_mm[axis] = 0;
    sync_plan_position();

    // Move towards the endstop until an endstop is triggered
    // ����λ������ֱ��������λ��Ϊֹ
    destination_abs_coordinate_mm[axis] = 1.5*z_max_height;
    feedrate_mm_min = HOME_FEEDRATE_MM_MIN;
    line_to_destination(NEED_CHECK_ENDSTOP);
    motor_synchronize();            // Wait for planner moves to finish!

    // Set the axis position as setup for the move
    current_abs_coordinate_mm[axis] = 0;
    sync_plan_position();

    // Move away from the endstop by the axis HOME_BUMP_MM
    // �뿪��λ��һС�ξ���
    destination_abs_coordinate_mm[axis] = -HOME_BUMP_MM;
    line_to_destination(NOT_CHECK_ENDSTOP);
    motor_synchronize();            // Wait for planner moves to finish!

    // Slow down the feedrate for the next move
    set_homing_bump_feedrate(axis);

    // Move slowly towards the endstop until triggered
    // �ٴλ�������λ���ƶ�ֱ��������λ��Ϊֹ
    destination_abs_coordinate_mm[axis] = 2*HOME_BUMP_MM;
    line_to_destination(NEED_CHECK_ENDSTOP);
    motor_synchronize();            // Wait for planner moves to finish!

    // retrace by the amount specified in endstop_adj
    if (0 > endstop_adj[axis])
    {
        // Set the axis position as setup for the move
        current_abs_coordinate_mm[axis] = 0;
        sync_plan_position();
        
        destination_abs_coordinate_mm[axis] = endstop_adj[axis];
        line_to_destination(NOT_CHECK_ENDSTOP);
        motor_synchronize();            // Wait for planner moves to finish!
    }

    // Set the axis position to its home position
    set_axis_is_at_home(axis);
    sync_plan_position();
    destination_abs_coordinate_mm[axis] = current_abs_coordinate_mm[axis];
    feedrate_mm_min = 0.0;

    return ;
}


void sync_plan_position_delta(void)
{
    calculate_delta(current_abs_coordinate_mm);
    plan_set_position(carriage_abs_z_coordinate_mm[X_AXIS],
                      carriage_abs_z_coordinate_mm[Y_AXIS],
                      carriage_abs_z_coordinate_mm[Z_AXIS],
                      current_abs_coordinate_mm[E_AXIS]);

    return ;
}


/*
 * �ж��Ƿ�Ϊ��ָ��
 * ��������Ƿ�Ϊ���ַ���
 * @cmd_buff �����
 */
BOOL gcode_is_empty_cmd(char *cmd_buff)
{
    if (0 == strlen(cmd_buff))
        return TRUE;
    else
        return FALSE;
}


// G21: Set Units to Millimeters ʹ�ú�����Ϊ��λ
void gcode_G21(void)
{
    // Ĭ��ʹ�ú�����Ϊ��λ������Ĭ��Ϊ�պ�����������
    return ;
}


// G28: �ƶ���ԭ��
void gcode_G28(void)
{
    int i = 0;
    float saved_feedrate_mm_min = 0;
    
    // Wait for planner moves to finish!
    motor_synchronize();

    // ���滷�����������ָ�
    saved_feedrate_mm_min = feedrate_mm_min;

    /**
     * A delta can only safely home all axis at the same time
     * all axis have to home at the same time
     */
    
    // Pretend the current position is 0,0,0
    for (i=X_AXIS; i<=Z_AXIS; i++)
    {
        current_abs_coordinate_mm[i] = 0;
    }
    sync_plan_position();

    // Move all carriages up together until the first endstop is hit.
    for (i=X_AXIS; i<=Z_AXIS; i++)
    {
        destination_abs_coordinate_mm[i] = 3*z_max_height;
    }
    feedrate_mm_min = 1.732 * HOME_FEEDRATE_MM_MIN;
    line_to_destination(NEED_CHECK_ENDSTOP);
    motor_synchronize();            // Wait for planner moves to finish!

    // Destination reached
    for (i=X_AXIS; i<=Z_AXIS; i++)
    {
        current_abs_coordinate_mm[i] = destination_abs_coordinate_mm[i];
    }

    // take care of back off and rehome now we are all at the top
    homeaxis(X_AXIS);
    homeaxis(Y_AXIS);
    homeaxis(Z_AXIS);

    sync_plan_position_delta();

    // �ָ�����
    feedrate_mm_min = saved_feedrate_mm_min;

    // Send end position to RepetierHost
    gcode_M114();

    return ;
}


/*
 * ���õ�ǰ����ģʽΪ��������ģʽ
 * ϵͳĬ��Ϊ��������ģʽ
 */
void gcode_G90(void)
{
    relative_mode = FALSE;
    printf("[%s] set abs coordinate mode.\n", __FUNCTION__);
}


// ���õ�ǰ����ģʽΪ�������ģʽ
void gcode_G91(void)
{
    relative_mode = TRUE;
    printf("[%s] set relative coordinate mode.\n", __FUNCTION__);
}


// G92: ���嵱ǰλ��
void gcode_G92(void)
{
    int i;

    for (i=0; NUM_AXIS>i; i++)
    {
        if (code_seen(axis_codes[i]))
        {
            current_abs_coordinate_mm[i] = code_value();
            sync_plan_position_delta();
        }
    }

    return ;
}


/*
 * ��ָ����gcode�ļ��ж���һ��ָ��
 * @fp gcode�ļ�������
 * @cmd_buff �����
 */
void gcode_read_one_cmd_from_file(FILE *fp, char *cmd_buff)
{
    int ch;
    int i;
    
    assert(NULL != fp);
    assert(NULL != cmd_buff);

    // ��ջ���
    cmd_buff[0] = '\0';

    for (i=0; i<ONE_GCODE_CMD_MAX_SIZE-1; )
    {
        // ȡ��һ���ַ�
        ch = fgetc(fp);

        if (EOF == ch)   // �ж��Ƿ������ļ���ȡ���
        {
            // �ǣ�������
            break;
        }
        else if (IS_END_OF_LINE(ch)) // �ж��Ƿ�Ϊ���з�
        {
            // �жϵ�ǰָ����Ƿ�Ϊ��
            if (0 == i)
            {
                // �ǣ���˵���ǿ��У����Ե���������ȡ��һ���ַ�
                continue;
            }
            else
            {
                // ���治Ϊ�գ�˵���Ѿ���ȡ��ָ���ˣ��������������ص�ǰָ��
                break;
            }
        }
        else if (IS_COMMENT_FLAG(ch))    // �ж��Ƿ�Ϊע��
        {
            // �ǣ���������������ע��
            while (1)
            {
                ch = fgetc(fp);
                if (IS_END_OF_LINE(ch) || (EOF==ch))
                {
                    // �������з����ļ�������
                    break;
                }
            }

            // �ж��Ƿ��Ѿ���ȡ������
            if (0 == i)
            {
                // ��û�������������ȡ��һ���ַ�
                continue;
            }
            else
            {
                // �Ѿ������ˣ�������
                break;
            }
        }
        else
        {
            cmd_buff[i] = (char)ch;
            i++;
        }
    }

    // ׷���ַ���������
    cmd_buff[i] = '\0';

    return ;
}


/**
 * M24: Start SD Print
 * ��ʼ��ӡgcode�ļ�
 */
void gcode_M24(void)
{
    FILE *fp_gcode = NULL;
    char *file_name = gcode_current_cmd_args;
    char cmd_buff[ONE_GCODE_CMD_MAX_SIZE];

    // ��gcode�ļ�
    printf("[%s] file_name=%s, start print...\n", __FUNCTION__, file_name);
    fp_gcode = fopen(file_name, "r");
    if (NULL == fp_gcode)
    {
        printf("[%s] fopen file %s fail.\n", __FUNCTION__, file_name);
        return ;
    }
    printf("[%s] file %s open success.\n", __FUNCTION__, file_name);

    while (1)
    {
        // ����һ��ָ��
        gcode_read_one_cmd_from_file(fp_gcode, cmd_buff);

        // �ж��Ƿ�Ϊ��ָ��
        if (TRUE == gcode_is_empty_cmd(cmd_buff))
        {
            // �ǣ���˵���Ѿ�ִ��������gcode�ļ���
            break;
        }

        // ��ӡ��ǰָ��
        printf("%s\n", cmd_buff);

        // ������ִ��
        gcode_parse_one_cmd(cmd_buff);
    }

    // �ر�gcode�ļ�
    fclose(fp_gcode);
    printf("[%s] print ok!\n", __FUNCTION__);

    return ;
}


/**
 * M104: Set hot end temperature
 * ���ü���ͷĿ���¶ȣ����ú���������
 */
void gcode_M104(void)
{
    // ���ü���ͷĿ���¶�
    if (code_seen('S')) temp_set_extruder_target_temp(code_value());

    // ��ӡ����ͷĿ���¶�
    temp_print_extruder_target_temp();

    return ;
}


// M105: ��ȡ�������¶�
void gcode_M105(void)
{
    printf("[%s] extruder current_temp=%f\n", __FUNCTION__, temp_get_extruder_current_temp());

    return ;
}


// M106: ������ȴ����
void gcode_M106(void)
{
    temp_cooling_fan_enable();
    return ;
}


// M107: �ر���ȴ����
void gcode_M107(void)
{
    temp_cooling_fan_disable();
    return ;
}



/**
 * M109: Sxxx Wait for extruder(s) to reach temperature. Waits only when heating.
 *       Rxxx Wait for extruder(s) to reach temperature. Waits when heating and cooling.
 * M109: ���ü������¶�
 * ���ú�ȵ��¶ȵ����趨ֵ��������������һ��ȷ��
 * ���裬��pid���¹����У���ǰ�¶���Ŀ���¶�+-1��Χ�ڱ�����10s������Ϊ�ȶ���
 */
void gcode_M109(void)
{
    float target_temp;                  // Ŀ���¶�
    float current_temp;                 // ��ǰ�¶�
    time_t residency_start_time_s = 0;  // ��ǰ�¶ȵ�һ�ν���Ŀ���¶�+-1�ķ�Χ��
    
    // ���ü���ͷĿ���¶�
    if (code_seen('S') || code_seen('R'))
    {
        target_temp = code_value();
        temp_set_extruder_target_temp(target_temp);
    }

    // ��ӡ����ͷĿ���¶�
    temp_print_extruder_target_temp();

    // �ȴ���ֱ������Ŀ���¶�
    while (1)
    {
        // �жϵ�ǰ�¶��Ƿ���Ŀ���¶ȸ�����
        current_temp = temp_get_extruder_current_temp();
        if (TEMP_WINDOW>labs(target_temp-current_temp))
        {
            // �ж��Ƿ��ǵ�һ�ε���Ŀ���¶�+-1��Χ��
            if (0==residency_start_time_s)
            {
                // �ǵ�һ�Σ����¼��ǰʱ��
                residency_start_time_s=time(NULL);
            }
            else
            {
                // ���ǵ�һ�Σ����ж��Ƿ��Ѿ��ȶ���
                if (TEMP_RESIDENCY_TIME<time(NULL)-residency_start_time_s)
                {
                    // �Ѿ��ȶ���
                    printf("[%s] current temp is target temp.\n", __FUNCTION__);
                    break;
                }
            }
        }

        sleep(1);
    }
}


/**
 * M114: Output current position to serial port
 */
void gcode_M114(void)
{
    printf("[%s]\n", __FUNCTION__);
    printf("X: %f\n", current_abs_coordinate_mm[X_AXIS]);
    printf("Y: %f\n", current_abs_coordinate_mm[Y_AXIS]);
    printf("Z: %f\n", current_abs_coordinate_mm[Z_AXIS]);
    printf("E: %f\n", current_abs_coordinate_mm[E_AXIS]);

    return ;
}


/**
 * M301: Set PID parameters P I D (and optionally C, L)
 * ����PIDϵ��
 *
 *   P[float] Kp term
 *   I[float] Ki term (unscaled)
 *   D[float] Kd term (unscaled)
 *
 * With PID_ADD_EXTRUSION_RATE:
 *
 *   C[float] Kc term
 *   L[float] LPQ length
 */
void gcode_M301(void)
{
    if (code_seen('P')) temp_set_Kp(code_value());
    if (code_seen('I')) temp_set_Ki(code_value());
    if (code_seen('D')) temp_set_Kd(code_value());
    if (code_seen('C')) temp_set_Kc(code_value());

    temp_print_Kp_Ki_Kd_Kc();

    return ;
}


/**
 * M665: Set delta configurations
 *
 *    L = diagonal rod
 *    R = delta radius
 *    S = segments per second
 *    A = Alpha (Tower 1) diagonal rod trim
 *    B = Beta (Tower 2) diagonal rod trim
 *    C = Gamma (Tower 3) diagonal rod trim
 *    Z = ����ӡ�߶�
 */
void gcode_M665(void)
{
    if (code_seen('L')) { delta_diagonal_rod = code_value(); }
    if (code_seen('R')) { delta_radius = code_value(); }
    if (code_seen('S')) { delta_segments_per_second = code_value(); }
    if (code_seen('A')) { delta_diagonal_rod_trim_tower_1 = code_value(); }
    if (code_seen('B')) { delta_diagonal_rod_trim_tower_2 = code_value(); }
    if (code_seen('C')) { delta_diagonal_rod_trim_tower_3 = code_value(); }
    if (code_seen('Z')) { z_max_height = code_value(); }

    printf("[%s]\
            \ndelta_diagonal_rod=%f, \
            \ndelta_radius=%f, \
            \ndelta_segments_per_second=%f, \
            \ndelta_diagonal_rod_trim_tower_1=%f, \
            \ndelta_diagonal_rod_trim_tower_2=%f, \
            \ndelta_diagonal_rod_trim_tower_3=%f, \
            \nz_max_height=%f\n",
            __FUNCTION__,
            delta_diagonal_rod,
            delta_radius,
            delta_segments_per_second,
            delta_diagonal_rod_trim_tower_1,
            delta_diagonal_rod_trim_tower_2,
            delta_diagonal_rod_trim_tower_3,
            z_max_height);

    // ���¼���
    recalc_delta_settings();

    return ;
}


/**
 * M666: Set delta endstop adjustment
 * ���治����������ֻ��ӡ��ǰendstop_adj�����鿴��ǰendstop_adj
 */
void gcode_M666(void)
{
    int i = 0;

    for (i=X_AXIS; i<=Z_AXIS; i++)
    {
        if (code_seen(axis_codes[i]))
        {
            endstop_adj[i] = code_value();
        }
    }
    printf("[%s] X=%f, Y=%f, Z=%f\n", 
            __FUNCTION__, 
            endstop_adj[X_AXIS],
            endstop_adj[Y_AXIS],
            endstop_adj[Z_AXIS]);

    return ;
}



// ������ִ��һ��ָ��
void gcode_parse_one_cmd(char *cmd_ptr)
{
    char cmd_code = '\0';
    UINT32 code_num = 0;

    // ������
    if (NULL == cmd_ptr)
    {
        return ;
    }

    // Get the command code, which must be G, M, or T
    cmd_code = *cmd_ptr++;

    // Bail early if there's no code
    if (!IS_NUMERIC(*cmd_ptr))
    {
        printf("[%s] unknown cmd err. current cmd=%s\r\n", __FUNCTION__, cmd_ptr);
        return ;
    }

    // Get and skip the code number
    do 
    {
        code_num = (code_num*10) + (*cmd_ptr-'0');
        cmd_ptr++;
    }
    while (IS_NUMERIC(*cmd_ptr));

    // Skip all spaces to get to the first argument, or nul
    while (IS_BACKSPACE(*cmd_ptr)) cmd_ptr++;

    // The command's arguments (if any) start here, for sure!
    gcode_current_cmd_args = cmd_ptr;

    // Handle a known G, M, or T
    switch (cmd_code)
    {
        case 'G':
            switch (code_num)
            {
                // G0: �����ƶ�
                // G1: �����ƶ�
                case 0:
                case 1:
                    gcode_G0_G1();
                    break;

                // Set Units to Millimeters ʹ�ú�����Ϊ��λ
                case 21:
                    gcode_G21();
                    break;

                // G28: �ƶ���ԭ��
                case 28:
                    gcode_G28();
                    break;

                // G90: ���õ�ǰ����ģʽΪ��������ģʽ
                case 90:
                    gcode_G90();
                    break;

                // G91: ���õ�ǰ����ģʽΪ�������ģʽ
                case 91:
                    gcode_G91();
                    break;

                // G92: ���嵱ǰλ��
                case 92:
                    gcode_G92();
                    break;

                default:
                    printf("[%s] unknown cmd=G%u\r\n", __FUNCTION__, code_num);
                    break;
            }
            break;

        case 'M':
            switch (code_num)
            {
                // M24: ��ʼ��ӡgcode�ļ�
                case 24:
                    gcode_M24();
                    break;
                
                // M84: ֹͣ�����Ŀ�ת
                // disable���е������ʱ����ʵ�֣�����治��3d��ӡ���ˣ����ԶϿ���Դ���������Ǹ���
                case 84:
                    break;
                    
                // M104: ���ü������¶�(����)
                // ���ú���������
                case 104:
                    gcode_M104();
                    break;

                // M105: ��ȡ�������¶�
                case 105:
                    gcode_M105();
                    break;

                // // M106: ������ȴ����
                // case 106:
                //     gcode_M106();
                //     break;

                // // M107: �ر���ȴ����
                // case 107:
                //     gcode_M107();
                //     break;

                // M109: ���ü������¶�
                // ���ú�ȵ��¶ȵ����趨ֵ��������������һ��ȷ��
                case 109:
                    gcode_M109();
                    break;

                // M114: ��ȡ��ǰλ��
                case 114:
                    gcode_M114();
                    break;

                // M140: �����ȴ��¶�
                // ���ú��������أ�Ŀǰ�ݲ�ʹ���ȴ�
                case 140:
                    break;

                // M301: ����PIDϵ��
                case 301:
                    gcode_M301();
                    break;

                // M665: Set delta configurations
                case 665:
                    gcode_M665();
                    break;

                // M666: Set delta endstop adjustment
                case 666:
                    gcode_M666();
                    break;

                default:
                    printf("[%s] unknown cmd=M%u\r\n", __FUNCTION__, code_num);
                    break;
            }
            break;

        case 'T':
            break;

        default:
            printf("[%s] unknown cmd code=%c\r\n", __FUNCTION__, cmd_code);
            break;
    }
    return ;
}
