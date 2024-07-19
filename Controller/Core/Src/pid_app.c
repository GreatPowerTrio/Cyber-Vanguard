#include "pid_app.h"
#include "pid.h"
#include "sensors.h"
#include "usart.h"
#include "comm.h"


// 可用参数1：output_max = 1000, kp = 40, speed = 50, acc = 200, 其余为0, 权重为0135
// 实际效果： 能巡圆圈和直线，但是速度太慢


// 可用参数2：output_max = 1000, kp = 40, speed = 150, acc = 0(直接启动)， 权重为121，其余为0
// 实际效果：有较好的巡线效果，速度足够，但是有磕头的现象

// 可用参数3：output_max = 800, kp = 40, ki = 1.5, kd = -0.4, integral = |20|, speed = 150, acc = 0(直接启动)， 权重为121，其余为0
// 实际效果：有较好的巡线效果，速度足够，磕头问题在车头加点重物就能解决

// 可用参数4：output_max = 800, kp = 48, ki = 0.8, kd = -0.5, integral = |20|, speed = 110, acc_start = 210， 权重为121，其余为0.5
// 速度降低

// 最终参数：output_max = 400，kp = 35，kd = -30，speed = 120，turn_speed = 180，权重120
/* 
#define TIME_STOP_BEFORE_TURN             500
#define TIME_STOP_AFTER_TURN              600
#define TIME_STOP_AFTER_HALF_TURN         900

#define 	MOTOR_CIRCLE				3200

#define	    MOTOR_MOVE_AFTER_CROSS 		MOTOR_CIRCLE * 0.42
#define 	MOTOR_QUARTER_TURN			MOTOR_CIRCLE * 0.70
#define 	MOTOR_HALF_TURN				MOTOR_CIRCLE * 1.38
#define 	MOTOR_LITTLE_TURN			MOTOR_CIRCLE * 0.2
#define 	MOTOR_MOVE_BACK				MOTOR_CIRCLE * 1.10

#define ACC_START_VAL 210   //加速度初始值，防止小车磕头
#define ACC_MAX_VAL   255 
*/
// 实际效果：加上树莓派后巡直线

pid_params_t motorL_pid;
pid_params_t motorR_pid;

// 初始化过程使用的变量
float output_max = 400;
float output_min = 0;
float integral_max =  0;
float integral_min =  0;

// PID参数，调参需要改的地方
float kp = 35;
float ki = 0;
float kd = -30;

// 目标值、测量值和计算的左右电机的最终速度值
float target = 0;
float mearsure;
float resultL;
float resultR;

// 基准速度值
float speed = 120;
float turn_speed = 180;

// 电机速度模式参数，加速度递增
uint16_t acc = ACC_START_VAL; 

#define TIME 100

/**
  * @brief pid整体控制函数，20ms调用一次
  * @retval 无
  */
