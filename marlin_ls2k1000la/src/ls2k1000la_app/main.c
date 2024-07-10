#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "public.h"
#include "temp.h"
#include "gcode.h"
#include "planner.h"
#include "motor.h"

int main(void)
{
    int ret = 0;
    pthread_t temp_tid;

    // ���������ʼ��
    ret = motor_init();
    if (SUCCESS != ret)
    {
        return ret;
    }
    gcode_init();
    plan_init();

    // ����һ���߳����ڿ����¶�
    ret = pthread_create(&temp_tid, NULL, temp_thread_fn, "Temp Thread");
    if (0 != ret)
    {
        printf("[%s] create temp thread fail.\n", __FUNCTION__);
        return ret;
    }

    gcode_M24();
}

