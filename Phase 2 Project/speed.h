/*
 * pedal.h
 *
 *  Created for TOPST VCP RTOS environment
 */

#ifndef __SPEED_H__
#define __SPEED_H__

// 공용 헤더
#include <sal_api.h>
#include <stdio.h>
#include <gpio.h>
#include <pdm.h>

// 기능 헤더
#include <pedal.h>
#include <uart5.h>

#define MOTOR_PWM_CH         1       // PDM 채널 1 A11
#define MOTOR_PWM_PORT       GPIO_PERICH_CH0  
#define MOTOR_PWM_PERIOD_NS  1000000   // 50us → 20kHz
#define MOTOR_IN1_PIN        GPIO_GPB(20) // 26
#define MOTOR_IN2_PIN        GPIO_GPB(19) // 27

extern uint32 speedTaskId;
extern uint32 speedTaskStk[512];

void Motor_SetPWM(uint32 duty_percent);
void Motor_Init(void);
void Motor_Forward(uint32 duty_percent);
void Motor_Backward(uint32 duty_percent);
void Motor_Backward(uint32 duty_percent);
void Motor_Stop(void);
void SPEED_Task(void *pArg);

#endif // __SPEED_H__