#include "stm32f10x.h"
#include "hcsr04.h"
#include "delay.h"

/**
  * 函    数：超声波模块初始化
  * 说    明：Trig -> PB10, Echo -> PB11
  */
void HCSR04_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // Trig (PB10) - 输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // Echo (PB11) - 输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // 下拉输入，默认低电平
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_ResetBits(GPIOB, GPIO_Pin_10); // 默认拉低 Trig
}

/**
  * 函    数：获取原始距离 (单次测量)
  * 返 回 值：float 距离，单位 cm
  */
float HCSR04_GetRawValue(void)
{
    int time = 0;
    
    // 1. 发送触发信号
    GPIO_SetBits(GPIOB, GPIO_Pin_10); 
    DelayUs(20);
    GPIO_ResetBits(GPIOB, GPIO_Pin_10);

    // 2. 等待 Echo 变高
    int timeout = 0;
    while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == RESET)
    {
        timeout++;
        DelayUs(1);
        if(timeout > 20000) return 0; // 超时
    }
    
    // 3. 计时高电平
    while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == SET)
    {
        DelayUs(10); 
        time++;
        if(time > 5000) break; // 最大测量距离限制
    }
    
    // 4. 计算距离
    return (float)time * 0.17f; 
}

/**
  * 函    数：获取平滑后的距离 (均值滤波)
  * 说    明：连续测5次，去极值取平均
  * 返 回 值：float 距离，单位 cm
  */
float HCSR04_GetValue(void)
{
    float buf[5];
    float temp;
    int i, j;
    
    // 连续测量5次
    for(i = 0; i < 5; i++)
    {
        buf[i] = HCSR04_GetRawValue();
        DelayMs(10); 
    }
    
    // 冒泡排序
    for(i = 0; i < 5 - 1; i++)
    {
        for(j = 0; j < 5 - 1 - i; j++)
        {
            if(buf[j] > buf[j + 1])
            {
                temp = buf[j];
                buf[j] = buf[j + 1];
                buf[j + 1] = temp;
            }
        }
    }
    
    // 去掉最小和最大，取中间平均
    float sum = 0;
    int count = 0;
    for(i = 1; i < 4; i++) 
    {
        if(buf[i] > 0.1)
        {
            sum += buf[i];
            count++;
        }
    }
    
    if(count > 0) return sum / count;
    else return buf[2];
}
