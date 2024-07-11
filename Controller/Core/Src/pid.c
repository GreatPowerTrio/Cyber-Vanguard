#include "pid.h"
#include "sensors.h"




/**
  * @brief 创建pid_param_t变量后，初始化需要执行的第一个函数，链接外部函数
  * @retval 无
  */
void pid_init(struct _pid_params_t *pid, float _kp, float _ki, float _kd)
{
    pid->pid_params_init    = pid_params_init;
    pid->pid_params_control = pid_params_control;
    pid->pid_params_compute = pid_params_compute;


    pid->kp = _kp;
    pid->ki = _ki;
    pid->kd = _kd;
}

/**
  * @brief 创建pid_param_t变量后，通过指针执行的初始化函数
  * @retval 无
  */
void pid_params_init    (struct _pid_params_t *pid, float _output_max, float _output_min, float _integral_max, float _integral_min)
{
    pid->output_max = _output_max;
    pid->output_min = _output_min;

    pid->integral_max = _integral_max;
    pid->integral_min = _integral_min;

    pid->error = 0;
    pid->last_error = 0;
    pid->sum_error = 0;
    pid->output = 0;

    pid->kp_out = 0;
    pid->ki_out = 0;
    pid->kd_out = 0;

    // 默认使能
    pid->status = PID_ENABLE;
}


/**
  * @brief 控制pid的使能与否
  * @retval 无
  */
void pid_params_control (struct _pid_params_t *pid, pid_status _status)
{
    pid->status = _status;
}

/**
  * @brief 设置目标值和测量值，计算输出结果
  * @retval 得到的输出值
  */
float pid_params_compute (struct _pid_params_t *pid, float _target, float _measure)
{
    pid->output = 0;

    // 使能状态下
    if(pid->status == PID_ENABLE)
    {
        pid->measure = _measure;
        pid->target = _target;

        // 误差 = 目标值 - 测量值
        pid->error = pid->target - pid->measure;


        // printf("err: %d \r\n", (int)(pid->error));

        /* 计算比例项 */
        pid->kp_out = pid->kp * pid->error;

        // printf("kp:%d kp_out: %d \r\n", (int)(pid->kp), (int)(pid->kp_out));


        // 计算累计误差
        pid->sum_error = pid->sum_error + pid->error;

        // 积分限幅
        if(pid->sum_error > pid->integral_max) pid->sum_error = pid->integral_max;
        if(pid->sum_error < pid->integral_min) pid->sum_error = pid->integral_min;

        /* 计算积分项 */
        pid->ki_out = pid->ki * pid->sum_error;


        /* 计算微分项 */
        pid->kd_out = pid->kd * (pid->error - pid->last_error);

        pid->last_error = pid->error;        


        /* 计算总输出 */
        pid->output = pid->kp_out + pid->ki_out + pid->kd_out;

        //输出限幅
        if(pid->output > pid->output_max) pid->output = pid->output_max;
        if(pid->output < pid->output_min) pid->output = pid->output_min;
    }
    return pid->output;
}