void pid_application(void)
{
    typedef enum 
    {
        _INIT,                          /* PID初始化 */
        _WAITING_NUMBER,                /* 等待树莓派传来的初始数字信息 */
        _WAITING_LOADING,               /* 等待人工装载药物 */
        _FIRST_JUDGE,                   /* 第一次判断：近端或远端病房 */
        /* 分支 */
        _GO_TO_ROOM1_OR_ROOM2,          /* 去一号病房或二号病房 */
        _GO_TO_2ND_CROSSING,            /* 去到第二个十字路口 */
        /* 分支 */
        _SECOND_JUDGE,                  /* 第二次判断：中部或远部病房 */
        _GO_TO_MID_ROOMS,               /* 去中部病房 */
        _GO_TO_3RD_CROSSING,            /* 小车的目标是远端病房，先走到第三个路口前，后退一段距离 */
        
        _THIRD_JUDGE,                   /* 第三次判读：识别4个数字 */
        _GO_TO_FAR_ROOMS,               /* 去远端病房 */
    } app_status;


    static app_status status = _INIT;

    /* 各子状体参数 */
    static uint8_t go_to_near_room_step;
    static uint8_t go_to_near_room_dir;

    static uint8_t go_to_2nd_crossing_step;
    static uint8_t go_to_2nd_crossing_cnt;
    static uint16_t go_to_2nd_crossing_tick;

    static uint8_t go_to_mid_room_step;
    static uint8_t go_to_mid_room_dir;

    static uint16_t go_to_3rd_crossing_tick;
    static uint8_t go_to_3rd_crossing_step;

    static uint8_t third_judge_step;


    static uint8_t go_to_far_room_step;
    /* 此处调试时指定远端病房位置 */
    static uint8_t go_to_far_room_dir1 = 0;
    static uint8_t go_to_far_room_dir2 = 0;
    static uint8_t go_to_far_room_numbers[5];

    // 灰度状态
    uint8_t sensor_state;

    /* 1 2 3 4，近端和中端病房 */
    static char dest_number = '5';
    static char detect_number;
    static char detect_dir;    


    switch (status)
    {
    case _INIT: /* PID初始化 */

        pid_app_init();
        status = _WAITING_NUMBER;
        break;
    
    case _WAITING_NUMBER: /* 等待数字识别 */

#if PID_VERSION_MODE
        if(comm_waiting_number() == true)
        {
            /* 获取识别到的初始数字信息 */
            dest_number = comm_message.left_number;
            status = _WAITING_LOADING;

        }
#else
        // dest_number = '5';
        status = _WAITING_LOADING;
#endif
        break;

    case _WAITING_LOADING: /* 等待药物装载 */

#if PID_LOADING_MODE
        if(comm_waiting_load() == true)
            status = _FIRST_JUDGE;
#else
        status = _FIRST_JUDGE;
#endif        
        break;

    case _FIRST_JUDGE: /* 第一次判断：近端或远端病房 */

        if(dest_number == '1' || dest_number == '2')
        {   
            /* 为0去一号病房，为1去二号病房 */
            go_to_near_room_dir = dest_number - '1';
            status = _GO_TO_ROOM1_OR_ROOM2;
        }
        else
        {
            status = _GO_TO_2ND_CROSSING;
            go_to_2nd_crossing_tick = HAL_GetTick();
        }
        break;

    case _GO_TO_ROOM1_OR_ROOM2: /* 去近端病房的逻辑 */

        // 获取传感器信息
        sensor_state = Sensor_Get_State();
        mearsure     = Sensor_Get_Weight();

        switch (go_to_near_room_step)
        {
        case 0: /* 巡红线环直走 */
            if(Sensor_Cross_Judge(sensor_state))
                go_to_near_room_step++;

            // 加速度平滑处理，防止磕头
            pid_acc_smooth();
            // 巡线环运作
            pid_app_red_line();
                
            break;
        
        case 1: /* 移动一小段距离后停下，并转弯 */
            // 加速度恢复初值
            pid_acc_reset();

            // 向前移动一段距离后停下
            Emm_V5_Pos_Control(0, 0, speed, 0, MOTOR_MOVE_AFTER_CROSS, false, false, &huart2);

            // 转向
            pid_app_quarter_turn(go_to_near_room_dir);

            go_to_near_room_step++;
            break;
        case 2: /* 巡红线环直走到终点 */

            if(Sensor_Ending_Judge(sensor_state))
                go_to_near_room_step++;

            // 加速度平滑处理，防止磕头
            pid_acc_smooth();

            pid_app_red_line();
            break;

        case 3: /* 停下 */

            pid_acc_reset();
            Emm_V5_Vel_Control(0, 0, 0, 240, false, &huart2);
            
            go_to_near_room_step++;

            break;
        
        case 4: /* 等到药物卸载 */

#if PID_LOADING_MODE
            if(comm_waiting_load() == true)
            {
                go_to_near_room_step++;
            }
#else 
            go_to_near_room_step++;
#endif
            break;

        case 5: /* 180°转弯 */

            pid_app_half_turn();

            go_to_near_room_step++;
            break;

        case 6: /* 返回寻红线直走 */

            if(Sensor_Cross_Judge(sensor_state))
                go_to_near_room_step++;

            // 加速度平滑处理，防止磕头
            pid_acc_smooth();

            pid_app_red_line();

            break;

        case 7: /* 遇到路口，移动一小段距离后停，转弯 */
            // 加速度恢复初值
            pid_acc_reset();

            Emm_V5_Pos_Control(0, 0, speed, 0, MOTOR_MOVE_AFTER_CROSS, false, false, &huart2);

            pid_app_quarter_turn(!go_to_near_room_dir);

            go_to_near_room_step++;

            break;
        
        case 8: /* 直走直到终点 */

            if(Sensor_Ending_Judge(sensor_state))
                go_to_near_room_step++;

            // 加速度平滑处理，防止磕头
            pid_acc_smooth();

            pid_app_red_line();

            break;

        case 9: /* 停 */

            /* 停 */
            pid_acc_reset();

            Emm_V5_Vel_Control(0, 0, 0, 0, false, &huart2);
            HAL_Delay(10);
        
            go_to_near_room_step++;

            break;
        case 10:

            break;

        default:
            break;
        }
        break;

    case _GO_TO_2ND_CROSSING: /* 走到第二个路口，并后退 */

        // 获取传感器信息
        sensor_state = Sensor_Get_State();
        mearsure     = Sensor_Get_Weight();

        switch (go_to_2nd_crossing_step)
        {
        case 0: /* 检测到第二个路口 */
            // 等待让小车走到第二个路口的时间
            if(Sensor_Cross_Judge(sensor_state) && HAL_GetTick() - go_to_2nd_crossing_tick >= 2500) 
                go_to_2nd_crossing_step++;

            // 加速度平滑处理，防止磕头
            pid_acc_smooth();

            pid_app_red_line();
             
            break;

        case 1: /* 停下后后退一段距离 */
            // 加速度恢复初值
            pid_acc_reset();

            Emm_V5_Pos_Control(0, 1, speed, 200, MOTOR_MOVE_BACK, false, false, &huart2);

            go_to_2nd_crossing_step++;
        
            break;
        
        case 2: /* 等待摄像头识别的数字信息 */

#if PID_VERSION_MODE
            if(comm_waiting_number() == true)
            {
                go_to_2nd_crossing_step++;
                status = _SECOND_JUDGE;

            }

#else
            HAL_Delay(1000);
            go_to_2nd_crossing_step++;
            status = _SECOND_JUDGE;
#endif
            break;

        default:
            break;
        }
        break;

    case _SECOND_JUDGE: /* 判断中部病房和远端病房 */

#if PID_VERSION_MODE  


        if(dest_number == comm_message.left_number)
        {
            // 与中部左病房匹配
            go_to_mid_room_dir = 0;
            status = _GO_TO_MID_ROOMS;      
        }
        else if(dest_number == comm_message.right_number)
        {
            // 与中部右病房匹配
            go_to_mid_room_dir = 1;
            status = _GO_TO_MID_ROOMS;
        }
        else
        {
            go_to_3rd_crossing_tick = HAL_GetTick();
            status = _GO_TO_3RD_CROSSING;
        }
#else

        if(dest_number == '3')
        {
            go_to_mid_room_dir = 0;
            status = _GO_TO_MID_ROOMS;
        }
        else if(dest_number == '4')
        {
            go_to_mid_room_dir = 1;
            status = _GO_TO_MID_ROOMS;
        }
        else
        {
            go_to_3rd_crossing_tick = HAL_GetTick();
            status = _GO_TO_3RD_CROSSING;           
        }
#endif
        break;

    case _GO_TO_MID_ROOMS: /* 去中部病房逻辑 */

        // 获取传感器信息
        sensor_state = Sensor_Get_State();
        mearsure     = Sensor_Get_Weight();

        switch (go_to_mid_room_step)
        {
        case 0: /* 向前走两部分距离并转弯 */

            Emm_V5_Pos_Control(0, 0, 200, 200, MOTOR_TWO_MOVE, false, false, &huart2);
            HAL_Delay(500);

            // 转向
            pid_app_quarter_turn(go_to_mid_room_dir);

            go_to_mid_room_step++;
            break;
            
        case 1: /* 开启巡红线环走到终点 */
        
            if(Sensor_Ending_Judge(sensor_state))
                go_to_mid_room_step++;

            // 加速度平滑处理，防止磕头
            pid_acc_smooth();

            pid_app_red_line();

            break;
        
        case 2: /* 停下 */

            pid_acc_reset();
            Emm_V5_Vel_Control(0, 0, 0, 0, false, &huart2);
            go_to_mid_room_step++;
            break;

        case 3: /* 等待药物卸载 */
#if PID_LOADING_MODE
            if(comm_waiting_load())
            {
                go_to_mid_room_step++;
            }
#else       
            HAL_Delay(500);
            go_to_mid_room_step++;
#endif
            break;

        case 4: /* 180°转弯 */

            pid_app_half_turn();
            go_to_mid_room_step++;
            break;            

        case 5: /* 返回寻红线直走 */

            if(Sensor_Cross_Judge(sensor_state))
                go_to_mid_room_step++;

            // 加速度平滑处理，防止磕头
            pid_acc_smooth();

            pid_app_red_line();

            break;

        case 6: /* 遇到路口，移动一小段距离停下，转弯 */

            pid_acc_reset();

            Emm_V5_Pos_Control(0, 0, speed, 0, MOTOR_MOVE_AFTER_CROSS, false, false, &huart2);
            // 转向
            pid_app_quarter_turn(!go_to_mid_room_dir);

            go_to_mid_room_step++;

            break;

        case 7: /* 直走直到终点 */

            if(Sensor_Ending_Judge(sensor_state))
                go_to_mid_room_step++;

            pid_acc_smooth();

            pid_app_red_line();

            break;

        case 8: /* 停 */

            pid_acc_reset();
            Emm_V5_Vel_Control(0, 0, 0, 0, false, &huart2);
            go_to_mid_room_step++;
            break;       

        default:
            break;
        }

        break;

    case _GO_TO_3RD_CROSSING: /* 移动到第三个路口逻辑 */

        // 获取传感器信息
        sensor_state = Sensor_Get_State();
        mearsure     = Sensor_Get_Weight();

        switch (go_to_3rd_crossing_step)
        {
        case 0: /* 开启巡线环直走到路口 */

            if(Sensor_Cross_Judge(sensor_state) && HAL_GetTick() - go_to_3rd_crossing_tick >= 2000) 
                go_to_3rd_crossing_step++;

            pid_acc_smooth();

            pid_app_red_line();

            break;
        case 1: /* 后退一段距离 */
        

            Emm_V5_Pos_Control(0, 1, speed, 200, MOTOR_MOVE_BACK, false, false, &huart2);
#if PID_VERSION_MODE == 0            
            HAL_Delay(1000);            
#endif
            go_to_3rd_crossing_step++;
            status = _THIRD_JUDGE;
            break;

        default:
            break;
        }

        break;

    case _THIRD_JUDGE: /* 第三次判读：识别4个数字 */
        
        switch (third_judge_step)
        {
        case 0: /* 向左旋转一定角度 */

            Emm_V5_Pos_Control(1, 0, 200, 200, MOTOR_LITTLE_TURN, false, false, &huart2);
            HAL_Delay(10);
            Emm_V5_Pos_Control(2, 1, 200, 200, MOTOR_LITTLE_TURN, false, false, &huart2);

            third_judge_step++;
            break;
        
        case 1: /* 等待数字识别 */

#if PID_VERSION_MODE
            if(comm_waiting_number() == true)
            {
                go_to_far_room_numbers[1] = comm_message.left_number;
                go_to_far_room_numbers[2] = comm_message.right_number;
                third_judge_step++;

            }
#else
            HAL_Delay(1000);
            third_judge_step++;
#endif
            break;
        
        case 2: /* 向左旋转一定角度 */

            Emm_V5_Pos_Control(1, 1, 200, 200, MOTOR_LITTLE_TURN * 2, false, false, &huart2);
            HAL_Delay(10);
            Emm_V5_Pos_Control(2, 0, 200, 200, MOTOR_LITTLE_TURN * 2, false, false, &huart2);
            third_judge_step++;
            break;

        case 3: /* 等待数字识别 */

#if PID_VERSION_MODE
            if(comm_waiting_number() == true)
            { 
                go_to_far_room_numbers[3] = comm_message.left_number;
                go_to_far_room_numbers[4] = comm_message.right_number;

                if(dest_number ==  go_to_far_room_numbers[1] || dest_number == go_to_far_room_numbers[2])
                {
                    go_to_far_room_dir1 = 0;
                }
                else if(dest_number ==  go_to_far_room_numbers[3] || dest_number == go_to_far_room_numbers[4])
                {
                    go_to_far_room_dir1 = 1;
                }
                third_judge_step++; 
              
            }
#else
            HAL_Delay(1000);
            third_judge_step++;
            /* 此处得到第一次转弯的方向 */
            // go_to_far_room_dir1 = 0;
#endif
            break;
        
        case 4: /* 回到中线 */

            Emm_V5_Pos_Control(1, 0, 200, 200, MOTOR_LITTLE_TURN, false, false, &huart2);
            HAL_Delay(10);
            Emm_V5_Pos_Control(2, 1, 200, 200, MOTOR_LITTLE_TURN, false, false, &huart2);
            HAL_Delay(400);

            third_judge_step++;
            status = _GO_TO_FAR_ROOMS;
            break;

        default:
            break;
        }

        break;

    case _GO_TO_FAR_ROOMS: /* 去远端病房 */
        // 获取传感器信息
        sensor_state = Sensor_Get_State();
        mearsure     = Sensor_Get_Weight();
        switch (go_to_far_room_step)
        {
        case 0: /* 向前移动两段距离并转弯 */

            pid_acc_reset();
            
            Emm_V5_Pos_Control(0, 0, 200, 200, MOTOR_TWO_MOVE, false, false, &huart2);
            HAL_Delay(500);
            // 转向
            pid_app_quarter_turn(go_to_far_room_dir1);
            
            go_to_far_room_step++;
            break;
        
        case 1: /* 开启巡线环，直到路口 */

            if(Sensor_Cross_Judge(sensor_state)) 
                go_to_far_room_step++;

            // 加速度平滑处理
            pid_acc_smooth();

            pid_app_red_line();
            break;
        
        case 2: /* 停，后退一段距离 */

            // 加速度恢复初值
            pid_acc_reset();

            Emm_V5_Pos_Control(0, 1, speed, 200, MOTOR_MOVE_BACK, false, false, &huart2);

#if PID_VERSION_MODE == 0
            HAL_Delay(1000);
#endif
            go_to_far_room_step++;
            break;

        case 3: /* 等待数字识别 */
#if PID_VERSION_MODE
            if(comm_waiting_number() == true)
            {
                if(dest_number == comm_message.left_number)
                {
                    go_to_far_room_dir2 = 0;
                }
                else if(dest_number == comm_message.right_number)
                {
                    go_to_far_room_dir2 = 1;
                }
                go_to_far_room_step++;
            }
#else
            // 得到第二个方向信息
            // go_to_far_room_dir2 = 1;
            go_to_far_room_step++;
#endif
            break;

        case 4: /* 向前移动距离并转弯 */

            Emm_V5_Pos_Control(0, 0, 200, 200, MOTOR_TWO_MOVE, false, false, &huart2);
            HAL_Delay(500);

            // 转向
            pid_app_quarter_turn(go_to_far_room_dir2);

            go_to_far_room_step++;

            break;

        case 5: /* 开启巡线环走到终点 */

            if(Sensor_Ending_Judge(sensor_state)) 
                go_to_far_room_step++;

            // 加速度平滑处理
            pid_acc_smooth();

            pid_app_red_line();

            break;

        case 6: /* 停下 */

            pid_acc_reset();
            Emm_V5_Vel_Control(0, 0, 0, 0, false, &huart2);
            go_to_far_room_step++;
            break;

        case 7: /* 等待药物卸载 */
#if PID_LOADING_MODE
            if(comm_waiting_load() == true)
            {
                go_to_far_room_step++;
            }
#else
            HAL_Delay(500);
            go_to_far_room_step++;
#endif
            break;

        case 8: /* 转弯180° */

            pid_app_half_turn();   
            go_to_far_room_step++;
            break;

        case 9: /* 巡线到路口 */

            if(Sensor_Cross_Judge(sensor_state)) 
                go_to_far_room_step++;
            // 加速度平滑处理
            pid_acc_smooth();

            pid_app_red_line();

            break;

        case 10: /* 前进一段距离停下，转弯 */
            pid_acc_reset();

            Emm_V5_Pos_Control(0, 0, speed, 0, MOTOR_MOVE_AFTER_CROSS, false, false, &huart2);
            pid_app_quarter_turn(!go_to_far_room_dir2);

            go_to_far_room_step++;
            break;

        case 11: /* 巡线到路口 */

            if(Sensor_Cross_Judge(sensor_state)) 
                go_to_far_room_step++;

            // 加速度平滑处理
            pid_acc_smooth();

            pid_app_red_line();

            break;
        case 12: /* 停下并转弯 */

            pid_acc_reset();

            Emm_V5_Pos_Control(0, 0, speed, 0, MOTOR_MOVE_AFTER_CROSS, false, false, &huart2);
            pid_app_quarter_turn(!go_to_far_room_dir1);

            go_to_far_room_step++;

            break;

        case 13: /* 巡线到终点 */
            if(Sensor_Ending_Judge(sensor_state))
                go_to_far_room_step++;

            // 加速度平滑处理，防止磕头
            pid_acc_smooth();

            pid_app_red_line();

            break;
        case 14: /* 停 */
            pid_acc_reset();

            Emm_V5_Vel_Control(0, 0, 0, 0, false, &huart2);

            go_to_far_room_step++;
            break;

        default:
            break;
        }

        break;

    default:
        break;
    }
    
}

