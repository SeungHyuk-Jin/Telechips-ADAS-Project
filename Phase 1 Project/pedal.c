/*
 * pedal.c
 *
 *  Created on: Jul 2, 2025
 *      Author: user
 */
#include "pedal.h"
#include "cmsis_os.h"

volatile uint32_t accel_pedal_hold_counter = 0;  // 유지 타이머 (tick 단위)
volatile uint32_t brake_pedal_hold_counter = 0;  // 유지 타이머 (tick 단위)
volatile uint8_t accel_pedal_pressed = 0;  // 1: 밟힘, 0: 안 밟힘
volatile uint8_t brake_pedal_pressed = 0;  // 1: 밟힘, 0: 안 밟힘

// 주행 속도
float current_speed = 0;

float accel_val, brake_val;

void StartPedalTask(void const *argument) {


    for (;;) {
    	PedalData_t data = {0};
    	if (xQueueReceive(pedalQueueHandle, &data, 50) == pdPASS) {
//    		Uart3_Printf("큐에서 값 꺼내기\r\n");

			if (pedal_input_enabled) {
				accel_val = data.accel * velocity_per_pressure_accel / 10.0f;
				brake_val = data.brake * velocity_per_pressure_brake / 10.0f;
			}
			// 페달 플래그 갱신
			if (data.accel > 50.0f) {
				accel_pedal_pressed = 1;
				accel_pedal_hold_counter = PEDAL_HOLD_DURATION_TICK;
			}
			if (data.brake > 50.0f) {
				brake_pedal_pressed = 1;
				brake_pedal_hold_counter = PEDAL_HOLD_DURATION_TICK;
			}
			//Uart3_Printf("acc bra: %d %d\r\n",accel_pedal_pressed,brake_pedal_pressed);
		}

		if (accel_pedal_hold_counter > 0) {
			accel_pedal_hold_counter--;
			if (accel_pedal_hold_counter == 0) {
				accel_pedal_pressed = 0;
			}
		}
		if (brake_pedal_hold_counter > 0) {
			brake_pedal_hold_counter--;
			if (brake_pedal_hold_counter == 0) {
				brake_pedal_pressed = 0;
			}
		}


        // Uart3_Printf("가속도 : %.2f km/h\r\n",accel_val);
        // Uart3_Printf("감속도 : %.2f km/h\r\n",brake_val);

        current_speed += accel_val - brake_val;
        if(current_speed > MAX_speed) current_speed = MAX_speed;
        else if(current_speed < 0) current_speed = 0;

        //Uart3_Printf("현재 속도: %.2f km/h\r\n\n", current_speed);

        accel_val = 0;
        brake_val = 0;
        osDelay(100);  // 100ms 간격으로 검사
    }
}
