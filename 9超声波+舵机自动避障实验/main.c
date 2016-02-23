/********************************* 深圳市航太电子有限公司 *******************************
* 实 验 名 ：小车超声波+舵机自动避障实验
* 实验说明 ：将超声波模块放在舵机旋转轴上，按下KEY2启动键后，通过转动舵机，来获取前方，
*            左边以及右边的障碍物距离，以此判断最佳行走路线
* 实验平台 ：流星5号、51单片机最小系统
* 连接方式 ：请参考interface.h文件
* 注    意 ：1、请将舵机接口线接向CN9
*            2、请详细阅读超声波相关资料，超声波模块是有个测量角度的约30°，若前方障碍物与
*            3、因舵机精度要求，这里选择定时器基准时钟为0.1ms
             4、超声波倾角过大，会造成超声波模块测距不准
						 5、本实验不需要显示屏，注意将屏幕取下，避免影响超声波
* 作    者 ：航太电子产品研发部    QQ ：1909197536
* 店    铺 ：http://shop120013844.taobao.com/
****************************************************************************************/

#include <reg52.h>
#include <intrins.h>
#include <stdio.h>
#include "interface.h"
#include "motor.h"
#include "UltrasonicCtrol.h"
#include "servo.h"

//全局变量定义
unsigned int speed_count=0;//占空比计数器 50次一周期
char front_left_speed_duty=SPEED_DUTY;
char front_right_speed_duty=SPEED_DUTY;

unsigned char tick_5ms = 0;//5ms计数器，作为主函数的基本周期
unsigned char tick_1ms = 0;//1ms计数器，作为电机的基本计数器
unsigned char tick_200ms = 0;
unsigned int tick_1s = 0;
unsigned char switch_flag=0;
volatile unsigned long system_tick=0;//系统时钟 1ms自加

char sys_status = 0;//系统状态 0 停止 1 运行
char ctrl_comm;//控制指令
char ctrl_comm_last;


/*******************************************************************************
* 函 数 名 ：Delayms
* 函数功能 ：实现 ms级的延时
* 输    入 ：ms
* 输    出 ：无
*******************************************************************************/
//void Delayms(unsigned int ms)
//{
//	unsigned int i,j;
//	for(i=0;i<ms;i++)
//	#if FOSC == 11059200L
//		for(j=0;j<114;j++);
//	#elif FOSC == 12000000L
//	  for(j=0;j<123;j++);
//	#elif FOSC == 24000000L
//		for(j=0;j<249;j++);
//	#elif FOSC == 48000000L
//		for(j=0;j<715;j++);
//	#else
//		for(j=0;j<114;j++);
//	#endif
//}
void Delayms(unsigned int ms)//由于中断占用了较多的时间，这里换一种延时方法
{
	unsigned long start = system_tick;
	
	while(system_tick - start < ms);
}

/*******************************************************************************
* 函 数 名 ：Timer0Init
* 函数功能 ：定时器0初始化
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void Timer0Init()
{
	TMOD|=0x01; //设置定时器0工作方式为1
	TH0=(65536-(FOSC/12*TIME_US)/1000000)/256;
	TL0=(65536-(FOSC/12*TIME_US)/1000000)%256;
	ET0=1; //开启定时器0中断
	TR0=1;	//开启定时器	
	EA=1;  //打开总中断
}

///获取三个方向的距离,进来前舵机方向为向前
void GetAllDistance(unsigned int *dis_left,unsigned int *dis_right,unsigned int *dis_direct)
{
	CarStop();
	Delayms(100);
	GetDistanceDelay();
	*dis_direct = distance_cm;
	
	DuojiRight();
	Delayms(200);
	GetDistanceDelay();//获取右边距离
	*dis_right = distance_cm;
	
	DuojiMid();
	DuojiLeft();
	Delayms(200);
	GetDistanceDelay();//获取左边距离
	*dis_left = distance_cm;
	
	DuojiMid();//归位
}

void VoidRun()
{
	 if(distance_cm < 10)//前方有障碍物
	 {
		unsigned int dis_left;//左边距离
		unsigned int dis_right;//右边距离
		unsigned int dis_direct;//中间距离
		if(distance_cm < 8)
		{
			CarBack();
			Delayms(300);
		}
		
		while(1)
		{
			GetAllDistance(&dis_left,&dis_right,&dis_direct);
			if(dis_direct > 50)//前方距离已经有50cm以上了就不需要管左右了，直接前进
			{
				CarGo();
				Delayms(500);
				return;				
			}
			if(dis_direct < 8)
			{
				CarBack();
				Delayms(500);
				continue;
			}
			else if(dis_direct >= dis_left && dis_direct >= dis_right)//前方距离最远
			{
				CarGo();
				return;
			}
			else if(dis_left <= dis_right)//右转
			{
				CarRight();
				Delayms(700);
			}
			else if(dis_right < dis_left)
			{
				CarLeft();
				Delayms(700);
			}
		}
	}
	else
	{
		CarGo();
	}
}

//读取键值，如果没读到返回 -1
int KeyScan(void)
{
	int value = -1;
	KEY1_IO = 1;
	KEY2_IO = 1;
	if(KEY_DOWN == KEY1_IO)
	{
		Delayms(5);//Delayms for a while
		if(KEY_DOWN == KEY1_IO)
		{
			value = KEY1;
		}
		while(KEY_DOWN == KEY1_IO);
	}

	if(KEY_DOWN == KEY2_IO)
	{
		Delayms(5);//Delayms for a while
		if(KEY_DOWN == KEY2_IO)
		{
			value = KEY2;
		}
		while(KEY_DOWN == KEY2_IO);
	}
	return value;
}

/*******************************************************************************
* 函 数 名 ：main
* 函数功能 ：主函数
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void main()
{
	int key_value;
	CarStop();	
	LED_OFF;
	UltraSoundInit();	
	Timer0Init();
	//5ms 执行一次
	while(1)
	{ 
		key_value = KeyScan();
		if(key_value == KEY1) 
		{
			sys_status = 0;
		}
			
		if(key_value == KEY2) 
		{
			sys_status = 1;
		}
		
		//执行部分
		if(tick_5ms >= 5)
		{
			tick_5ms = 0;
			if(sys_status == 1)
			{
				Distance();
				VoidRun();
			}else
			{
				CarStop();
			}  
		}   
	}
}

/*******************************************************************************
* 函 数 名 ：Timer0Int
* 函数功能 ：定时器0中断函数 ， 每TIME_US ms进入
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void Timer0Int() interrupt 1
{
	TH0=(65536-(FOSC/12*TIME_US)/1000000)/256;
	TL0=(65536-(FOSC/12*TIME_US)/1000000)%256;
	tick_1ms++;
	if(tick_1ms >= 10)
	{
		system_tick++;
		tick_1ms = 0;	
		tick_5ms++;
	}
	speed_count++;
	if(speed_count >= 50)//5ms
	{
		speed_count = 0;
		
		tick_200ms++;
		if(tick_200ms >= 40)
		{
			tick_200ms = 0;
			if(switch_flag)
			{
				LED_ON;
				switch_flag = 0;
			}else
			{
				LED_OFF;
				switch_flag = 1;				
			}
		}
	}
	CarMove();
	
	duoji_count++;
	if(duoji_count <= zhuanjiao)
	{
		DUOJI_IO = 0; //后面加了一个驱动，这边需要反相 modfied by LC 2015.09.19 22:39
	}
	else
	{
		DUOJI_IO = 1;
	}
	if(duoji_count >= 200)//20ms
	{
		duoji_count = 0;
	}
	
}


