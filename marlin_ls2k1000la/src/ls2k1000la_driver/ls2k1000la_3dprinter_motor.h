#ifndef MY_ls2k1000la_3dprinter_motor_H
#define MY_ls2k1000la_3dprinter_motor_H

#include <irq.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/serial_8250.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/spi/mmc_spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/mmc/host.h>
#include <linux/phy.h>
#include <linux/stmmac.h>
#include <linux/i2c.h>
#include <linux/videodev2.h>
#include <linux/input.h>
#include <linux/clk.h>

#include <asm-generic/sizes.h>

#include <linux/smp.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/acpi.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <asm/bootinfo.h>
#include <boot_param.h>
#include <loongson.h>
#include <loongson-pch.h>

// xyz�����step������ls1c��Ӳ��pwm0,pwm1��pwm2��������������
// e�����step���Ż��������ģ�����������
// X
#define PRINTER_MOTOR_X_ENABLE_PIN              (8)
#define PRINTER_MOTOR_X_DIRECTION_PIN           (9)
#define PRINTER_MOTOR_X_STEP_PIN                (20)    // PWM0/GPIO20
#define PRINTER_MOTOR_X_ENDSTOP_PIN             (10)
// Y
#define PRINTER_MOTOR_Y_ENABLE_PIN              (11)
#define PRINTER_MOTOR_Y_DIRECTOR_PIN            (12)
#define PRINTER_MOTOR_Y_STEP_PIN                (21)    // PWM1/GPIO21
#define PRINTER_MOTOR_Y_ENDSTOP_PIN             (13)
// Z
#define PRINTER_MOTOR_Z_ENABLE_PIN              (14)
#define PRINTER_MOTOR_Z_DIRECTOR_PIN            (15)
#define PRINTER_MOTOR_Z_STEP_PIN                (22)    // PWM2/GPIO22
#define PRINTER_MOTOR_Z_ENDSTOP_PIN             (16)
// E
#define PRINTER_MOTOR_E_ENABLE_PIN              (17)
#define PRINTER_MOTOR_E_DIRECTOR_PIN            (18)
#define PRINTER_MOTOR_E_STEP_PIN                (23)    // GPIO23����ʹ��PWM3��
#define PRINTER_MOTOR_E_ENDSTOP_PIN             (19)


enum {
    LS2K1000LA_PWM_0 = 0,
    LS2K1000LA_PWM_1 = 1,
    LS2K1000LA_PWM_2 = 2,
    LS2K1000LA_PWM_3 = 3,
};


// pwm0��pwm1��pmw2���ڲ�������ֱ�����xyz���
#define MOTOR_X_PULSE_PWM                       (LS2K1000LA_PWM_0)
#define MOTOR_Y_PULSE_PWM                       (LS2K1000LA_PWM_1)
#define MOTOR_Z_PULSE_PWM                       (LS2K1000LA_PWM_2)

// pwm3������ʱ�������ڿ��Ʋ�������ٶ�
#define MOTOR_TIMER_PWM                         (LS2K1000LA_PWM_3)
#define MOTOR_TIMER_IRQ                         (LS2K1000LA_PWM3_IRQ)


/**
 * The axis order in all axis related arrays is X, Y, Z, E
 */
#define NUM_AXIS 4


enum AxisEnum {X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, E_AXIS = 3};


struct platform_3dprinter_motor_data {
    unsigned char enable_gpio;              // ʹ������
    unsigned char direction_gpio;           // ��������
    unsigned char step_gpio;                // ������������
    unsigned char endstop_gpio;             // ��λ��������
};




// �������оƬ��ʹ��
enum
{
    GPIO_MOTOR_ENABLE = 0,                  // �͵�ƽʹ��
    GPIO_MOTOR_DISABLE = 1,
};


// io�ڵ�ƽֵ
enum
{
    GPIO_LEVEL_LOW = 0,                     // �͵�ƽ
    GPIO_LEVEL_HIGH = 1,                    // �ߵ�ƽ
};


// ��λ����״̬
enum
{
    GPIO_ENDSTOP_STATUS_CLOSE = 0,          // �պ�ʱ��·�����0
    GPIO_ENDSTOP_STATUS_OPEN = 1,           // ��ʱ��·�����1
};

#endif











