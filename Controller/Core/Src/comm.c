#include "comm.h"
#include "usart.h"
#include "pid_app.h"
#include "usart.h"


char comm_string[LENGTH];



bool comm_number_flag;


bool comm_load_flag;

number_message comm_message;



void comm_get_date(uint8_t data)
{

  static uint8_t comm_pointer = 0;

  comm_string[comm_pointer++] = data;

  if(comm_string[0] != '(') comm_pointer = 0;

  if(comm_pointer == 6)
  {
    if(comm_string[0] == '(' && comm_string[3] == ')' && comm_string[5] == '\n')
    {
      comm_message.left_number = comm_string[1];
      comm_message.right_number= comm_string[2];

      comm_number_flag = true;
    }
    comm_pointer = 0;
  }

}


/**
  * @brief 等待数字识别，标志位在串口中断中使用
  */
bool comm_waiting_number(void)
{
  typedef enum
  {
    _INIT,
    _DETECT,
    _END,
  } state_t;

  static state_t status = _INIT;
  static int16_t tick;

  switch (status)
  {
  case _INIT:

    usart_SendByte('O', &huart3);
    tick = HAL_GetTick();
    status = _DETECT;
    break;
  
  case _DETECT:

    HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
    if(HAL_GetTick() - tick >= 2000)
    {
      if(comm_number_flag)
      {
        status = _END;
        comm_number_flag = false;
        return true;
      }
    }

    break;
  
  case _END:
      status = _INIT;

    break;


  default:
    break;
  }


  return false;
}

/**
  * @brief 等待药物转载或卸载，标志位在按键中断中使用
  */
bool comm_waiting_load(void)
{
    if(comm_load_flag == true)
    {
        comm_load_flag = false;
        return true;
    }
    return false;
}


