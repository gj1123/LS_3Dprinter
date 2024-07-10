

#ifndef __PRINTER_H
#define __PRINTER_H


#include "public.h"


/**
 * The axis order in all axis related arrays is X, Y, Z, E
 */
#define NUM_AXIS 4


/**
 * Axis indices as enumerated constants
 *
 * A_AXIS and B_AXIS are used by COREXY printers
 * X_HEAD and Y_HEAD is used for systems that don't have a 1:1 relationship between X_AXIS and X Head movement, like CoreXY bots.
 */
enum AxisEnum {X_AXIS = 0, A_AXIS = 0, Y_AXIS = 1, B_AXIS = 1, Z_AXIS = 2, C_AXIS = 2, E_AXIS = 3, X_HEAD = 4, Y_HEAD = 5, Z_HEAD = 5};

enum EndstopEnum {X_MIN = 0, Y_MIN = 1, Z_MIN = 2, Z_MIN_PROBE = 3, X_MAX = 4, Y_MAX = 5, Z_MAX = 6, Z2_MIN = 7, Z2_MAX = 8};



// 归零时的feedrate，单位毫米每分
#define HOME_FEEDRATE_MM_MIN            (9000.0)    // 默认采用最大速度150mm/s=9000mm/min


// 在驱动执行block的过程中，是否检查限位器
#define NOT_CHECK_ENDSTOP               (FALSE)
#define NEED_CHECK_ENDSTOP              (TRUE)

//homing hits the endstop, then retracts by this distance, before it tries to slowly bump again:
#define HOME_BUMP_MM                    (5)
#define HOMING_BUMP_DIVISOR {10, 10, 10}  // Re-Bump Speed Divisor (Divides the Homing Feedrate)

// 一条gcode命令的最大大小
#define ONE_GCODE_CMD_MAX_SIZE              (96)


#endif

