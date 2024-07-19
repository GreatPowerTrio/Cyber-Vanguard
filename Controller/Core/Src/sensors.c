#include "sensors.h"



uint8_t sensorStates[8];




/**
  * @brief 获取灰度传感器检测到的高低电平状态
  * @retval 返回一个8位无符号值，每个位代表灰度传感器的电平状态
  */
uint8_t Sensor_Get_State(void)
{
    uint8_t sensorVal = 0x00;

    // 取反，检测到黑色为1
    sensorStates[0] = !HAL_GPIO_ReadPin(SENSOR0_GPIO_Port, SENSOR0_Pin);
    sensorStates[1] = !HAL_GPIO_ReadPin(SENSOR1_GPIO_Port, SENSOR1_Pin);
    sensorStates[2] = !HAL_GPIO_ReadPin(SENSOR2_GPIO_Port, SENSOR2_Pin);
    sensorStates[3] = !HAL_GPIO_ReadPin(SENSOR3_GPIO_Port, SENSOR3_Pin);
    sensorStates[4] = !HAL_GPIO_ReadPin(SENSOR4_GPIO_Port, SENSOR4_Pin);
    sensorStates[5] = !HAL_GPIO_ReadPin(SENSOR5_GPIO_Port, SENSOR5_Pin);
    sensorStates[6] = !HAL_GPIO_ReadPin(SENSOR6_GPIO_Port, SENSOR6_Pin);
    sensorStates[7] = !HAL_GPIO_ReadPin(SENSOR7_GPIO_Port, SENSOR7_Pin);

    // 需要根据灰度传感器输出的高低电平修改，此处为：检测到轨迹为高电平
    for(uint8_t i = 0; i < 8; i++)
    {
        // 从高位到低位，传感器编号从左往右，从低到高
        sensorVal |= sensorStates[i] << i;
    }

    return sensorVal;
}

/**
  * @brief 计算灰度状态的权值，最中间为0，往左边递减，往右边递增，用于pid循迹
  * @retval 返回一个8位无符号值，即计算得到的权值
  */
int32_t Sensor_Get_Weight(void)
{
    int32_t weight = 0;

    // weight += sensorStates[3] *  0;
    // weight += sensorStates[4] *  0;
    weight += sensorStates[2] * -1;
    weight += sensorStates[5] *  1;
    weight += sensorStates[1] * -2;
    weight += sensorStates[6] *  2;
    weight += sensorStates[0] *  0;
    weight += sensorStates[7] *  0;

    return weight;
}

/**
  * @brief 判断是否为十字路口或T字路口
  */
bool Sensor_Cross_Judge(uint8_t state)
{
  return state == 0xFF || state == 0xF8 || state == 0xF0 || state == 0x0F || state == 0x1F;
}

/**
  * @brief 判断是否为终点
  */
bool Sensor_Ending_Judge(uint8_t state)
{
  return state == 0x00;
}
