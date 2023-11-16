#include "stdio.h"
#include "math.h"
#include "stdint.h"
#include "string.h"

uint8_t data_buff[500*1000]={0};//预留500kb
uint32_t data_len=0;//长度，在套入其他程序时注意保留原长度，//TODO 在计算内部flash时需要改进，不能直接在后面写入，分开来计算

// char data_fill[64]={0};//填充序列
// uint32_t data_fill_len=0;//填充序列长度


void get_data(void)
{
    memset(data_buff,0,data_len);//清空内存
    //将文件读入buff中，并获取长度用于后续操作


}


void data_fill(void)
{
    //考虑当前页剩余不足8字节的情况
    uint32_t quoetient=data_len/64;//
    uint32_t remainder=data_len%64;
    uint32_t temp_len;//指向末尾
    if((remainder>=56) && (remainder<64))//如果等于0刚好quoetient+1，但是如果>56 && <64,如果等于0刚好quoetient需要+2;
        temp_len=(quoetient+2)*64;//指向末尾
    else
        temp_len=(quoetient+1)*64;//指向末尾
    *(uint32_t *)(&data_buff[temp_len-8])=data_len*8;//写入bit数


    //填充阶段
    data_buff[data_len]=0x80;//填充一个1
    for(int i=1;i<temp_len-8-data_len;i++)
    {
        data_buff[data_len+i]=0x00;
    }
    data_len=temp_len;
}

void printf_data(void)
{
    for(int i=0;i<data_len;i++)
    {
        printf("%02x ",data_buff[i]);
    }
    printf("\n");
}



uint32_t F(uint32_t x,uint32_t y,uint32_t z)
{
    return (x&y)|((~x)&z);
}

uint32_t G(uint32_t x,uint32_t y,uint32_t z)
{
    return (x&z)|(y&(~z));
}

uint32_t H(uint32_t x,uint32_t y,uint32_t z)
{
    return x^y^z;
}

uint32_t I(uint32_t x,uint32_t y,uint32_t z)
{
    return y^(x|(~z));
}

const uint32_t CS64_16[64] = {  
0XD76AA478,0XE8C7B756,0X242070DB,0XC1BDCEEE,0XF57C0FAF,0X4787C62A,0XA8304613,0XFD469501,
0X698098D8,0X8B44F7AF,0XFFFF5BB1,0X895CD7BE,0X6B901122,0XFD987193,0XA679438E,0X49B40821,
0XF61E2562,0XC040B340,0X265E5A51,0XE9B6C7AA,0XD62F105D,0X02441453,0XD8A1E681,0XE7D3FBC8,
0X21E1CDE6,0XC33707D6,0XF4D50D87,0X455A14ED,0XA9E3E905,0XFCEFA3F8,0X676F02D9,0X8D2A4C8A,
0XFFFA3942,0X8771F681,0X6D9D6122,0XFDE5380C,0XA4BEEA44,0X4BDECFA9,0XF6BB4B60,0XBEBFBC70,
0X289B7EC6,0XEAA127FA,0XD4EF3085,0X04881D05,0XD9D4D039,0XE6DB99E5,0X1FA27CF8,0XC4AC5665,
0XF4292244,0X432AFF97,0XAB9423A7,0XFC93A039,0X655B59C3,0X8F0CCC92,0XFFEFF47D,0X85845DD1,
0X6FA87E4F,0XFE2CE6E0,0XA3014314,0X4E0811A1,0XF7537E82,0XBD3AF235,0X2AD7D2BB,0XEB86D391
};

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21



// #define FF(a, b, c, d, x, s, ac) {(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b);}
// #define GG(a, b, c, d, x, s, ac) {(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b);}
// #define HH(a, b, c, d, x, s, ac) {(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b);}
// #define II(a, b, c, d, x, s, ac) {(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b);}

