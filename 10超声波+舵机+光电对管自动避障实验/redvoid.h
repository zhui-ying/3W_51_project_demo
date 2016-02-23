#ifndef __REDVOID_H__
#define __REDVOID_H__

#define VOID_LEFT 1//左边检测到
#define VOID_RIGHT 2//右边检测到
#define VOID_BOTH 3 //两边都检测到
#define VOID_NONE 0//都没检测到

//void VoidRun(void);
char GetVoidStatus(void);
void DelayCheck(int ms);

#endif
