#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"

typedef struct
{
	_Bool Motor_Status;
} MOTOR_INFO;

#define MOTOR_ON	1
#define MOTOR_OFF	0

extern MOTOR_INFO motor_info;

void Motor_Init(void);
void Motor_Set(_Bool status);

#endif
