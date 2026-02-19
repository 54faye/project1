//单片机头文件
#include "stm32f10x.h"

//硬件驱动
#include "key.h"
#include "delay.h"
#include "Led.h"

/*
************************************************************
*	函数名称：	led_Init
*
*	函数功能：	蜂鸣器初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void key_Init(void)
{

	GPIO_InitTypeDef gpio_initstruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//打开GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);   //开启AFIO时钟
	
	gpio_initstruct.GPIO_Mode = GPIO_Mode_IPU;				//设置为输出
	gpio_initstruct.GPIO_Pin = GPIO_Pin_1;						//将初始化的Pin脚
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;				//可承载的最大频率
	GPIO_Init(GPIOA, &gpio_initstruct);							//初始化key
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource1);
	
	EXTI_InitTypeDef EXTI_InitStructure;//结构体类型名EXTI_InitTypeDef，变量名EXTI_InitStructure
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//因为上面是GPIO_Mode_IPU设置为高电平，所以触发中断是下降
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //分组方式，整个芯片只能用一种。如放在模块中进行分组，要确保每个模块分组都选的是同一个；或者将这个代码放在主函数的最开始
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//因为我们这个程序只有一个，所以中断优先级的配置也是非常随意的
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	



}
void EXTI1_IRQHandler(void)
{
	DelayXms(10);
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)==RESET)
	{
		if(led_info.Led_Status==LED_ON)
		{
			Led_Set(LED_OFF);
		}else
		{
			Led_Set(LED_ON);
		}
	
	}
	EXTI_ClearITPendingBit(EXTI_Line1);
}



