// 一些很常用的函数的实现

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// 将一个浮点数约束在一个范围内
// @num 被约束的数
// @min 范围的下限
// @max 范围的上限
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


