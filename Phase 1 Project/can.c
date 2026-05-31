/*
 * can.c
 *
 *  Created on: Jul 2, 2025
 *      Author: user
 */
#include "can.h"


CAN_RxHeaderTypeDef lastRxHeader;
uint8_t lastRxData[8];
unsigned char Id100_Flag = 0;
unsigned char Id200_Flag = 0;

volatile uint8_t can_send_flag;

CAN_HandleTypeDef hcan1;

char can_data[8];

// CAN 수신 필터 함수 (반드시 있어야 함)
void CAN1_Filter_Config(void) {
	CAN_FilterTypeDef filterConfig;

	filterConfig.FilterActivation = ENABLE;
	filterConfig.FilterBank = 0;
	filterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;

	filterConfig.FilterIdHigh = 0x0000;         // ID 필터 상위
	filterConfig.FilterIdLow = 0x0000;          // ID 필터 하위
	filterConfig.FilterMaskIdHigh = 0x0000;     // 모든 ID 수신!!
	filterConfig.FilterMaskIdLow = 0x0000;

	filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

//    filterConfig.FilterIdHigh = 0x6420;     // 4440 -> 222 / 6420 -> 321
//    filterConfig.FilterIdLow  = 0x0000;
//    filterConfig.FilterMaskIdHigh = 0xFFE0;
//    filterConfig.FilterMaskIdLow  = 0x0000;

	if (HAL_CAN_ConfigFilter(&hcan1, &filterConfig) != HAL_OK) {
		Error_Handler();
	}
}

void CAN1_Send_Test_2(void) {
	CAN_TxHeaderTypeDef txHeader;
	uint8_t txData[1] = { 1 };
	uint32_t txMailbox;

	txHeader.DLC = 1;                          // 데이터 길이
	txHeader.StdId = 0x112;                    // 표준 ID
	txHeader.IDE = CAN_ID_STD;                 // 표준 ID 모드
	txHeader.RTR = CAN_RTR_DATA;               // 데이터 프레임
	txHeader.TransmitGlobalTime = DISABLE;

	if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK) {
		Error_Handler(); // 송신 실패 처리
	}
	HAL_Delay(100);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {

	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &lastRxHeader, lastRxData);

	switch (lastRxHeader.StdId) {
	case 0x111:
		// ID 0x100 처리
		// Uart3_Printf("interrupt in\r\n");
		Id100_Flag = 1;
		break;

	case 0x200:
		// ID 0x200 처리
		Id200_Flag = 1;
		break;

	default:
		// 무시 또는 로그
		break;
	}
}

void StartCANTask(void const *argument) {
	for (;;) {
//		CAN1_Send_Test_2(); // 송신 (delay 방식)

		if (Id100_Flag == 1) { // 수신 인터럽트 관련

			Uart3_Printf("interrupt flag in\r\n");

			Uart3_Printf("CAN 수신: ID=0x%X, Data=", lastRxHeader.StdId);
			for (int i = 0; i < lastRxHeader.DLC; i++) {
//				can_data[i]=lastRxData[i];
				Uart3_Printf("%X", lastRxData[i]);
			}
			Uart3_Printf("\r\n");

			Uart3_Printf("data[0] : %d\r\n",lastRxData[0]);
			Uart3_Printf("acc : %d\r\n",accel_pedal_pressed);
			Uart3_Printf("bra : %d\r\n",brake_pedal_pressed);
			if (lastRxData[0] == 1 && (accel_pedal_pressed || brake_pedal_pressed)){
				CAN1_Send_Test_2();
				Uart3_Printf("운전자 의식 확인\r\n");
				Uart3_Printf("MCB 기능 정지\r\n");
				pedal_input_enabled = 1;
				osThreadSuspend(sisTaskHandle);
			}
			else {
				osThreadResume(sisTaskHandle);
			}

			Id100_Flag = 0; 	// clear
		}

		if ( can_send_flag ) {
			can_send_flag = 0;
			CAN1_Send_Test_2();
		}

		osDelay(100);
	}
}

