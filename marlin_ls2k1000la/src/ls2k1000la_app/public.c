// һЩ�ܳ��õĺ�����ʵ��

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// ��һ��������Լ����һ����Χ��
// @num ��Լ������
// @min ��Χ������
// @max ��Χ������
float constrain_float(const float num, const float min, const float max)
{
    if (min > num)
    {
        return min;
    }
    else if (max < num)
    {
        return max;
    }
    else
    {
        return num;
    }    
}


