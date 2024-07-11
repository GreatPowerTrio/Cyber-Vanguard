#ifndef _PID_APP_H_
#define _PID_APP_H_




// pid状态枚举，用于初始化部分只执行一次
typedef enum 
{
    APP_INIT,
    APP_ONGOING,
} app_status;


//循迹转向环应用
void pid_application_turn(void);















#endif
