/*
 * drivers\misc\ls2k1000la_3dprinter_heater_fan.c
 * 3d打印机加热头和风扇(散热)驱动
 * 用linux内核定时器模拟pwm
 */
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include </home/yxc/Documents/marlin_ls2k1000la-master/src/ls2k1000la_driver/ls2k1000la_3dprinter_heater_fan.h>


// pwm周期(ms)
#define HEATER_FAN_PWM_TIME_MS              (200)   // 200ms

enum
{
    HEATER_NOT_WORK = 0,    // 不加热(加热装置不工作)
    HEATER_WORK = 1,        // 加热(加热装置正常工作)
};

// 一个pwm周期内，工作的时长，单位ms
// 由于驱动中不能进行浮点计算
// 所以在应用程序中将pid的占空比直接换算为一个周期内加热头和散热的风扇工作的时长
typedef struct{
    unsigned int extruder_heater_ms;        // 挤出机加热的时长(ms)
    unsigned int extruder_fan_ms;           // 挤出机风扇工作的时长(ms)
}heater_fan_work_time_t;


static heater_fan_work_time_t heater_fan_work_time = {0};
static struct platform_3dprinter_heater_fan_data *heater_fan_data = NULL;

// 用于模拟pwm的定时器
static struct timer_list extruder_heater_timer;         // 挤出机加热的定时器
static struct timer_list extruder_fan_timer;            // 挤出机散热风扇的定时器
static DEFINE_MUTEX(heater_fan_lock);


// 给挤出机加热
static void extruder_heater_enable(void)
{
    gpio_direction_output(heater_fan_data->extruder_heater_gpio, HEATER_WORK);
}

// 挤出机不加热
static void extruder_heater_disable(void)
{
    gpio_direction_output(heater_fan_data->extruder_heater_gpio, HEATER_NOT_WORK);
}

// 挤出机加热定时器中断(模拟pwm)
void extruder_heater_timer_timeout_fn(unsigned long arg)
{
    static int level = 0;
    
    // 判断是否需要加热
    if (0 == heater_fan_work_time.extruder_heater_ms)
    {
        // 不需要加热
        extruder_heater_disable();
        mod_timer(&extruder_heater_timer, jiffies + HEATER_FAN_PWM_TIME_MS);
        level = 0;
        return ;
    }

    // 是否在整个pwm周期内一直加热
    if (HEATER_FAN_PWM_TIME_MS <= heater_fan_work_time.extruder_heater_ms)
    {
        // 一直加热
        extruder_heater_enable();
        mod_timer(&extruder_heater_timer, jiffies + HEATER_FAN_PWM_TIME_MS);
        level = 0;
        return ;
    }

    // 模拟pwm
    if (0 == level)
    {
        level = 1;
        extruder_heater_enable();
        mod_timer(&extruder_heater_timer, 
                  jiffies + heater_fan_work_time.extruder_heater_ms);
    }
    else
    {
        level = 0;
        extruder_heater_disable();
        mod_timer(&extruder_heater_timer, 
                  jiffies + (HEATER_FAN_PWM_TIME_MS - heater_fan_work_time.extruder_heater_ms));
    }
    
    return ;
}


static int heater_fan_close(struct inode *inode, struct file *filep)
{
    // 停止挤出机的加热和风扇
    extruder_heater_disable();
    // extruder_fan_disable();
    // cooling_fan_disable();
    heater_fan_work_time.extruder_heater_ms = 0;
    // heater_fan_work_time.extruder_fan_ms    = 0;
    
    return 0;
}


// 控制挤出机上的加热头和散热风扇
static ssize_t heater_fan_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
    heater_fan_work_time_t work_time = {0};
    
    if (sizeof(heater_fan_work_time_t) != count)
    {
        // 写入额数据不对
        return 0;
    }

    if (mutex_lock_interruptible(&heater_fan_lock))
    {
        return -ERESTARTSYS;
    }

    copy_from_user(&work_time, buf, sizeof(heater_fan_work_time_t));
    heater_fan_work_time.extruder_heater_ms = work_time.extruder_heater_ms;
    heater_fan_work_time.extruder_fan_ms    = work_time.extruder_fan_ms;

    mutex_unlock(&heater_fan_lock);
    

    return 0;
}



static struct file_operations ls2k1000la_heater_fan_ops = {
    .owner      = THIS_MODULE,
    // .open       = heater_fan_open,
    .release    = heater_fan_close,
    .write      = heater_fan_write,
    // .unlocked_ioctl      = cooling_fan_ioctl,
};


static struct miscdevice ls2k1000la_heater_fan_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "3dPrinter_heater_fan",
    .fops = &ls2k1000la_heater_fan_ops,
};


static int heater_fan_probe(struct platform_device *pdev)
{
    int ret = 0;
    
    printk(KERN_INFO "ls2k1000la 3dprinter heater fan driver's build start\n");

    heater_fan_data = pdev->dev.platform_data;
    if (!heater_fan_data)
    {
        dev_err(&pdev->dev, "failed to find platform data\n");
        return -EINVAL;
    }

    // 挤出机加热头
    ret = gpio_request(heater_fan_data->extruder_heater_gpio, "ls2k1000la_3dPrinter_heater_fan");
    if (0 > ret)
    {
        printk(KERN_ERR "[%s] request extruder heater gpio fail.", __FUNCTION__);
        return ret;
    }
    gpio_direction_output(heater_fan_data->extruder_heater_gpio, HEATER_NOT_WORK);

    // 初始化挤出机加热的定时器(用于模拟pwm)
    heater_fan_work_time.extruder_heater_ms = 0;        // 加热头不工作
    timer_setup(1, &extruder_heater_timer_timeout_fn, 1);
    extruder_heater_timer.function  = extruder_heater_timer_timeout_fn;
    extruder_heater_timer.expires   = jiffies + HEATER_FAN_PWM_TIME_MS;
    add_timer(&extruder_heater_timer);

    return 0;

    return ret;
}


static int heater_fan_remove(struct platform_device *pdev)
{
    gpio_free(heater_fan_data->extruder_heater_gpio);
    gpio_free(heater_fan_data->extruder_fan_gpio);
    gpio_free(heater_fan_data->cooling_fan_gpio);
    del_timer(&extruder_heater_timer);
    del_timer(&extruder_fan_timer);

    return 0;
}


static struct platform_driver ls2k1000la_heater_fan_driver = {
    .driver = {
        .name = "ls2k1000la_3dPrinter_heater_fan",
        .owner = THIS_MODULE,
    },
    .probe = heater_fan_probe,
    .remove = heater_fan_remove,
};


static int __init heater_fan_init(void)
{
    if (misc_register(&ls2k1000la_heater_fan_miscdev))
    {
        printk(KERN_ERR "could not register 3dPrinter heater fan driver!\n");
        return -EBUSY;
    }
    return platform_driver_register(&ls2k1000la_heater_fan_driver);
}


static void __exit heater_fan_exit(void)
{
    misc_deregister(&ls2k1000la_heater_fan_miscdev);
    platform_driver_unregister(&ls2k1000la_heater_fan_driver);
}


module_init(heater_fan_init);
module_exit(heater_fan_exit);

MODULE_AUTHOR("龙芯创印");
MODULE_DESCRIPTION("ls2k1000la 3dprinter heater and fan driver");
MODULE_LICENSE("GPL");
