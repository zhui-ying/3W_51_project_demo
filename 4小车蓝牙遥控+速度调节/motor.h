#ifndef __MOTOR_H_
#define __MOTOR_H_

extern unsigned int speed_count;//占空比计数器 50次一周期
extern char front_left_speed_duty;
extern char front_right_speed_duty;
extern char behind_left_speed_duty;
extern char behind_right_speed_duty;

extern char speed_duty_l;//设置左右两轮的速度
extern char speed_duty_r;

void CarMove();
void CarGo();
void CarBack();
void CarLeft();
void CarRight();
void CarStop();

#endif