/**
  * @brief 加速度平滑处理，防止小车磕头，在巡红线环处调用
  */
void pid_acc_smooth(void)
{
    if(++acc > ACC_MAX_VAL) acc = ACC_MAX_VAL;
}

/**
  * @brief 加速度重置，在小车停止处调用
  */
void pid_acc_reset(void)
{
    acc = ACC_START_VAL;
}

/**
  * @brief 开启转向环
  */
void pid_app_red_line(void)
{
    // 左电机
    resultL = motorL_pid.pid_params_compute(&motorL_pid, target, mearsure);
    Emm_V5_Vel_Control(1, 0, speed + resultL, acc, false, &huart2);
    
    HAL_Delay(10);
    // 右电机
    resultR = motorR_pid.pid_params_compute(&motorR_pid, target, mearsure);
    Emm_V5_Vel_Control(2, 0, speed + resultR, acc, false, &huart2);  
}

/**
  * @brief pid参数初始化
  */
void pid_app_init(void)
{
    // 电机默认设置参考Motor.c
    // 初始化左右电机，右边电机pid参数是相反的
    pid_init(&motorL_pid, kp, ki, kd);
    motorL_pid.pid_params_init(&motorL_pid, output_max, output_min, integral_max, integral_min);

    pid_init(&motorR_pid, -kp, -ki, -kd);
    motorR_pid.pid_params_init(&motorR_pid, output_max, output_min, integral_max, integral_min);
}


