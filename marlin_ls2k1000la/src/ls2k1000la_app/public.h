// ����ͷ�ļ�
// ���һЩ�����õ��ĺꡢ������

#ifndef __PUBLIC_H
#define __PUBLIC_H


enum
{
    ERROR = -1,
    SUCCESS = 0,
};



// Macros for bit masks
#define BV(n)               (1<<(n))
#define TEST_BIT(n,b)       (((n)&BV(b))!=0)
#define SBI(n,b)            ((n) |= BV(b))
#define CBI(n,b)            ((n) &= ~BV(b))
#define SET_BIT(n,b,value)  (n) ^= ((-value)^(n)) & (BV(b))


// �Ƿ�Ϊ�ַ���������
#define IS_END_OF_STRING(ch)    ('\0'==(ch))
// �Ƿ�Ϊ�س�����
#define IS_END_OF_LINE(ch)      (('\r'==(ch)) || ('\n'==(ch)))
// �ж��Ƿ�Ϊע���ַ�
#define IS_COMMENT_FLAG(ch)                 (';'==(ch))
// �ж��Ƿ�Ϊ�ո�
#define IS_BACKSPACE(ch)                    (' '==(ch))
// �ж��Ƿ�Ϊ����
#define IS_NUMERIC(n)                       (('0'<=n) && ('9'>=n))


// Macros to contrain values
#define NOLESS(v,n)     do{ if((v)<(n)) (v)=(n); } while(0)
#define NOMORE(v,n)     do{ if((v)>(n)) (v)=(n); } while(0)

// ƽ��
#define SQUARE(n)       ((n)*(n))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))



#ifndef TRUE
#define TRUE                    (1)
#endif

#ifndef FALSE
#define FALSE                   (0)
#endif

#define DEBUG

#ifdef DEBUG
#define debug_printf(format,...)        printf(format, ##__VA_ARGS__);
#else
#define debug_printf(format,...)
#endif

typedef unsigned char           BOOL;
typedef char                    INT8;
typedef unsigned char           UINT8;
typedef short                   INT16;
typedef unsigned short          UINT16;
typedef int                     INT32;
typedef unsigned int            UINT32;
typedef float                   FLOAT;


// ��һ��������Լ����һ����Χ��
// @num ��Լ������
// @min ��Χ������
// @max ��Χ������
float constrain_float(const float num, const float min, const float max);


#endif

