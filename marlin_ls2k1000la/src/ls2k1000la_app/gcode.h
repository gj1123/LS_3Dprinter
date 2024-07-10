// gcodeָ��

#ifndef __GCODE_H
#define __GCODE_H


#include "public.h"


void gcode_init(void);

/*
 * �ж��Ƿ�Ϊ��ָ��
 * ��������Ƿ�Ϊ���ַ���
 * @cmd_buff �����
 */
BOOL gcode_is_empty_cmd(char *cmd_buff);


// G0,G1: Coordinated movement of X Y Z E axes
void gcode_G0_G1(void);

// G21: Set Units to Millimeters ʹ�ú�����Ϊ��λ
void gcode_G21(void);

// G28: �ƶ���ԭ��
void gcode_G28(void);


/*
 * ���õ�ǰ����ģʽΪ��������ģʽ
 * ϵͳĬ��Ϊ��������ģʽ
 */
void gcode_G90(void);

// ���õ�ǰ����ģʽΪ�������ģʽ
void gcode_G91(void);

// G92: ���嵱ǰλ��
void gcode_G92(void);



/**
 * M24: Start SD Print
 * ��ʼ��ӡgcode�ļ�
 */
void gcode_M24(void);

/**
 * M104: Set hot end temperature
 * ���ü���ͷĿ���¶ȣ����ú���������
 */
void gcode_M104(void);


// M105: ��ȡ�������¶�
void gcode_M105(void);


// M106: ������ȴ����
void gcode_M106(void);


// M107: �ر���ȴ����
void gcode_M107(void);


/**
 * M109: Sxxx Wait for extruder(s) to reach temperature. Waits only when heating.
 *       Rxxx Wait for extruder(s) to reach temperature. Waits when heating and cooling.
 * M109: ���ü������¶�
 * ���ú�ȵ��¶ȵ����趨ֵ��������������һ��ȷ��
 */
void gcode_M109(void);


/**
 * M114: Output current position to serial port
 */
void gcode_M114(void);



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
 *    Z = ����ӡ�߶�
 */
void gcode_M665(void);

/**
 * M666: Set delta endstop adjustment
 * ���治����������ֻ��ӡ��ǰendstop_adj�����鿴��ǰendstop_adj
 */
void gcode_M666(void);



// ������ִ��һ��ָ��
void gcode_parse_one_cmd(char *cmd_ptr);

    
#endif



