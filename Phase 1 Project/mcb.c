/*
 * sis.c
 *
 *  Created on: Jul 2, 2025
 *      Author: user
 */

#include <mcb.h>

volatile uint8_t pedal_input_enabled = 1;

// 진동 감지 체크 함수

uint8_t Read_VibrationSensor(void)
{
    return HAL_GPIO_ReadPin(VIB_GPIO_PORT, VIB_PIN);
}

void StartSISTask(void const *argument) {
	for (;;) {
		pedal_input_enabled = 0;
		lka_flag = 0;
		scc_flag = 0;
		brake_val = pressure_range_MAX * velocity_per_pressure_brake / 10.0f;
		accel_val = 0;
		if (current_speed == 0) {
			Uart3_Printf("현재속도: 0 km/h, mcb 기능 종료\r\n");
			can_send_flag = 1;
			pedal_input_enabled = 1;
			osThreadSuspend(sisTaskHandle);
		}
        osDelay(50);  // 100ms 간격으로 검사
	}
}
