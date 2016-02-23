/********************************* 深圳市航太电子有限公司 *******************************
* 实 验 名 ：小车自动循迹实验实验
* 实验说明 ：按下KEY2启动键后，小车根据车前三个光电对管的反馈开始自动沿黑线行驶
* 实验平台 ：流星5号、51单片机最小系统
* 连接方式 ：请参考interface.h文件
* 注    意 ：根据环境不同，需要调节光电对管对应的三个电位器，使黑线检测达到最佳状态
* 作    者 ：航太电子产品研发部    QQ ：1909197536
* 店    铺 ：http://shop120013844.taobao.com/
****************************************************************************************/

#include <reg52.h>
#include <intrins.h>
#include <stdio.h>
#include "LCD1602.h"
#include "interface.h"
#include "motor.h"

//全局变量定义
unsigned int speed_count=0;//占空比计数器 50次一周期
char front_left_speed_duty=SPEED_DUTY;
char front_right_speed_duty=SPEED_DUTY;

unsigned char tick_5ms = 0;//5ms计数器，作为主函数的基本周期
unsigned char tick_1ms = 0;//1ms计数器，作为电机的基本计数器
unsigned char tick_200ms = 0;
unsigned int tick_1s = 0;
unsigned char switch_flag=0;

char sys_status = 0;//系统状态 0 停止 1 运行
char ctrl_comm;//控制指令
char ctrl_comm_last;

/*******************************************************************************
* 函 数 名 ：Delayms
* 函数功能 ：实现 ms级的延时
* 输    入 ：ms
* 输    出 ：无
*******************************************************************************/
void Delayms(unsigned int ms)
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

//读取键值，如果没读到返回 -1
int KeyScan(void)
{
	int value = -1;
	KEY1_IO = 1;
	KEY2_IO = 1;
	if(KEY_DOWN == KEY1_IO)
	{
		Delayms(5);//delay for a while
		if(KEY_DOWN == KEY1_IO)
		{
			value = KEY1;
		}
		while(KEY_DOWN == KEY1_IO);
	}

	if(KEY_DOWN == KEY2_IO)
	{
		Delayms(5);//delay for a while
		if(KEY_DOWN == KEY2_IO)
		{
			value = KEY2;
		}
		while(KEY_DOWN == KEY2_IO);
	}
	return value;
}


//循迹，通过判断三个光电对管的状态来控制小车运动
void SearchRun()
{
	//三路都检测到 前进
	if(SEARCH_M_IO == BLACK_AREA && SEARCH_L_IO == BLACK_AREA && SEARCH_R_IO == BLACK_AREA)
	{
		ctrl_comm = COMM_UP;
		return;
	}
	
	if(SEARCH_R_IO == BLACK_AREA)//右转
	{
		ctrl_comm = COMM_RIGHT;
	}
	else if(SEARCH_L_IO == BLACK_AREA)//左转
	{
		ctrl_comm = COMM_LEFT;
	}
	else if(SEARCH_M_IO == BLACK_AREA)//前进
	{
		ctrl_comm = COMM_UP;
	}
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
	LCD1602Init();
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
			if(sys_status == 1)
			{
				SearchRun();
			}else
			{
				ctrl_comm = COMM_STOP;
			}
			
			//do something
			if(ctrl_comm_last != ctrl_comm)//接收到红外信号
			{
				ctrl_comm_last = ctrl_comm;
				switch(ctrl_comm)
				{
				case COMM_UP:    CarGo();break;
				case COMM_DOWN:  CarBack();break;
				case COMM_LEFT:  CarLeft();break;
				case COMM_RIGHT: CarRight();break;
				case COMM_STOP:  CarStop();break;
				default : break;
				}
				LCD1602WriteCommand(ctrl_comm);
			}
		} 
	}
}

/*******************************************************************************
* 函 数 名 ：Timer0Int
* 函数功能 ：定时器0中断函数 ， 每隔TIME_MS ms进入
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void Timer0Int() interrupt 1
{
	TH0=(65536-(FOSC/12*TIME_US)/1000000)/256;
	TL0=(65536-(FOSC/12*TIME_US)/1000000)%256;
	tick_1ms++;
	if(tick_1ms >= 5)
	{
		tick_1ms = 0;	
		tick_5ms++;
	}
	speed_count++;
	if(speed_count >= 50)
	{
		speed_count = 0;
	}
	CarMove();
}


