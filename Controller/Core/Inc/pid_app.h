#ifndef _PID_APP_H_
#define _PID_APP_H_


#include "main.h"

#define PID_VERSION_MODE 1
#define PID_LOADING_MODE 0


#define TIME_STOP_BEFORE_TURN             500
#define TIME_STOP_AFTER_TURN              600
#define TIME_STOP_AFTER_HALF_TURN         900

#define 	MOTOR_CIRCLE				3200

#define	MOTOR_MOVE_AFTER_CROSS 		      MOTOR_CIRCLE * 0.42
#define 	MOTOR_QUARTER_TURN			MOTOR_CIRCLE * 0.70
#define 	MOTOR_HALF_TURN				MOTOR_CIRCLE * 1.38
#define 	MOTOR_LITTLE_TURN			      MOTOR_CIRCLE * 0.2
#define 	MOTOR_MOVE_BACK				MOTOR_CIRCLE * 1.50



#define     MOTOR_TWO_MOVE                      MOTOR_MOVE_BACK + MOTOR_MOVE_AFTER_CROSS - MOTOR_CIRCLE * 0.20

#define ACC_START_VAL 210   //加速度初始值，防止小车磕头
#define ACC_MAX_VAL   255


/* 
  7               8
  |---------------|
  5       |       6
          |
    3-----------4
          |
          |
    1-----------2
          |
 */




//应用
void pid_application(void);


void pid_acc_smooth(void);
void pid_acc_reset(void);




void pid_app_init(void);
void pid_app_red_line(void);

void pid_app_quarter_turn(uint8_t dir);
void pid_app_half_turn(void);

void pid_uart_test(void);


#endif
