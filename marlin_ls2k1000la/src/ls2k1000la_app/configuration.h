// 配置头文件

#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H





// Make delta curves from many straight lines (linear interpolation).
// This is a trade-off between visible corners (not enough segments)
// and processor overload (too many expensive sqrt calls).
#define DELTA_SEGMENTS_PER_SECOND 200


// NOTE NB all values for DELTA_* values MUST be floating point, so always have a decimal point in them

// Center-to-center distance of the holes in the diagonal push rods.
// 杆长
#define DELTA_DIAGONAL_ROD 220.0 // mm

// Horizontal offset from middle of printer to smooth rod center.
// 电机轴的圆半径(电机轴组成的等边三角形的最小外接圆的半径)
#define DELTA_SMOOTH_ROD_OFFSET 152.0 // mm

// Horizontal offset of the universal joints on the end effector.
// 装喷嘴的平台的中心到杆连接处的距离
#define DELTA_EFFECTOR_OFFSET 35.0 // mm

// Horizontal offset of the universal joints on the carriages.
//电机轴滑块的距离
#define DELTA_CARRIAGE_OFFSET 25.0 // mm

// Horizontal distance bridged by diagonal push rods when effector is centered.
#define DELTA_RADIUS (DELTA_SMOOTH_ROD_OFFSET-DELTA_EFFECTOR_OFFSET-DELTA_CARRIAGE_OFFSET)

// Print surface diameter/2 minus unreachable space (avoid collisions with vertical towers).
#define DELTA_PRINTABLE_RADIUS 70



// Travel limits after homing (units are in mm)
#define X_MIN_POS -DELTA_PRINTABLE_RADIUS
#define Y_MIN_POS -DELTA_PRINTABLE_RADIUS
#define Z_MIN_POS 0
#define X_MAX_POS DELTA_PRINTABLE_RADIUS
#define Y_MAX_POS DELTA_PRINTABLE_RADIUS
#define Z_MAX_POS 342.1


// 在电机轴上移动1mm的步数
// 步进电机跑一圈的步数 / 一圈运行的距离
// 42步进电机一圈200步，16细分，所以一圈步数=16*200=3200=100
// 步进电机跑一圈运行的距离就需要测了
// XYZ三个电机都是一圈跑32.0mm，所以每移动1mm需要的步数=(200*16)/32.0=100.0
// 挤出机E一圈跑19.5mm，所以每移动1mm需要的步数=(200*16)/19.5=164.10256
#define DEFAULT_AXIS_STEPS_PER_UNIT         {100.0, 100.0, 100.0, 164.10256}


// pid的取值范围[0,1]
#define PID_MAX                             (1)
#define PID_MIN                             (0)

// 虽然理论上，pid值在区间[PID_MIN, PID_MAX]之间都可以
// 但pid值太小，开关器件不会动作，等同于为0
// 所以当pid值小于此阀值时，直接取0
#define PID_ACTION_MIN                      (0.02)
// 加热装置惯性太大，在长时间加热是使用较小的pid值
#define PID_LONG_TIME_ACTION                (0.6)

// pid的调节范围
// 当前温度与目标温度的偏差大于该值时，将把pid值设为最大或最小(即全速加热或者不加热)
#define PID_FUNCTIONAL_RANGE                (5)

// 当温度偏差在区间[-PID_FUNCTIONAL_RANGE, PID_FUNCTIONAL_RANGE]内 
// 即当前温度在目标温度附近才使用pid算法控温
// pid的系数
#define DEFUALT_Kp                          (0.15)
#define DEFAULT_Ki                          (0.0015)
#define DEFAULT_Kd                          (1)
#define DEFAULT_Kc                          (0.0008)


// 平滑因子
#define PID_SMOOTHING_FACTOR                (0.5)



#endif

