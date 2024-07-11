#ifndef _TIMER_H_
#define _TIMER_H_

#include "main.h"


typedef struct
{
    // 是否计时完成，当前时刻，指定的计时时间
    bool ready;
    uint16_t tick;
    uint16_t timeout;

} soft_timer;

// 定时器列表枚举
typedef enum 
{   
    SOFT_TIMER0,
    SOFT_TIMER1,
    SOFT_TIMER_MAX_SIZE,
} soft_timer_type;


void soft_timer_init(soft_timer_type timer, uint16_t timeout);
void soft_timer_run(void);
bool soft_timer_is_timeout(soft_timer_type timer);


#endif
