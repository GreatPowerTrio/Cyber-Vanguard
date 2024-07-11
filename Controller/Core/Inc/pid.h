#ifndef _PID_H_
#define _PID_H_


#include "main.h"

// pid使能状态枚举
typedef enum
{
    PID_ENABLE,
    PID_DISABLE,
} pid_status;


// pid参数结构体
typedef struct _pid_params_t
{
    float kp;
    float ki;
    float kd;

    float kp_out;
    float ki_out;
    float kd_out;

    // 目标值和测量值
    float target;
    float measure;

    // 误差、上一次的误差和误差的累积
    float error;
    float last_error;
    float sum_error;

    // 输出值
    float output;
    // 输出限幅
    float output_max;
    float output_min;

    // 积分限幅
    float integral_max;
    float integral_min;

    // 状态：使能或失能
    pid_status status;

    // 初始化函数
    void (*pid_params_init)   (struct _pid_params_t *pid, float _output_max, float _output_min, float _integral_max, float _integral_min);
    
    void (*pid_params_control)(struct _pid_params_t *pid, pid_status _status);
    float (*pid_params_compute)(struct _pid_params_t *pid, float _target, float _measure);

} pid_params_t;

// 链接结构体与函数
void pid_init(struct _pid_params_t *pid, float _kp, float _ki, float _kd);

//初始化输出和积分限幅，其他参数置零
void pid_params_init    (struct _pid_params_t *pid, float _output_max, float _output_min, float _integral_max, float _integral_min);

//使能控制
void pid_params_control (struct _pid_params_t *pid, pid_status _status);

//设置目标值和测量值，计算输出结果
float pid_params_compute (struct _pid_params_t *pid, float _target, float _measure);





#endif
