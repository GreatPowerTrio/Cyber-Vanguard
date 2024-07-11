#include "timer.h"

// 定时器表
soft_timer timers[SOFT_TIMER_MAX_SIZE];

/**
  * @brief 软件定时器初始化
  * @param timer 指定定时器名称，可在枚举中查看和添加
  * @param timeout 定时时间(ms)
  * @retval 无
  */
void soft_timer_init(soft_timer_type timer, uint16_t timeout)
{
    timers[timer].ready = false;
    timers[timer].tick  = 0;
    timers[timer].timeout = timeout;

}
/**
  * @brief 软件定时器运行，在tim2 1ms定时器中断中执行
  */
void soft_timer_run(void)
{
    for(soft_timer_type timer = 0; timer < SOFT_TIMER_MAX_SIZE; timer++)
    {
        if(++timers[timer].tick > timers[timer].timeout && timers[timer].ready == false)
        {
            timers[timer].tick = 0;
            timers[timer].ready = true;
        }

    }
}

/**
  * @brief 软件定时器使用，在main函数中使用
  * @param timer 指定定时器名称，可在枚举中查看和添加
  * @retval 返回是否定时完成标志
  */
bool soft_timer_is_timeout(soft_timer_type timer)
{
    bool res = timers[timer].ready;
    if(res) 
        timers[timer].ready = false;
    return res;
}


