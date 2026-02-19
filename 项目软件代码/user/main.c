/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	main.c
	*
	*	作者： 		54faye
	*
	*	日期： 		2025-12-07
	*
	*	版本： 		V2.0
	*
	*	说明： 		接入onenet，上传数据和命令控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//网络协议层
#include "onenet.h"

//网络设备
#include "esp8266.h"

//硬件驱动
#include "delay.h"
#include "usart.h"
#include "Led.h"
#include "key.h"
#include "dht11.h"
#include "OLED.h"
#include "motor.h"
#include "hcsr04.h"
#include "light.h"
#include "buzzer.h"

//C库
#include <string.h>


#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"//通信协议，ip地址，端口号


/*
************************************************************
*	函数名称：	Hardware_Init
*
*	函数功能：	硬件初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		初始化单片机功能以及外接设备
************************************************************
*/
void Hardware_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断控制器分组设置

	Delay_Init();									//systick初始化
	
	Usart1_Init(115200);							//串口1，打印信息用
	
	Usart2_Init(115200);							//串口2，驱动ESP8266用
	
	//IIC_Init();										//软件IIC初始化
	key_Init();
	Led_Init();	//LED初始化
	OLED_Init(); //OLED初始化
	Motor_Init(); //电机初始化
	HCSR04_Init(); //超声波初始化
	Light_Init(); //光敏初始化
	Buzzer_Init();//蜂鸣器初始化
	OLED_Clear();
	OLED_ShowString(0, 0, "Tmp:  ℃ Hum:  %", OLED_8X16);
	OLED_ShowString(0, 16, "Dis:   cm Lt:   ", OLED_8X16);
	OLED_ShowString(0, 32, "Le:  Mot:  Bz: ", OLED_8X16);
	OLED_Update();

	if(DHT11_Init())
	{
		UsartPrintf(USART_DEBUG,"DHT11 Error \r\n");
		DelayMs(1000);
	}
	
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
	
}

/*
************************************************************
*	函数名称：	main
*
*	函数功能：	
*
*	入口参数：	无
*
*	返回参数：	0
*
*	说明：		
************************************************************
*/
u8 temp,humi;
u8 distance,light;
int main(void)
{
	
	unsigned short timeCount = 0;	//发送间隔变量
	
	unsigned char *dataPtr = NULL;
	
	Hardware_Init();				//初始化外围硬件
	ESP8266_Init();					//初始化ESP8266

	UsartPrintf(USART_DEBUG, "Connect MQTTs Server...\r\n");
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
		DelayXms(500);
	UsartPrintf(USART_DEBUG, "Connect MQTTs Server Success\r\n");
	while(OneNet_DevLink())			//接入OneNET
		DelayXms(500);
	
	OneNET_Subscribe();
	while(1)
	{
//		DHT11_Read_Data(&temp,&humi);
//		UsartPrintf(USART_DEBUG,"temp %d ,humi %d\r\n",temp,humi);
		if(++timeCount >= 30)  // 改为30：加快主循环刷新 (约300ms一次，既不卡顿也够快)
		{
			DHT11_Read_Data(&temp,&humi);
			distance = HCSR04_GetValue();
			light = Light_GetVal();
			
			
			//--功能1：温控逻辑：温度大于30度启动电机
			if (temp > 30)
				Motor_Set(MOTOR_ON); // 100%全速正转，排除启动力矩不足
			else
				Motor_Set(MOTOR_OFF);  // 停止

			// --- 功能2：智能光控灯 ---
            // 读取光敏 ADC 值 (0-4095)，通常光越强电压越低值越小，光越暗值越大
            if (light > 1000) // 这里的阈值1000需要根据实际光线环境调试，如果太暗则调大，太亮则调小
            {
                // 环境暗 -> 开灯
                Led_Set(LED_ON); 
            }
            else
            {
                // 环境亮 -> 关灯
                Led_Set(LED_OFF);
            }
			
            
            // --- 功能3：超声波防盗报警 ---
            if(distance > 0 && distance < 15.0) // 距离小于15cm报警
            {
                buzzer_Set(BUZZER_ON);
                // 也可以选择让 LED 闪烁作为视觉警告
            }
            else
            {
                buzzer_Set(BUZZER_OFF);
            }

			//数据显示
			OLED_ShowNum(32, 0, temp, 2, OLED_8X16);
			OLED_ShowNum(96, 0, humi, 2, OLED_8X16);
			OLED_ShowNum(32, 16, distance, 3, OLED_8X16);
			OLED_ShowNum(104, 16, light, 3, OLED_8X16);
            OLED_ShowNum(24, 32, (light > 1000), 1, OLED_8X16);
            OLED_ShowNum(72, 32, (temp > 30), 1, OLED_8X16);
            OLED_ShowNum(112, 32, (distance > 0 && distance < 15.0), 1, OLED_8X16);
            
            OLED_Update();

//			UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
			OneNet_SendData();									//发送数据
			
			timeCount = 0;
			ESP8266_Clear();
		}		
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
		{
			OneNet_RevPro(dataPtr);
			OLED_ShowNum(24, 32, led_info.Led_Status, 1, OLED_8X16);
			OLED_Update();
		}
		DelayMs(10);	
	}

}
