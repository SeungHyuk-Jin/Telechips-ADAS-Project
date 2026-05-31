/*
 * speed.c
 *
 *  Created on: Jul 2, 2025
 *      Author: user
 */

// 모터
#include "speed.h"

TIM_HandleTypeDef htim3;

void Motor_Init(void)
{
    // 방향핀 초기값
    HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, GPIO_PIN_RESET);

    // PWM 타이머 시작
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

void Motor_Forward(uint16_t speed_percent)
{
    HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, GPIO_PIN_RESET);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, speed_percent * 10); // 0~1000
}

void Motor_Backward(uint16_t speed_percent)
{
    HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, GPIO_PIN_SET);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, speed_percent * 10); // 0~1000
}

void Motor_Stop(void)
{
    HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, GPIO_PIN_RESET);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
}

void StartSPDTask(void const *argument) {
    for (;;) {
    	// Uart3_Printf("%d\r\n", pedal_pressed);
    	int speed = 0;

    	if ( current_speed == 0 ) {
    		speed = 0;
    	}
    	else {
    		speed = 20 + current_speed * 80/200;
    	}

    	Motor_Forward(speed);   // 지정한 0~100% 속도로 전진

        osDelay(100);
    }
}