void FF(uint32_t *a,uint32_t b,uint32_t c,uint32_t d,uint32_t mj,uint32_t s,uint32_t ti)
{
    uint32_t temp=*a+(F(b,c,d)+mj+ti);//进行F计算
    *a=b+((temp<<s)|(temp>>(32-s)));//循环位移后+b
}
void GG(uint32_t *a,uint32_t b,uint32_t c,uint32_t d,uint32_t mj,uint32_t s,uint32_t ti)
{
    uint32_t temp=*a+(G(b,c,d)+mj+ti);//进行G计算
    *a=b+((temp<<s)|(temp>>(32-s)));//循环位移后+b
}
void HH(uint32_t *a,uint32_t b,uint32_t c,uint32_t d,uint32_t mj,uint32_t s,uint32_t ti)
{
    uint32_t temp=*a+(H(b,c,d)+mj+ti);//进行H计算
    *a=b+((temp<<s)|(temp>>(32-s)));//循环位移后+b
}
void II(uint32_t *a,uint32_t b,uint32_t c,uint32_t d,uint32_t mj,uint32_t s,uint32_t ti)
{
    uint32_t temp=*a+(I(b,c,d)+mj+ti);//进行I计算
    *a=b+((temp<<s)|(temp>>(32-s)));//循环位移后+b
}






/********************************** 
 * @Date: 2023-11-01
 * @Author: 张晟毓
 * @description: 
 * @param uint32_t *a
 * @param uint32_t *b
 * @param uint32_t *c
 * @param uint32_t *d
 * @param uint8_t data
 * @return *
 **********************************/
