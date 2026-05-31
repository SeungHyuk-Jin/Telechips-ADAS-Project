/*
 * uss.c
 *
 *  Created on: Jul 2, 2025
 *      Author: user
 */
#include "uss.h"


// 초음파 센서

#define TRIG_PIN GPIO_PIN_5
#define TRIG_PORT GPIOA
#define ECHO_PIN GPIO_PIN_6
#define ECHO_PORT GPIOA

volatile float shared_distance;

TIM_HandleTypeDef htim2;

float read_distance_cm(void)
{
    uint32_t start, end, timeout;

    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
    delay_us(2);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

    timeout = 1000000;
    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_RESET && timeout--);
    if (timeout == 0) return 0;
    start = __HAL_TIM_GET_COUNTER(&htim2);

    timeout = 1000000;
    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_SET && timeout--);
    if (timeout == 0) return 0;
    end = __HAL_TIM_GET_COUNTER(&htim2);

    uint32_t duration = end - start;
    float distance = ((float)duration * 0.0343f) / 2.0f;

    return distance;
}

void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

int compare_float(const void *a, const void *b)
{
    float fa = *(float*)a;
    float fb = *(float*)b;

    if (fa < fb) return -1;
    else if (fa > fb) return 1;
    else return 0;
}

void StartUSSTask(void const *argument) {
	for(;;) {

	    float samples[9];

	    for (int i = 0; i < 9; i++) {
	        samples[i] = read_distance_cm();
	        osDelay(20);  // 50ms 간격 (센서 재충전 시간 고려)
	    }

	    // 정렬 후 중간값 반환
	    qsort(samples, 9, sizeof(float), compare_float);
	    shared_distance = samples[4];

		// Uart3_Printf("\r\n거리: %.2f cm\r\n", shared_distance);


	}
}
