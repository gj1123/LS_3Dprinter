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


/* xyz三个电机轴按逆时针方向排列，如下
 *    z
 *  x   y
 * 因为xyz电机轴构成一个等边三角形，把整个三角洲打印机旋转后可以得到如下，这也是可以的
 *    x
 *  y   z
 * 不论那个是xyz，逆时针分布是原则
 * 笛卡尔空间的xyz坐标中的x轴与xy电机轴平行，并且经过等边三角形圆心
 */
 
// 左前方的电机轴在笛卡尔坐标系的x值
float delta_tower1_x;       // front left tower
// 左前方的电机轴在笛卡尔坐标系的y值
float delta_tower1_y;
// 右前方的电机轴在笛卡尔坐标系的x值
float delta_tower2_x;       // front right tower
// 右前方的电机轴在笛卡尔坐标系的y值
float delta_tower2_y;
// 后面中间的那个电机轴在笛卡尔坐标系的x值
float delta_tower3_x;       // back middle tower
// 后面中间的那个电机轴在笛卡尔坐标系的y值
float delta_tower3_y;

// these are the default values, can be overriden with M665
float delta_radius = DELTA_RADIUS;

// 推杆长
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
 * FALSE:绝对坐标模式, TRUE:相对坐标模式
 */
BOOL relative_mode = FALSE;  

float min_pos[3] = { X_MIN_POS, Y_MIN_POS, Z_MIN_POS };
float max_pos[3] = { X_MAX_POS, Y_MAX_POS, Z_MAX_POS };
float z_max_height = Z_MAX_POS-Z_MIN_POS;               // 最大的打印高度
float endstop_adj[3] = {0.0};

float delta_segments_per_second = DELTA_SEGMENTS_PER_SECOND;

char* seen_pointer; ///< A pointer to find chars in the command string (X, Y, Z, E, etc.)


// 当前坐标
// 笛卡尔空间绝对坐标，单位mm
// 如果开始运动了，current_abs_coordinate_mm就是
// 上一个prepare_move()循环执行后上一次的destination_abs_coordinate_mm的值
float current_abs_coordinate_mm[NUM_AXIS] = { 0.0};
// 从指令G1中解析出来的目标坐标
// 笛卡尔空间绝对坐标，单位mm
float destination_abs_coordinate_mm[NUM_AXIS] = { 0.0 };
// 三个电机对应滑块在笛卡尔坐标系z轴值
// 绝对坐标，单位mm
float carriage_abs_z_coordinate_mm[3] = { 0 };


// 从指令G1中解析出来的打印速度，单位mm/min
float feedrate_mm_min = HOME_FEEDRATE_MM_MIN;

// 当前命令的参数
char *gcode_current_cmd_args = NULL;


void gcode_M114(void);
void gcode_parse_one_cmd(char *cmd_ptr);


// 重新计算
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


// 从配置文件中加载配置信息
void gcode_load_setting(void)
{
    // 手动调平得到的值
    delta_radius = 97;

    endstop_adj[X_AXIS] = -4.7;
    endstop_adj[Y_AXIS] = -12.8;
    endstop_adj[Z_AXIS] = -1.7;
}


// 初始化
void gcode_init(void)
{
    // 加载配置信息
    gcode_load_setting();

    // 重新计算各个参数
    recalc_delta_settings();

    return ;
}


// 判断当前命令参数中是否有关键字X,Y,Z,E,F......
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


// 获取当前命令参数中关键字X,Y,Z,E,F...对应的数字
float code_value(void)
{
    float ret = 0.0;

    ret = strtod(seen_pointer+1, NULL);

    return ret;
}


// 更新当前坐标
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


// 把xyz目标坐标限制在打印允许的范围内
void clamp_to_software_endstops(float target[3])
{
    NOLESS(target[X_AXIS], min_pos[X_AXIS]);
    NOLESS(target[Y_AXIS], min_pos[Y_AXIS]);
//    NOLESS(target[Z_AXIS], min_pos[Z_AXIS]);      // 手动调平时可能遇到z<0

    NOMORE(target[X_AXIS], max_pos[X_AXIS]);
    NOMORE(target[Y_AXIS], max_pos[Y_AXIS]);
    NOMORE(target[Z_AXIS], max_pos[Z_AXIS]);

    return ;
}


// 将打印机的笛卡尔世界坐标转换为xyz电机轴的运动量
// @cartesian 笛卡尔世界坐标xyz的值
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


