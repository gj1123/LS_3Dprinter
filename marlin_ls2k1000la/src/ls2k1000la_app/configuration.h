// ����ͷ�ļ�

#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H





// Make delta curves from many straight lines (linear interpolation).
// This is a trade-off between visible corners (not enough segments)
// and processor overload (too many expensive sqrt calls).
#define DELTA_SEGMENTS_PER_SECOND 200


// NOTE NB all values for DELTA_* values MUST be floating point, so always have a decimal point in them

// Center-to-center distance of the holes in the diagonal push rods.
// �˳�
#define DELTA_DIAGONAL_ROD 220.0 // mm

// Horizontal offset from middle of printer to smooth rod center.
// ������Բ�뾶(�������ɵĵȱ������ε���С���Բ�İ뾶)
#define DELTA_SMOOTH_ROD_OFFSET 152.0 // mm

// Horizontal offset of the universal joints on the end effector.
// װ�����ƽ̨�����ĵ������Ӵ��ľ���
#define DELTA_EFFECTOR_OFFSET 35.0 // mm

// Horizontal offset of the universal joints on the carriages.
//����Ử��ľ���
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


// �ڵ�������ƶ�1mm�Ĳ���
// ���������һȦ�Ĳ��� / һȦ���еľ���
// 42�������һȦ200����16ϸ�֣�����һȦ����=16*200=3200=100
// ���������һȦ���еľ������Ҫ����
// XYZ�����������һȦ��32.0mm������ÿ�ƶ�1mm��Ҫ�Ĳ���=(200*16)/32.0=100.0
// ������EһȦ��19.5mm������ÿ�ƶ�1mm��Ҫ�Ĳ���=(200*16)/19.5=164.10256
#define DEFAULT_AXIS_STEPS_PER_UNIT         {100.0, 100.0, 100.0, 164.10256}


// pid��ȡֵ��Χ[0,1]
#define PID_MAX                             (1)
#define PID_MIN                             (0)

// ��Ȼ�����ϣ�pidֵ������[PID_MIN, PID_MAX]֮�䶼����
// ��pidֵ̫С�������������ᶯ������ͬ��Ϊ0
// ���Ե�pidֵС�ڴ˷�ֵʱ��ֱ��ȡ0
#define PID_ACTION_MIN                      (0.02)
// ����װ�ù���̫���ڳ�ʱ�������ʹ�ý�С��pidֵ
#define PID_LONG_TIME_ACTION                (0.6)

// pid�ĵ��ڷ�Χ
// ��ǰ�¶���Ŀ���¶ȵ�ƫ����ڸ�ֵʱ������pidֵ��Ϊ������С(��ȫ�ټ��Ȼ��߲�����)
#define PID_FUNCTIONAL_RANGE                (5)

// ���¶�ƫ��������[-PID_FUNCTIONAL_RANGE, PID_FUNCTIONAL_RANGE]�� 
// ����ǰ�¶���Ŀ���¶ȸ�����ʹ��pid�㷨����
// pid��ϵ��
#define DEFUALT_Kp                          (0.15)
#define DEFAULT_Ki                          (0.0015)
#define DEFAULT_Kd                          (1)
#define DEFAULT_Kc                          (0.0008)


// ƽ������
#define PID_SMOOTHING_FACTOR                (0.5)



#endif

