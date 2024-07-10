// gcode指令

#ifndef __GCODE_H
#define __GCODE_H


#include "public.h"


void gcode_init(void);

/*
 * 判断是否为空指令
 * 即命令缓存是否为空字符串
 * @cmd_buff 命令缓存
 */
BOOL gcode_is_empty_cmd(char *cmd_buff);


// G0,G1: Coordinated movement of X Y Z E axes
void gcode_G0_G1(void);

// G21: Set Units to Millimeters 使用毫米作为单位
void gcode_G21(void);

// G28: 移动到原点
void gcode_G28(void);


/*
 * 设置当前坐标模式为绝对坐标模式
 * 系统默认为绝对坐标模式
 */
void gcode_G90(void);

// 设置当前坐标模式为相对坐标模式
void gcode_G91(void);

// G92: 定义当前位置
void gcode_G92(void);



/**
 * M24: Start SD Print
 * 开始打印gcode文件
 */
void gcode_M24(void);

/**
 * M104: Set hot end temperature
 * 设置挤出头目标温度，设置后立即返回
 */
void gcode_M104(void);


// M105: 获取挤出机温度
void gcode_M105(void);


// M106: 开启冷却风扇
void gcode_M106(void);


// M107: 关闭冷却风扇
void gcode_M107(void);


/**
 * M109: Sxxx Wait for extruder(s) to reach temperature. Waits only when heating.
 *       Rxxx Wait for extruder(s) to reach temperature. Waits when heating and cooling.
 * M109: 设置挤出机温度
 * 设置后等到温度到达设定值后再向主机发送一个确认
 */
void gcode_M109(void);


/**
 * M114: Output current position to serial port
 */
void gcode_M114(void);



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
void gcode_M301(void);


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
void gcode_M665(void);

/**
 * M666: Set delta endstop adjustment
 * 后面不跟参数，则只打印当前endstop_adj，即查看当前endstop_adj
 */
void gcode_M666(void);



// 解析并执行一条指令
void gcode_parse_one_cmd(char *cmd_ptr);

    
#endif



