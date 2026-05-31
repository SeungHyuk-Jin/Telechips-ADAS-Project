///*
// * servo.c
// *
// *  Created on: Jul 4, 2025
// *      Author: lees
// */
//
//#include "servo.h"
//float angle;
//
//void StartServoTask(void const *argument) {
//	for(;;) {
////		Uart3_Printf("서보 task 시작\r\n");
////		angle = ((float)offset_val - 50) * 50 / (current_speed + 1) + 90;
////		if(angle > 180)	angle = 180;
////		if(angle < 0)	angle = 0;
////		angle = angle * 500 / 90 + 1500;
////		Uart3_Printf("서보오프셋: %d\r\n",offset_val);
////		Uart3_Printf("서보 angle flo: %.2f\r\n",angle);
////		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, (int)angle);
////		osDelay(100);
////
////		Uart3_Printf("서보 task 종료\r\n\n");
//	}
//}
//
////  // 예: 1ms 펄스 (0도)
////	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 1000);
////	osDelay(100);
////
////	// 예: 1.5ms 펄스 (90도)
////	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 1500);
////	osDelay(100);
////
////	// 예: 2ms 펄스 (180도)
////	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 2000);
////	osDelay(100);
