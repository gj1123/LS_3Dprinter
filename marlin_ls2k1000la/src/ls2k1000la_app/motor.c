// 提供电机驱动相关接口

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "public.h"
#include "motor.h"


// 驱动的状态信息
typedef struct
{
    // block缓存是否已满
    // 用于应用程序往驱动写block时判断缓存是否已满
    char cmd_block_fifo_full;   // true or false

    // fifo中所有step都已执行
    // fifo总没有block，并且最后一个block也已全部执行完成
    char buffered_steps_executed;   // true or false
} motor_status_t;


// 步进电机驱动设备文件描述符
int fd_motor = 0;

// 电机初始化
int motor_init(void)
{
    fd_motor = open("/dev/3dPrinter_motor", O_RDWR);
    if (-1 == fd_motor)
    {
        printf("[%s] open file /dev/3dPrinter_motor fail.\n", __FUNCTION__);
        return ERROR;
    }
    return SUCCESS;
}


// 获取驱动中电机状态
void motor_get_status(motor_status_t *status)
{
    int ret = 0;
    ret = read(fd_motor, status, sizeof(motor_status_t));
    if (sizeof(motor_status_t) != ret)
    {
        printf("[%s] read fail. ret=%d\n", __FUNCTION__, ret);
        return;
    }
    return;
}


// 判断驱动中的block fifo是否已经满了
// @ret TRUE or FALSE
int motor_block_fifo_is_full(void)
{
    motor_status_t status;
    motor_get_status(&status);
    return status.cmd_block_fifo_full;
}


// fifo中所有step都已执行
// fifo总没有block，并且最后一个block也已全部执行完成
// @ret TRUE or FALSE
int motor_buffered_steps_is_executed(void)
{
    motor_status_t status;
    motor_get_status(&status);
    return status.buffered_steps_executed;
}


// Block until all buffered steps are executed
void motor_synchronize(void)
{
    while (TRUE != motor_buffered_steps_is_executed())
    {
        sleep(1);
    }
    return ;
}


// 向驱动写入block
int motor_write_block(motor_cmd_block_t *block_p)
{
    int ret = 0;
    ret = write(fd_motor, block_p, sizeof(motor_cmd_block_t));
    if (sizeof(motor_cmd_block_t) != ret)
    {
        printf("[%s] write block fail. ret=%d\n", __FUNCTION__, ret);
        return ERROR;
    }
    return SUCCESS;
}
