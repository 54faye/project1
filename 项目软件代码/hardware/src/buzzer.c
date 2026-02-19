#include "buzzer.h"
#include "stm32f10x.h"                  // Device header
BUZZER_INFO buzzer_info = {0};
/**
  * 函    数：蜂鸣器初始化
  * 说    明：使用 PB5 引脚
  */
void Buzzer_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    buzzer_Set(BUZZER_OFF); // 初始化主要设为高电平(1)，确保低电平触发的蜂鸣器上电不响
}

/*
************************************************************
*	函数名称：	buzzer_Set
*
*	函数功能：	蜂鸣器控制
*
*	入口参数：	status：开关蜂鸣器
*
*	返回参数：	无
*
*	说明：		开-buzzer_ON		关-buzzer_OFF
************************************************************
*/
void buzzer_Set(_Bool status)
{
	
	GPIO_WriteBit(GPIOB, GPIO_Pin_5, status == BUZZER_ON ? Bit_RESET : Bit_SET);		//如果status等于led_ON，则返回Bit_SET，否则返回Bit_RESET
	
	buzzer_info.Buzzer_Status = status;

}