// delta结构的prepare_move
// @target_abs 笛卡尔空间目标坐标(绝对坐标)
// 本函数的详细讲解请百度"Delta3D打印机代码解读及调机心得"
BOOL prepare_move_delta(float target_abs[NUM_AXIS])
{
    float difference[NUM_AXIS];     // 目标坐标与当前坐标的差值，单位mm
    float cartesian_mm = 0;         // 目标坐标与当前坐标的笛卡尔距离
    float seconds = 0.0;            // 从当前坐标运动到目标坐标需要的时间
    INT32 segments = 0;             // 从当前坐标到目标坐标整个路径被分割成segments段
    UINT8 i = 0;
    UINT32 s = 0;
    float fraction = 0.0;

    // 计算目标坐标和当期坐标的笛卡尔距离
    for (i=0; i<NUM_AXIS; i++)
    {
        difference[i] = target_abs[i] - current_abs_coordinate_mm[i];            // 计算相对位置值(坐标差)
    }
    cartesian_mm = sqrt(SQUARE(difference[X_AXIS])                  // 计算笛卡尔空间距离
                        + SQUARE(difference[Y_AXIS]) 
                        + SQUARE(difference[Z_AXIS]));
    if (0.000001 > cartesian_mm) 
        cartesian_mm = abs(difference[E_AXIS]);    // 如果XYZ轴的距离很小，则用E轴
    if (0.000001 > cartesian_mm)
        return FALSE;

    // 根据距离和速度计算执行时间
    // cartesian_mm 单位mm
    // feedrate_mm_min 单位mm/min
    // feedrate_multiplier_percent 单位百分比
    seconds = cartesian_mm / (feedrate_mm_min/60);
    // 计算需要的步数
    // 1秒的step分为多段细分执行
    segments = MAX(1, (UINT32)(delta_segments_per_second*seconds));

    // 依次处理每一步
    for (s=1; s<=segments; s++)
    {
        // 直线插值
        fraction = (float)s / (float)segments;
        for (i=0; i<NUM_AXIS; i++)
        {
            target_abs[i] = current_abs_coordinate_mm[i] + difference[i] * fraction;
        }

        // 将打印机的笛卡尔世界坐标转换三个垂直的电机轴的运动坐标
        calculate_delta(target_abs);

        // 规划路径
        // x,y轴的运动往往速度变化非常频繁，如果采用匀速运动，
        // 速度的突然变化会给电路带来很大的冲击，影响机器的稳定性。
        // 这个物理层的原理，就决定了算法层应该采用加速减速的运动算法。
        // 路径规划器意味着，程序在执行步进电机的动作之前，
        // 就已经计算好了整个过程的速度曲线。后面就只是Stepper模块忠实地执行。
        plan_buffer_line(carriage_abs_z_coordinate_mm[X_AXIS], 
                         carriage_abs_z_coordinate_mm[Y_AXIS], 
                         carriage_abs_z_coordinate_mm[Z_AXIS], 
                         target_abs[E_AXIS], 
                         feedrate_mm_min/60,    // 进给率单位mm/sec
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
    // 把xyz目标坐标限制在打印允许的范围内
    // destination是绝对值，单位mm
    clamp_to_software_endstops(destination_abs_coordinate_mm);

    // delta结构的prepare_move
    if (TRUE != prepare_move_delta(destination_abs_coordinate_mm))
    {
        return ;
    }

    // 更新当前坐标
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


// 同步修改plan相关位置
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
                     feedrate_mm_min/60,    // 进给率单位mm/sec
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
    // 三角洲3D打印机归零后的坐标为(0,0,z_max_height)
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
        // 不会出现这种情况
    }

    return ;
}


/**
 * Home an individual axis
 * 归零某一个电机轴
 */
void homeaxis(enum AxisEnum axis)
{
    // Set the axis position as setup for the move
    current_abs_coordinate_mm[axis] = 0;
    sync_plan_position();

    // Move towards the endstop until an endstop is triggered
    // 朝限位器运行直到碰到限位器为止
    destination_abs_coordinate_mm[axis] = 1.5*z_max_height;
    feedrate_mm_min = HOME_FEEDRATE_MM_MIN;
    line_to_destination(NEED_CHECK_ENDSTOP);
    motor_synchronize();            // Wait for planner moves to finish!

    // Set the axis position as setup for the move
    current_abs_coordinate_mm[axis] = 0;
    sync_plan_position();

    // Move away from the endstop by the axis HOME_BUMP_MM
    // 离开限位器一小段距离
    destination_abs_coordinate_mm[axis] = -HOME_BUMP_MM;
    line_to_destination(NOT_CHECK_ENDSTOP);
    motor_synchronize();            // Wait for planner moves to finish!

    // Slow down the feedrate for the next move
    set_homing_bump_feedrate(axis);

    // Move slowly towards the endstop until triggered
    // 再次缓慢朝限位器移动直到碰到限位器为止
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
 * 判断是否为空指令
 * 即命令缓存是否为空字符串
 * @cmd_buff 命令缓存
 */
BOOL gcode_is_empty_cmd(char *cmd_buff)
{
    if (0 == strlen(cmd_buff))
        return TRUE;
    else
        return FALSE;
}


// G21: Set Units to Millimeters 使用毫米作为单位
void gcode_G21(void)
{
    // 默认使用毫米作为单位，所以默认为空函数，不处理
    return ;
}


// G28: 移动到原点
void gcode_G28(void)
{
    int i = 0;
    float saved_feedrate_mm_min = 0;
    
    // Wait for planner moves to finish!
    motor_synchronize();

    // 保存环境，待后续恢复
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

    // 恢复环境
    feedrate_mm_min = saved_feedrate_mm_min;

    // Send end position to RepetierHost
    gcode_M114();

    return ;
}


/*
 * 设置当前坐标模式为绝对坐标模式
 * 系统默认为绝对坐标模式
 */
void gcode_G90(void)
{
    relative_mode = FALSE;
    printf("[%s] set abs coordinate mode.\n", __FUNCTION__);
}


// 设置当前坐标模式为相对坐标模式
void gcode_G91(void)
{
    relative_mode = TRUE;
    printf("[%s] set relative coordinate mode.\n", __FUNCTION__);
}


// G92: 定义当前位置
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
 * 从指定的gcode文件中读出一条指令
 * @fp gcode文件描述符
 * @cmd_buff 命令缓存
 */
void gcode_read_one_cmd_from_file(FILE *fp, char *cmd_buff)
{
    int ch;
    int i;
    
    assert(NULL != fp);
    assert(NULL != cmd_buff);

    // 清空缓存
    cmd_buff[0] = '\0';

    for (i=0; i<ONE_GCODE_CMD_MAX_SIZE-1; )
    {
        // 取出一个字符
        ch = fgetc(fp);

        if (EOF == ch)   // 判断是否整个文件读取完成
        {
            // 是，则跳出
            break;
        }
        else if (IS_END_OF_LINE(ch)) // 判断是否为换行符
        {
            // 判断当前指令缓存是否为空
            if (0 == i)
            {
                // 是，则说明是空行，忽略掉，继续读取下一个字符
                continue;
            }
            else
            {
                // 缓存不为空，说明已经读取到指令了，则跳出，并返回当前指令
                break;
            }
        }
        else if (IS_COMMENT_FLAG(ch))    // 判断是否为注释
        {
            // 是，则跳过本行所有注释
            while (1)
            {
                ch = fgetc(fp);
                if (IS_END_OF_LINE(ch) || (EOF==ch))
                {
                    // 遇到换行符或文件结束符
                    break;
                }
            }

            // 判断是否已经读取了命令
            if (0 == i)
            {
                // 还没读到命令，继续读取下一个字符
                continue;
            }
            else
            {
                // 已经读到了，则跳出
                break;
            }
        }
        else
        {
            cmd_buff[i] = (char)ch;
            i++;
        }
    }

    // 追加字符串结束符
    cmd_buff[i] = '\0';

    return ;
}


/**
 * M24: Start SD Print
 * 开始打印gcode文件
 */
void gcode_M24(void)
{
    FILE *fp_gcode = NULL;
    char *file_name = gcode_current_cmd_args;
    char cmd_buff[ONE_GCODE_CMD_MAX_SIZE];

    // 打开gcode文件
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
        // 读出一条指令
        gcode_read_one_cmd_from_file(fp_gcode, cmd_buff);

        // 判断是否为空指令
        if (TRUE == gcode_is_empty_cmd(cmd_buff))
        {
            // 是，则说明已经执行完整个gcode文件了
            break;
        }

        // 打印当前指令
        printf("%s\n", cmd_buff);

        // 解析并执行
        gcode_parse_one_cmd(cmd_buff);
    }

    // 关闭gcode文件
    fclose(fp_gcode);
    printf("[%s] print ok!\n", __FUNCTION__);

    return ;
}


/**
 * M104: Set hot end temperature
 * 设置挤出头目标温度，设置后立即返回
 */
void gcode_M104(void)
{
    // 设置挤出头目标温度
    if (code_seen('S')) temp_set_extruder_target_temp(code_value());

    // 打印挤出头目标温度
    temp_print_extruder_target_temp();

    return ;
}


// M105: 获取挤出机温度
void gcode_M105(void)
{
    printf("[%s] extruder current_temp=%f\n", __FUNCTION__, temp_get_extruder_current_temp());

    return ;
}


// M106: 开启冷却风扇
void gcode_M106(void)
{
    temp_cooling_fan_enable();
    return ;
}


// M107: 关闭冷却风扇
void gcode_M107(void)
{
    temp_cooling_fan_disable();
    return ;
}



/**
 * M109: Sxxx Wait for extruder(s) to reach temperature. Waits only when heating.
 *       Rxxx Wait for extruder(s) to reach temperature. Waits when heating and cooling.
 * M109: 设置挤出机温度
 * 设置后等到温度到达设定值后再向主机发送一个确认
 * 假设，在pid控温过程中，当前温度在目标温度+-1范围内保持了10s，则认为稳定了
 */
void gcode_M109(void)
{
    float target_temp;                  // 目标温度
    float current_temp;                 // 当前温度
    time_t residency_start_time_s = 0;  // 当前温度第一次进入目标温度+-1的范围内
    
    // 设置挤出头目标温度
    if (code_seen('S') || code_seen('R'))
    {
        target_temp = code_value();
        temp_set_extruder_target_temp(target_temp);
    }

    // 打印挤出头目标温度
    temp_print_extruder_target_temp();

    // 等待，直到到达目标温度
    while (1)
    {
        // 判断当前温度是否在目标温度附近了
        current_temp = temp_get_extruder_current_temp();
        if (TEMP_WINDOW>labs(target_temp-current_temp))
        {
            // 判断是否是第一次到达目标温度+-1范围内
            if (0==residency_start_time_s)
            {
                // 是第一次，则记录当前时间
                residency_start_time_s=time(NULL);
            }
            else
            {
                // 不是第一次，则判断是否已经稳定了
                if (TEMP_RESIDENCY_TIME<time(NULL)-residency_start_time_s)
                {
                    // 已经稳定了
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
 * 设置PID系数
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
 *    Z = 最大打印高度
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

    // 重新计算
    recalc_delta_settings();

    return ;
}


/**
 * M666: Set delta endstop adjustment
 * 后面不跟参数，则只打印当前endstop_adj，即查看当前endstop_adj
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



// 解析并执行一条指令
void gcode_parse_one_cmd(char *cmd_ptr)
{
    char cmd_code = '\0';
    UINT32 code_num = 0;

    // 检查入参
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
                // G0: 快速移动
                // G1: 控制移动
                case 0:
                case 1:
                    gcode_G0_G1();
                    break;

                // Set Units to Millimeters 使用毫米作为单位
                case 21:
                    gcode_G21();
                    break;

                // G28: 移动到原点
                case 28:
                    gcode_G28();
                    break;

                // G90: 设置当前坐标模式为绝对坐标模式
                case 90:
                    gcode_G90();
                    break;

                // G91: 设置当前坐标模式为相对坐标模式
                case 91:
                    gcode_G91();
                    break;

                // G92: 定义当前位置
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
                // M24: 开始打印gcode文件
                case 24:
                    gcode_M24();
                    break;
                
                // M84: 停止机器的空转
                // disable所有电机，暂时不用实现，如果真不用3d打印机了，可以断开电源，这样岂不是更好
                case 84:
                    break;
                    
                // M104: 设置挤出机温度(快速)
                // 设置后立即返回
                case 104:
                    gcode_M104();
                    break;

                // M105: 获取挤出机温度
                case 105:
                    gcode_M105();
                    break;

                // // M106: 开启冷却风扇
                // case 106:
                //     gcode_M106();
                //     break;

                // // M107: 关闭冷却风扇
                // case 107:
                //     gcode_M107();
                //     break;

                // M109: 设置挤出机温度
                // 设置后等到温度到达设定值后再向主机发送一个确认
                case 109:
                    gcode_M109();
                    break;

                // M114: 获取当前位置
                case 114:
                    gcode_M114();
                    break;

                // M140: 设置热床温度
                // 设置后立即返回，目前暂不使用热床
                case 140:
                    break;

                // M301: 设置PID系数
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
