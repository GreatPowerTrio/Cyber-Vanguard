#include "pid_app.h"
#include "pid.h"
#include "sensors.h"


pid_params_t motorL_pid;
pid_params_t motorR_pid;

// 初始化过程使用的变量
float output_max = 100;
float output_min = 0;
float integral_max = 100;
float integral_min = 0;

// PID参数，调参需要改的地方
float kp = 10000;
float ki = 0;
float kd = 0;

// 目标值、测量值和计算的速度值
float target = 0;
float mearsure = 0;
float resultL = 0;
float resultR = 0;

// 基准速度值
float speed = 40;


// 电机速度模式参数
uint16_t acc = 10; 


uint8_t state;

/**
  * @brief pid整体控制函数，这里是巡线环，20ms调用一次
  * @retval 无
  */
void pid_application_turn(void)
{
    static app_status status = APP_INIT;


    switch (status)
    {
    case APP_INIT:
        /* pid初始化代码 */

        // 电机驱动按键设置部分：

        // 需要先设置两个电机同向

        // 初始化左电机


        pid_init(&motorL_pid, kp, ki, kd);
        motorL_pid.pid_params_init(&motorL_pid, output_max, output_min, integral_max, integral_min);
        // motorL_pid.pid_params_set_val(&motorL_pid, kp, ki, kd);

        // 初始化右电机
        pid_init(&motorR_pid, -kp, -ki, -kd);
        motorR_pid.pid_params_init(&motorR_pid, output_max, output_min, integral_max, integral_min);
        // 注意这里参数是反的
        // motorR_pid.pid_params_set_val(&motorR_pid, -kp, -ki, -kd);

        printf("%d %d\r\n", (int)motorL_pid.output_max, (int)motorL_pid.kp);

        status = APP_ONGOING;
        break;
    
    case APP_ONGOING:


        /* pid运行代码 */

        state = Sensor_Get_State();
        mearsure = Sensor_Get_Weight();

        if(state == 0xFF)
        {
            Emm_V5_Vel_Control(0, 0, 0, 0, false);
        }
        else
        {
            // 左电机
            resultL = motorL_pid.pid_params_compute(&motorL_pid, target, mearsure);
            Emm_V5_Vel_Control(1, 0, speed + resultL, acc, false);
            

            HAL_Delay(10);
            // 右电机
            resultR = motorR_pid.pid_params_compute(&motorR_pid, target, mearsure);
            Emm_V5_Vel_Control(2, 0, speed + resultR, acc, false);
            
            // printf("Measure:%d L:%d R:%d\r\n", (int)mearsure, (int)resultL, (int)resultR);

        }
        


        break;
    default:
        break;
    }
    
}