void calculate(uint32_t *a,uint32_t *b,uint32_t *c,uint32_t *d,uint8_t data[64])
{
    uint32_t ta=*a;
    uint32_t tb=*b;
    uint32_t tc=*c;
    uint32_t td=*d;
    uint32_t x[16]={0};

    for(int i=0,j=0;i<64;i+=4,j++)
    {
        x[j]=(uint32_t)data[i]|((uint32_t)data[i+1]<<8)|((uint32_t)data[i+2]<<16)|((uint32_t)data[i+3]<<24);
    }

    /*
    abcd:幻数
    x:
    */

    /* Round 1 */
	FF (&ta, tb, tc, td, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (&td, ta, tb, tc, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (&tc, td, ta, tb, x[ 2], S13, 0x242070db); /* 3 */
	FF (&tb, tc, td, ta, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (&ta, tb, tc, td, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (&td, ta, tb, tc, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (&tc, td, ta, tb, x[ 6], S13, 0xa8304613); /* 7 */
	FF (&tb, tc, td, ta, x[ 7], S14, 0xfd469501); /* 8 */
	FF (&ta, tb, tc, td, x[ 8], S11, 0x698098d8); /* 9 */
	FF (&td, ta, tb, tc, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (&tc, td, ta, tb, x[10], S13, 0xffff5bb1); /* 11 */
	FF (&tb, tc, td, ta, x[11], S14, 0x895cd7be); /* 12 */
	FF (&ta, tb, tc, td, x[12], S11, 0x6b901122); /* 13 */
	FF (&td, ta, tb, tc, x[13], S12, 0xfd987193); /* 14 */
	FF (&tc, td, ta, tb, x[14], S13, 0xa679438e); /* 15 */
	FF (&tb, tc, td, ta, x[15], S14, 0x49b40821); /* 16 */
    printf("1. %x%x%x%x %x\n",ta,tb,tc,td,x[14]);
 
	/* Round 2 */
	GG (&ta, tb, tc, td, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (&td, ta, tb, tc, x[ 6], S22, 0xc040b340); /* 18 */
	GG (&tc, td, ta, tb, x[11], S23, 0x265e5a51); /* 19 */
	GG (&tb, tc, td, ta, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (&ta, tb, tc, td, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (&td, ta, tb, tc, x[10], S22,  0x2441453); /* 22 */
	GG (&tc, td, ta, tb, x[15], S23, 0xd8a1e681); /* 23 */
	GG (&tb, tc, td, ta, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (&ta, tb, tc, td, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (&td, ta, tb, tc, x[14], S22, 0xc33707d6); /* 26 */
	GG (&tc, td, ta, tb, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (&tb, tc, td, ta, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (&ta, tb, tc, td, x[13], S21, 0xa9e3e905); /* 29 */
	GG (&td, ta, tb, tc, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (&tc, td, ta, tb, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (&tb, tc, td, ta, x[12], S24, 0x8d2a4c8a); /* 32 */
    printf("1. %x%x%x%x %x\n",ta,tb,tc,td,x[14]);
	/* Round 3 */
	HH (&ta, tb, tc, td, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (&td, ta, tb, tc, x[ 8], S32, 0x8771f681); /* 34 */
	HH (&tc, td, ta, tb, x[11], S33, 0x6d9d6122); /* 35 */
	HH (&tb, tc, td, ta, x[14], S34, 0xfde5380c); /* 36 */
 	HH (&ta, tb, tc, td, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (&td, ta, tb, tc, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (&tc, td, ta, tb, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (&tb, tc, td, ta, x[10], S34, 0xbebfbc70); /* 40 */
	HH (&ta, tb, tc, td, x[13], S31, 0x289b7ec6); /* 41 */
	HH (&td, ta, tb, tc, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (&tc, td, ta, tb, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (&tb, tc, td, ta, x[ 6], S34,  0x4881d05); /* 44 */
	HH (&ta, tb, tc, td, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (&td, ta, tb, tc, x[12], S32, 0xe6db99e5); /* 46 */
	HH (&tc, td, ta, tb, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (&tb, tc, td, ta, x[ 2], S34, 0xc4ac5665); /* 48 */
    printf("1. %x%x%x%x %x\n",ta,tb,tc,td,x[14]);
	/* Round 4 */
	II (&ta, tb, tc, td, x[ 0], S41, 0xf4292244); /* 49 */
	II (&td, ta, tb, tc, x[ 7], S42, 0x432aff97); /* 50 */
	II (&tc, td, ta, tb, x[14], S43, 0xab9423a7); /* 51 */
	II (&tb, tc, td, ta, x[ 5], S44, 0xfc93a039); /* 52 */
	II (&ta, tb, tc, td, x[12], S41, 0x655b59c3); /* 53 */
	II (&td, ta, tb, tc, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (&tc, td, ta, tb, x[10], S43, 0xffeff47d); /* 55 */
	II (&tb, tc, td, ta, x[ 1], S44, 0x85845dd1); /* 56 */
 	II (&ta, tb, tc, td, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (&td, ta, tb, tc, x[15], S42, 0xfe2ce6e0); /* 58 */
 	II (&tc, td, ta, tb, x[ 6], S43, 0xa3014314); /* 59 */
	II (&tb, tc, td, ta, x[13], S44, 0x4e0811a1); /* 60 */
	II (&ta, tb, tc, td, x[ 4], S41, 0xf7537e82); /* 61 */
	II (&td, ta, tb, tc, x[11], S42, 0xbd3af235); /* 62 */
	II (&tc, td, ta, tb, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (&tb, tc, td, ta, x[ 9], S44, 0xeb86d391); /* 64 */
    printf("1. %x%x%x%x %x\n",ta,tb,tc,td,x[14]);
    *a+=ta;
    *b+=tb;
    *c+=tc;
    *d+=td;
    printf("1. %x%x%x%x %x\n",ta,tb,tc,td,x[14]);
}






int main(void)
{
    scanf("%s",data_buff);
    data_len=strlen(data_buff);

    printf("data_len is %d\n",data_len);

    data_fill();

    printf("data_len is %d\n",data_len);
    printf_data();







    uint32_t a=0x67452301;//幻数
    uint32_t b=0xefcdab89;//幻数
    uint32_t c=0x98badcfe;//幻数
    uint32_t d=0x10325476;//幻数


    for(int i=0;i<data_len/64;i++)//以64字节进行计算
    {
        //取出64字节
        calculate(&a,&b,&c,&d,&data_buff[i*64]);
    }

    
    for(int i=0;i<4;i++)
    {
        printf("%x",((uint8_t*)&a)[i]);
    }
    for(int i=0;i<4;i++)
    {
        printf("%x",((uint8_t*)&b)[i]);
    }
    for(int i=0;i<4;i++)
    {
        printf("%x",((uint8_t*)&c)[i]);
    }
    for(int i=0;i<4;i++)
    {
        printf("%x",((uint8_t*)&d)[i]);
    }
    //以字节的方式顺序打印






    return 0;
}
