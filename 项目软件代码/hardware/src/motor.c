#include "stm32f10x.h"
#include "motor.h"

MOTOR_INFO motor_info = {0};

/**
  * 函    数：电机初始化
  * 说    明：配置 PB6 为 PWM 输出，PB8, PB9 为方向控制
  */
void Motor_Init(void)
{
    /*开启时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //开启GPIOB时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  //开启TIM4时钟

    /*GPIO初始化*/
    GPIO_InitTypeDef GPIO_InitStructure;

    // 配置方向引脚 PB8, PB9 为推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 初始化时先设置为停止状态，防止上电乱跑
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    GPIO_ResetBits(GPIOB, GPIO_Pin_9);

    // 配置速度引脚 PB6 为复用推挽输出 (用于PWM)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*时基单元初始化*/
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 100 - 1;       // ARR 自动重装值 (0~100 代表速度百分比)
    TIM_TimeBaseStructure.TIM_Prescaler = 720 - 1;    // PSC 预分频器 (72MHz / 720 = 100kHz 计数频率，PWM频率=1kHz)
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /*输出比较初始化 (PWM模式)*/
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure); // 给结构体赋默认值
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;      // 初始占空比为0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure); // 初始化 TIM4 Channel 1 (PB6)

    /*开启定时器*/
    TIM_Cmd(TIM4, ENABLE);
    
    Motor_Set(MOTOR_OFF);
}

/*
************************************************************
*	函数名称：	Motor_Set
*
*	函数功能：	电机控制
*
*	入口参数：	status：开关电机
*
*	返回参数：	无
*
*	说明：		开-MOTOR_ON		关-MOTOR_OFF
************************************************************
*/
void Motor_Set(_Bool status)
{
    if (status == MOTOR_ON)
    {
        // 正转
        GPIO_SetBits(GPIOB, GPIO_Pin_8);
        GPIO_ResetBits(GPIOB, GPIO_Pin_9);
        TIM_SetCompare1(TIM4, 100); // 100% 速度
    }
    else
    {
        // 停止
        GPIO_ResetBits(GPIOB, GPIO_Pin_8);
        GPIO_ResetBits(GPIOB, GPIO_Pin_9);
        TIM_SetCompare1(TIM4, 0);   // 0% 速度
    }

    motor_info.Motor_Status = status;
}
