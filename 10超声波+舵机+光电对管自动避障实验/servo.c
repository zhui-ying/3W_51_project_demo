//舵机脉宽可调节范围 0.5ms -- 2.5ms
//对应的zhuanjiao参数 5 -- 25
#include "servo.h"
#include "interface.h"
unsigned char duoji_count=0;
unsigned char zhuanjiao = 15;

/*******************************************************************************
* 函 数 名 ：Delayms
* 函数功能 ：实现 ms级的延时
* 输    入 ：ms
* 输    出 ：无
*******************************************************************************/
static void Delayms(unsigned int ms)
{
	unsigned int i,j;
	for(i=0;i<ms;i++)
	#if FOSC == 11059200L
		for(j=0;j<114;j++);
	#elif FOSC == 12000000L
	  for(j=0;j<123;j++);
	#elif FOSC == 24000000L
		for(j=0;j<249;j++);
	#elif FOSC == 48000000L
		for(j=0;j<715;j++);
	#else
		for(j=0;j<114;j++);
	#endif
}

void DuojiMid()
{
	zhuanjiao = 15;
	Delayms(300);//延时
}

void DuojiRight()
{
	zhuanjiao = 8;
	Delayms(300);//延时
}

void DuojiLeft()
{
	zhuanjiao = 22;
	Delayms(300);//延时
}


