/*
 * drivers\misc\ls2k1000la_3dprinter_heater_fan.c
 * 3d��ӡ������ͷ�ͷ���(ɢ��)����
 * ��linux�ں˶�ʱ��ģ��pwm
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


// pwm����(ms)
#define HEATER_FAN_PWM_TIME_MS              (200)   // 200ms

enum
{
    HEATER_NOT_WORK = 0,    // ������(����װ�ò�����)
    HEATER_WORK = 1,        // ����(����װ����������)
};

// һ��pwm�����ڣ�������ʱ������λms
// ���������в��ܽ��и������
// ������Ӧ�ó����н�pid��ռ�ձ�ֱ�ӻ���Ϊһ�������ڼ���ͷ��ɢ�ȵķ��ȹ�����ʱ��
typedef struct{
    unsigned int extruder_heater_ms;        // ���������ȵ�ʱ��(ms)
    unsigned int extruder_fan_ms;           // ���������ȹ�����ʱ��(ms)
}heater_fan_work_time_t;


static heater_fan_work_time_t heater_fan_work_time = {0};
static struct platform_3dprinter_heater_fan_data *heater_fan_data = NULL;

// ����ģ��pwm�Ķ�ʱ��
static struct timer_list extruder_heater_timer;         // ���������ȵĶ�ʱ��
static struct timer_list extruder_fan_timer;            // ������ɢ�ȷ��ȵĶ�ʱ��
static DEFINE_MUTEX(heater_fan_lock);


// ������������
static void extruder_heater_enable(void)
{
    gpio_direction_output(heater_fan_data->extruder_heater_gpio, HEATER_WORK);
}

// ������������
static void extruder_heater_disable(void)
{
    gpio_direction_output(heater_fan_data->extruder_heater_gpio, HEATER_NOT_WORK);
}

// ���������ȶ�ʱ���ж�(ģ��pwm)
void extruder_heater_timer_timeout_fn(unsigned long arg)
{
    static int level = 0;
    
    // �ж��Ƿ���Ҫ����
    if (0 == heater_fan_work_time.extruder_heater_ms)
    {
        // ����Ҫ����
        extruder_heater_disable();
        mod_timer(&extruder_heater_timer, jiffies + HEATER_FAN_PWM_TIME_MS);
        level = 0;
        return ;
    }

    // �Ƿ�������pwm������һֱ����
    if (HEATER_FAN_PWM_TIME_MS <= heater_fan_work_time.extruder_heater_ms)
    {
        // һֱ����
        extruder_heater_enable();
        mod_timer(&extruder_heater_timer, jiffies + HEATER_FAN_PWM_TIME_MS);
        level = 0;
        return ;
    }

    // ģ��pwm
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
    // ֹͣ�������ļ��Ⱥͷ���
    extruder_heater_disable();
    // extruder_fan_disable();
    // cooling_fan_disable();
    heater_fan_work_time.extruder_heater_ms = 0;
    // heater_fan_work_time.extruder_fan_ms    = 0;
    
    return 0;
}


// ���Ƽ������ϵļ���ͷ��ɢ�ȷ���
static ssize_t heater_fan_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
    heater_fan_work_time_t work_time = {0};
    
    if (sizeof(heater_fan_work_time_t) != count)
    {
        // д������ݲ���
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

    // ����������ͷ
    ret = gpio_request(heater_fan_data->extruder_heater_gpio, "ls2k1000la_3dPrinter_heater_fan");
    if (0 > ret)
    {
        printk(KERN_ERR "[%s] request extruder heater gpio fail.", __FUNCTION__);
        return ret;
    }
    gpio_direction_output(heater_fan_data->extruder_heater_gpio, HEATER_NOT_WORK);

    // ��ʼ�����������ȵĶ�ʱ��(����ģ��pwm)
    heater_fan_work_time.extruder_heater_ms = 0;        // ����ͷ������
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

MODULE_AUTHOR("��о��ӡ");
MODULE_DESCRIPTION("ls2k1000la 3dprinter heater and fan driver");
MODULE_LICENSE("GPL");
