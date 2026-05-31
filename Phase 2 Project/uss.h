#ifndef __USS_H__
#define __USS_H__

// 공용 헤더
#include <bsp.h>
#include <gpio.h> 
#include <sal_api.h>
#include <timer.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>

// 기능 헤더
#include <uart5.h>

#define TRIG_PIN    GPIO_GPB(24)
#define ECHO_PIN    GPIO_GPB(23)
#define TIMER_CH_USED  TIMER_CH_5 

extern volatile float	shared_distance;

extern uint32 ultrasonicTaskId;
extern uint32 ultrasonicTaskStk[512]; 

extern TaskHandle_t ultrasonicTaskHandle;

void	USS_Task(void *pArg);
void	timer_init_for_us(void);

#endif // __USS_H__