/*
 * include\linux\ls1c_3dprinter_heater_fan.h
 * 3d��ӡ������ͷ�ͷ���(ɢ��)������ͷ�ļ�
 */
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
#ifndef MY_LS1C_3DPRINTER_HEATER_FAN_H
#define MY_LS1C_3DPRINTER_HEATER_FAN_H


// ����������ͷ��gpio
#define PRINTER_EXTRUDER_HEATER_PIN     (90)    // I2S_BCLK/GPIO90
// ������ɢ�ȷ��ȵ�gpio
#define PRINTER_EXTRUDER_FAN_PIN        (91)    // I2S_MCLK/GPIO91
// ��ӡ��Ʒ����ȴ���ȵ�gpio
#define PRINTER_COOLING_FAN_PIN         (88)    // I2S_DO/GPIO88


struct platform_3dprinter_heater_fan_data {
    unsigned char extruder_heater_gpio;         // ����������ͷ��gpio
    unsigned char extruder_fan_gpio;            // ������ɢ�ȷ��ȵ�gpio
    unsigned char cooling_fan_gpio;             // ��ӡ��Ʒ����ȴ���ȵ�gpio
};


#endif

