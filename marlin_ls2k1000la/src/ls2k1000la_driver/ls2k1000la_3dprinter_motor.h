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

// xyz电机的step引脚由ls1c的硬件pwm0,pwm1和pwm2产生单脉冲驱动
// e电机的step引脚还是由软件模拟产生单脉冲
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
#define PRINTER_MOTOR_E_STEP_PIN                (23)    // GPIO23（不使用PWM3）
#define PRINTER_MOTOR_E_ENDSTOP_PIN             (19)


enum {
    LS2K1000LA_PWM_0 = 0,
    LS2K1000LA_PWM_1 = 1,
    LS2K1000LA_PWM_2 = 2,
    LS2K1000LA_PWM_3 = 3,
};


// pwm0、pwm1和pmw2用于产单脉冲分别驱动xyz电机
#define MOTOR_X_PULSE_PWM                       (LS2K1000LA_PWM_0)
#define MOTOR_Y_PULSE_PWM                       (LS2K1000LA_PWM_1)
#define MOTOR_Z_PULSE_PWM                       (LS2K1000LA_PWM_2)

// pwm3用作定时器，用于控制步进电机速度
#define MOTOR_TIMER_PWM                         (LS2K1000LA_PWM_3)
#define MOTOR_TIMER_IRQ                         (LS2K1000LA_PWM3_IRQ)


/**
 * The axis order in all axis related arrays is X, Y, Z, E
 */
#define NUM_AXIS 4


enum AxisEnum {X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, E_AXIS = 3};


struct platform_3dprinter_motor_data {
    unsigned char enable_gpio;              // 使能引脚
    unsigned char direction_gpio;           // 方向引脚
    unsigned char step_gpio;                // 步进脉冲引脚
    unsigned char endstop_gpio;             // 限位开关引脚
};




// 电机驱动芯片的使能
enum
{
    GPIO_MOTOR_ENABLE = 0,                  // 低电平使能
    GPIO_MOTOR_DISABLE = 1,
};


// io口电平值
enum
{
    GPIO_LEVEL_LOW = 0,                     // 低电平
    GPIO_LEVEL_HIGH = 1,                    // 高电平
};


// 限位开关状态
enum
{
    GPIO_ENDSTOP_STATUS_CLOSE = 0,          // 闭合时短路，输出0
    GPIO_ENDSTOP_STATUS_OPEN = 1,           // 打开时断路，输出1
};

#endif











