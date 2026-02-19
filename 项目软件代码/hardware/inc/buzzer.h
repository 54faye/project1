#ifndef __BUZZER_H
#define __BUZZER_H

#include "stm32f10x.h"

typedef struct
{

	_Bool Buzzer_Status;

} BUZZER_INFO;

#define BUZZER_ON		1

#define BUZZER_OFF	0

extern BUZZER_INFO buzzer_info;

void Buzzer_Init(void);
void buzzer_Set(_Bool status);

#endif