/**
  * @brief 直角转弯
  */
void pid_app_quarter_turn(uint8_t dir)
{
    HAL_Delay(TIME_STOP_BEFORE_TURN);
    Emm_V5_Pos_Control(1, dir, turn_speed, 0, MOTOR_QUARTER_TURN, false, false, &huart2);
    HAL_Delay(10);
    Emm_V5_Pos_Control(2, !dir, turn_speed, 0, MOTOR_QUARTER_TURN, false, false, &huart2);
    HAL_Delay(TIME_STOP_AFTER_TURN);
}

/**
  * @brief 掉头
  */
void pid_app_half_turn(void)
{
    Emm_V5_Pos_Control(1, 0, turn_speed, 0, MOTOR_HALF_TURN, false, false, &huart2);
    HAL_Delay(10);
    Emm_V5_Pos_Control(2, 1, turn_speed, 0, MOTOR_HALF_TURN, false, false, &huart2);
    HAL_Delay(TIME_STOP_AFTER_HALF_TURN);
}


/**
  * @brief 串口调试程序
  */
void pid_uart_test(void)
{
    typedef enum 
    {
        _INIT,
        _WAITING_FIRST_NUMBER,
        _DEAL1,
        _WAITING_SECOND_NUMBER,
        _DEAL2,

        _END,
    } status_t;


    static status_t status = _INIT;

    switch (status)
    {
    case _INIT:
        status = _WAITING_FIRST_NUMBER;
        break;
    
    case _WAITING_FIRST_NUMBER:

        if(comm_waiting_number())
        {
            status = _DEAL1;
        }

        break;
    
    case _DEAL1:

        status = _WAITING_SECOND_NUMBER;


        usart_SendByte(comm_message.left_number, &huart3);
        usart_SendByte(comm_message.right_number,&huart3);
        usart_SendByte('\r', &huart3);
        usart_SendByte('\n', &huart3);

        HAL_Delay(20);

        HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
        break;
    
    case _WAITING_SECOND_NUMBER:

        if(comm_waiting_number())
        {
            status = _DEAL2;
        }

        break;

    case _DEAL2:


        usart_SendByte(comm_message.left_number, &huart3);
        usart_SendByte(comm_message.right_number,&huart3);
        usart_SendByte('\r', &huart3);
        usart_SendByte('\n', &huart3);
        
        HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
        status = _INIT;

        break;
    default:
        break;
    }

}