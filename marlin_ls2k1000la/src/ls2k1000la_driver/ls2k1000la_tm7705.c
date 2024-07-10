/*
 *
 * ˫·16λspi�ӿ�ADоƬ--TM7705������
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
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>




// ͨ�żĴ���bit����
enum 
{
	// �Ĵ���ѡ��  RS2 RS1 RS0
	TM7705_REG_COMM	        = (0 << 4), // ͨ�żĴ���
	TM7705_REG_SETUP	    = (1 << 4), // ���üĴ���
	TM7705_REG_CLOCK	    = (2 << 4), // ʱ�ӼĴ���
	TM7705_REG_DATA	        = (3 << 4), // ���ݼĴ���
	TM7705_REG_TEST         = (4 << 4), // ���ԼĴ���
	TM7705_REG_OFFSET       = (6 << 4), // ƫ�ƼĴ���
	TM7705_REG_GAIN         = (7 << 4), // ����Ĵ���
	
    // ��д����
	TM7705_WRITE 		    = (0 << 3), // д����
	TM7705_READ 		    = (1 << 3), // ������

	// ͨ��
	TM7705_CH_1		        = 0,    // AIN1+  AIN1-
	TM7705_CH_2		        = 1,    // AIN2+  AIN2-
	TM7705_CH_3		        = 2,    // AIN1-  AIN1-
	TM7705_CH_4		        = 3     // AIN1-  AIN2-
};


/* ���üĴ���bit���� */
enum
{
	TM7705_MD_NORMAL		= (0 << 6),	/* ����ģʽ */
	TM7705_MD_CAL_SELF		= (1 << 6),	/* ��У׼ģʽ */
	TM7705_MD_CAL_ZERO		= (2 << 6),	/* У׼0�̶�ģʽ */
	TM7705_MD_CAL_FULL		= (3 << 6),	/* У׼���̶�ģʽ */

	TM7705_GAIN_1			= (0 << 3),	/* ���� */
	TM7705_GAIN_2			= (1 << 3),	/* ���� */
	TM7705_GAIN_4			= (2 << 3),	/* ���� */
	TM7705_GAIN_8			= (3 << 3),	/* ���� */
	TM7705_GAIN_16			= (4 << 3),	/* ���� */
	TM7705_GAIN_32			= (5 << 3),	/* ���� */
	TM7705_GAIN_64			= (6 << 3),	/* ���� */
	TM7705_GAIN_128		    = (7 << 3),	/* ���� */

	/* ����˫���Ի��ǵ����Զ����ı��κ������źŵ�״̬����ֻ�ı�������ݵĴ����ת�������ϵ�У׼�� */
	TM7705_BIPOLAR			= (0 << 2),	/* ˫�������� */
	TM7705_UNIPOLAR		    = (1 << 2),	/* ���������� */

	TM7705_BUF_NO			= (0 << 1),	/* �����޻��壨�ڲ�������������) */
	TM7705_BUF_EN			= (1 << 1),	/* �����л��� (�����ڲ�������) */

	TM7705_FSYNC_0			= 0,    // ģ����������˲���������������
	TM7705_FSYNC_1			= 1		// ģ����������˲���������
};



/* ʱ�ӼĴ���bit���� */
enum
{
	TM7705_CLKDIS_0	        = (0 << 4),		/* ʱ�����ʹ�� ������Ӿ���ʱ������ʹ�ܲ����񵴣� */
	TM7705_CLKDIS_1	        = (1 << 4),		/* ʱ�ӽ�ֹ �����ⲿ�ṩʱ��ʱ�����ø�λ���Խ�ֹMCK_OUT�������ʱ����ʡ�� */

    TM7705_CLKDIV_0         = (0 << 3),     // ����Ƶ
    TM7705_CLKDIV_1         = (1 << 3),     // 2��Ƶ���ⲿ����Ϊ4.9152Mhzʱ��Ӧ2��Ƶ

    TM7705_CLK_0            = (0 << 2),     // ��ʱ��=1Mhz����CLKDIV=0����ʱ��=2Mhz����CLKDIV=1
    TM7705_CLK_1            = (1 << 2),     // ��ʱ��=2.4576Mhz����CLKDIV=0, ��ʱ��=4.9152Mhz����CLKDIV=1

    // ע�������������clkλ�й�
    // ��TM7705_CLK_0ʱ�����������ֻ��Ϊ20,25,100,200
    TM7705_UPDATE_20        = (0),
    TM7705_UPDATE_25        = (1),
    TM7705_UPDATE_100       = (2),
    TM7705_UPDATE_200       = (3),
    // ��TM7705_CLK_1ʱ�����������ֻ��Ϊ50,60,250,500
    TM7705_UPDATE_50        = (0),
    TM7705_UPDATE_60        = (1),
    TM7705_UPDATE_250       = (2),
    TM7705_UPDATE_500       = (3)
};



#define TM7705_CHANNEL_NUM              (2)     // tm7705ͨ������
#define TM7705_DRDY_PIN                 (87)    // GPIO87/I2S_DI   tm7705������DRDY 
#define TM7705_RESET_PIN                (89)    // GPIO89/I2S_LRCK  tm7705������RESET


