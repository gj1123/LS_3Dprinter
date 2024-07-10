// �ṩ���������ؽӿ�

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "public.h"
#include "motor.h"


// ������״̬��Ϣ
typedef struct
{
    // block�����Ƿ�����
    // ����Ӧ�ó���������дblockʱ�жϻ����Ƿ�����
    char cmd_block_fifo_full;   // true or false

    // fifo������step����ִ��
    // fifo��û��block���������һ��blockҲ��ȫ��ִ�����
    char buffered_steps_executed;   // true or false
} motor_status_t;


// ������������豸�ļ�������
int fd_motor = 0;

// �����ʼ��
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


// ��ȡ�����е��״̬
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


// �ж������е�block fifo�Ƿ��Ѿ�����
// @ret TRUE or FALSE
int motor_block_fifo_is_full(void)
{
    motor_status_t status;
    motor_get_status(&status);
    return status.cmd_block_fifo_full;
}


// fifo������step����ִ��
// fifo��û��block���������һ��blockҲ��ȫ��ִ�����
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


// ������д��block
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
