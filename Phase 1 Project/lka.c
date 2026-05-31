/*
 * lka.c
 *
 *  Created on: Jul 2, 2025
 *      Author: user
 */

#include <lka.h>

volatile float angle;
volatile uint8_t lka_flag = 0;
volatile uint8_t lka_1st_flag = 0;
volatile uint8_t lka_2nd_flag = 0;
volatile uint8_t lka_on_flag = 0;
volatile uint8_t dir_flag = 0;

TIM_HandleTypeDef htim4;

void StartLKATask(void const *argument) {


    for(;;) {

		if (offset_val >= 0 && offset_val <= 100) {
			angle = ((float) offset_val - 50) * 1000 / (current_speed + 1) + 90;
		}
		if(angle > 180)	angle = 180;
		else if(angle < 0)	angle = 0;
		angle = angle * 500 / 90 + 1000;

//		Uart3_Printf("서보오프셋: %d\r\n",offset_val);
//		Uart3_Printf("서보 angle flo: %.2f\r\n",angle);

		TIM4->ARR = 19999;
		TIM4->CCR1 = (int)angle;

		osDelay(30);

//		Uart3_Printf("서보 task 종료\r\n\n");
//        osDelay(500);
    }
}