struct tm7705 {
    struct device *hwmon_dev;
    struct mutex lock;
};



// ͨ��reset�Ÿ�λtm7705
static void tm7705_reset(void)
{
    gpio_direction_output(TM7705_RESET_PIN, 1);
    msleep(1);
    gpio_direction_output(TM7705_RESET_PIN, 0);
    msleep(2);
    gpio_direction_output(TM7705_RESET_PIN, 1);
    msleep(1);

    return ;
}


// ͬ��spi�ӿ�ʱ��
static void tm7705_sync_spi(struct spi_device *spi)
{
    u8 tx_buf[4] = {0xFF};
    
    // ����32������ʱ������TM7705��DIN��д���߼�"1"
    spi_write(spi, tx_buf, sizeof(tx_buf));

    return ;
}


// �ȴ��ڲ��������
static int tm7705_wait_DRDY(void)
{
    int i = 0;
    int time_cnt = 500;

    for (i=0; i<time_cnt; i++)
    {
        if (0 == gpio_get_value(TM7705_DRDY_PIN))
        {
            break;
        }
        msleep(1);
    }

    if (i >= time_cnt)
    {
        return -1;
    }

    return 0;
}


// ��У׼
static void tm7705_calib_self(struct spi_device *spi, u8 channel)
{
    u8 tx_buf[2] = {0};

    tx_buf[0] = TM7705_REG_SETUP | TM7705_WRITE | channel;
    tx_buf[1] = TM7705_MD_CAL_SELF | TM7705_GAIN_1 | TM7705_UNIPOLAR | TM7705_BUF_EN | TM7705_FSYNC_0;
    spi_write(spi, tx_buf, sizeof(tx_buf));

    tm7705_wait_DRDY();         /* �ȴ��ڲ�������� --- ʱ��ϳ���Լ180ms */

    msleep(50);
    
    return ;
}


// ����tm7705��ָ��ͨ��
static void tm7705_config_channel(struct spi_device *spi, u8 channel)
{
    u8 tx_buf[2] = {0};

    tx_buf[0] = TM7705_REG_CLOCK | TM7705_WRITE | channel;
    tx_buf[1] = TM7705_CLKDIS_0 | TM7705_CLKDIV_1 | TM7705_CLK_1 | TM7705_UPDATE_50;
    spi_write(spi, tx_buf, sizeof(tx_buf));

    // ��У׼
    tm7705_calib_self(spi, channel);

    return ;
}


// ��λtm7705����������
static void tm7705_reset_and_reconfig(struct spi_device *spi)
{
    // ͨ��reset�Ÿ�λtm7705
    tm7705_reset();

    // ͬ��spi�ӿ�ʱ��
    msleep(5);
    tm7705_sync_spi(spi);
    msleep(5);

    // ����tm7705ʱ�ӼĴ���
    tm7705_config_channel(spi, TM7705_CH_1);
//    tm7705_config_channel(spi, TM7705_CH_2);
    
    return ;
}


/*
 * ��ȡһ��ͨ����ֵ
 * @dev �豸������
 * @channel ͨ��
 * ad ������adֵ
 */
static int tm7705_read_channel(struct device *dev, u8 channel, u16 *ad)
{
    struct spi_device *spi = to_spi_device(dev);
    struct tm7705 *adc = spi_get_drvdata(spi);
    int ret = 0;
    u16 value = 9;
    u8 tx_buf[1] = {0};
    u8 rx_buf[2] = {0};

    if (mutex_lock_interruptible(&adc->lock))
    {
        return -ERESTARTSYS;
    }

    // �ȴ�ת�����
    ret = tm7705_wait_DRDY();
    if(ret)
    {
        printk(KERN_ERR "[%s] tm7705_wait_DRDY() time out.\n", __FUNCTION__);
        goto fail;
    }
    
    tx_buf[0] = TM7705_REG_DATA | TM7705_READ | channel;
    ret = spi_write_then_read(spi, tx_buf, sizeof(tx_buf), rx_buf, sizeof(rx_buf));
    value = (rx_buf[0]<<8) + rx_buf[1];
    if (0 > ret)    // spiͨ��ʧ��
    {
        printk(KERN_ERR "[%s] tm7705_read_byte() fail. ret=%d\n", __FUNCTION__, ret);
        goto fail;
    }
    if (0xfff == value)  // tm7705�ϵ�һ��ʱ��󣬿��ܻ���ֶ�����ֵһֱ��0xfff�����
    {
        printk(KERN_ERR "[%s] value=0xfff\n", __FUNCTION__);
        ret = -1;
        goto fail;
    }

    // ���ADֵ
    *ad = value;

fail:
    mutex_unlock(&adc->lock);
    return ret;
}


/* sysfs hook function */
static ssize_t tm7705_get_sensor_value(struct device *dev,
                           struct device_attribute *devattr,
                           char *buf)
{
    struct spi_device *spi = to_spi_device(dev);
    struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
    int ret = 0;
    u16 ad = 0;
    int i = 0;

    /*
	 * Ϊ�˱���ͨ���л���ɶ���ʧЧ����2��
	 * ʵ����ÿ�ζ���������һ�βɼ��Ľ��(��������ͨ������ɼ����ܿ���Ч��)
	 */
    for (i=0; i<2; i++)
    {
        ret = tm7705_read_channel(dev, attr->index, &ad);
        if (ret)
        {
            // ʧ�ܣ�������tm7705����������
            tm7705_reset_and_reconfig(spi);
            printk(KERN_ERR "[%s] tm7705 reset and reconfig.\n", __FUNCTION__);
            return ret;
        }
        printk(KERN_DEBUG "[%s] tm7705 ad=0x%x\n", __FUNCTION__, ad);
        
        // ls1c���ٶ��൱TM7705̫�죬��ʱһ�±�����һ�ζ����DRDY��δ��ʱ�ı�״̬ls1c�ֿ�ʼ����һ�ζ�д
        msleep(1);
    }

    // ��adֵ���ݸ��û�����
    ret = sprintf(buf, "%u\n", ad);
    
    return ret;
}




static struct sensor_device_attribute ad_input[] = {
    SENSOR_ATTR(ch1, S_IRUGO, tm7705_get_sensor_value, NULL, TM7705_CH_1),
    SENSOR_ATTR(ch2, S_IRUGO, tm7705_get_sensor_value, NULL, TM7705_CH_2),
};


static int __devinit tm7705_probe(struct spi_device *spi)
{
    struct tm7705 *adc;
    int i;
    int status;

    adc = kzalloc(sizeof *adc, GFP_KERNEL);
    if (!adc)
    {
        return -ENOMEM;
    }

    mutex_init(&adc->lock);
    mutex_lock(&adc->lock);
    
    spi_set_drvdata(spi, adc);
    for (i=0; i<TM7705_CHANNEL_NUM; i++)
    {
        status = device_create_file(&spi->dev, &ad_input[i].dev_attr);
        if (status)
        {
            dev_err(&spi->dev, "device_create_file() failed.\n");
            goto fail_crete_file;
        }
    }

    adc->hwmon_dev = hwmon_device_register(&spi->dev);
    if (IS_ERR(adc->hwmon_dev))
    {
        dev_err(&spi->dev, "hwmon_device_register() fail.\n");
        status = PTR_ERR(adc->hwmon_dev);
        goto fail_crete_file;
    }

    // gpio��ʼ��
    status = gpio_request(TM7705_DRDY_PIN, "TM7705");   // tm7705 DRDY pin
    if (status)
    {
        dev_err(&spi->dev, "gpio_request(TM7705_DRDY_PIN) fail.\n");
        goto fail_device_register;
    }
    gpio_direction_input(TM7705_DRDY_PIN);
    status = gpio_request(TM7705_RESET_PIN, "TM7705");  // tm7705 reset pin
    if (status)
    {
        dev_err(&spi->dev, "gpio_request(TM7705_RESET_PIN) fail.\n");
        goto fail_request_drdy_pin;
    }
    gpio_direction_output(TM7705_RESET_PIN, 1);

    // ��λtm7705����������
    tm7705_reset_and_reconfig(spi);

    mutex_unlock(&adc->lock);
    return 0;

fail_request_drdy_pin:
    gpio_free(TM7705_DRDY_PIN);
fail_device_register:
    hwmon_device_unregister(adc->hwmon_dev);
fail_crete_file:
    for (i--; i>=0; i--)
    {
        device_remove_file(&spi->dev, &ad_input[i].dev_attr);
    }
    spi_set_drvdata(spi, NULL);
    mutex_unlock(&adc->lock);
    kfree(adc);
    
    return status;
}


static int __devexit tm7705_remove(struct spi_device *spi)
{
    struct tm7705 *adc = spi_get_drvdata(spi);
    int i;

    mutex_lock(&adc->lock);

    gpio_free(TM7705_DRDY_PIN);
    gpio_free(TM7705_DRDY_PIN);
    hwmon_device_unregister(adc->hwmon_dev);
    for (i=0; i<TM7705_CHANNEL_NUM; i++)
    {
        device_remove_file(&spi->dev, &ad_input[i].dev_attr);
    }
    spi_set_drvdata(spi, NULL);

    mutex_unlock(&adc->lock);
    
    kfree(adc);
    return 0;
}


static struct spi_driver tm7705_driver = {
    .driver = {
        .name = "TM7705",
        .owner = THIS_MODULE,
    },
    .probe = tm7705_probe,
    .remove = __devexit_p(tm7705_remove),
};



static int __init init_tm7705(void)
{
    return spi_register_driver(&tm7705_driver);
}


static void __exit exit_tm7705(void)
{
    spi_unregister_driver(&tm7705_driver);
}


module_init(motor_init);
module_exit(motor_exit);

MODULE_AUTHOR("��о��ӡ");
MODULE_DESCRIPTION("ls2k1000la 3dprinter motor driver");
MODULE_LICENSE("GPL");
