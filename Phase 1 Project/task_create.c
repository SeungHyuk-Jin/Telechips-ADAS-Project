/*
 * task_create.c
 *
 *  Created on: Jul 2, 2025
 *      Author: user
 */

#include "task_create.h"

osThreadId mainTaskHandle;
osThreadId sccTaskHandle;
osThreadId lkaTaskHandle;
osThreadId ussTaskHandle;
osThreadId spdTaskHandle;
osThreadId canTaskHandle;
osThreadId sisTaskHandle;
osThreadId pedalTaskHandle;
osThreadId servoTaskHandle;
osThreadId tcpTaskHandle;
osThreadId tcp2TaskHandle;
QueueHandle_t pedalQueueHandle;
osThreadId tcpSendTaskHandle;

void Create_AllTasks(void)
{
//	osThreadDef(StartMainTask,StartMainTask, osPriorityNormal, 1, 512);
//	mainTaskHandle = osThreadCreate(osThread(StartMainTask), NULL);
//
//	osThreadDef(StartUSSTask,StartUSSTask, osPriorityNormal, 1, 256);
//	ussTaskHandle = osThreadCreate(osThread(StartUSSTask), NULL);
//
//	osThreadDef(StartSPDTask,StartSPDTask, osPriorityNormal, 1, 512);
//	spdTaskHandle = osThreadCreate(osThread(StartSPDTask), NULL);
//
//	osThreadDef(StartCANTask, StartCANTask, osPriorityNormal, 1,512);
//	canTaskHandle = osThreadCreate(osThread(StartCANTask), NULL);
//
//	osThreadDef(StartSCCTask, StartSCCTask, osPriorityNormal, 1, 512);
//	sccTaskHandle = osThreadCreate(osThread(StartSCCTask), NULL);
//	osThreadSuspend(sccTaskHandle);  // 바로 중지
//
//	osThreadDef(StartLKATask, StartLKATask, osPriorityNormal, 1, 512);
//	lkaTaskHandle = osThreadCreate(osThread(StartLKATask), NULL);
//	osThreadSuspend(lkaTaskHandle);  // 바로 중지
//
//	// 충격감지센서
//	osThreadDef(StartSISTask, StartSISTask, osPriorityNormal, 1, 512);
//	sisTaskHandle = osThreadCreate(osThread(StartSISTask), NULL);
//	osThreadSuspend(sisTaskHandle);
//
//	// 페달압력센서
//	osThreadDef(StartPedalTask, StartPedalTask, osPriorityNormal, 1, 512);
//	pedalTaskHandle = osThreadCreate(osThread(StartPedalTask), NULL);

	osThreadDef(tcpTask, tcp_server_thread, osPriorityLow, 0, 512);
	tcpTaskHandle = osThreadCreate(osThread(tcpTask), (void *)5000);

	osThreadDef(tcpTask2, tcp_server_thread2, osPriorityBelowNormal, 0, 512);
	tcp2TaskHandle = osThreadCreate(osThread(tcpTask2), (void *)5001);


	// Normal ( Main, Speed(모터제어), LKA (차선유지) )
	osThreadDef(StartMainTask, StartMainTask, osPriorityNormal, 1, 512);
	mainTaskHandle = osThreadCreate(osThread(StartMainTask), NULL);

	osThreadDef(StartSPDTask, StartSPDTask, osPriorityNormal, 1, 512);
	spdTaskHandle = osThreadCreate(osThread(StartSPDTask), NULL);

	osThreadDef(StartLKATask, StartLKATask, osPriorityNormal, 1, 512);
	lkaTaskHandle = osThreadCreate(osThread(StartLKATask), NULL);
	osThreadSuspend(lkaTaskHandle);  // 바로 중지


	// AboveNormal ( Pedal(속도제어), Uss(초음파), SCC(스마트 크루즈) )
	osThreadDef(StartPedalTask, StartPedalTask, osPriorityAboveNormal, 1, 512);
	pedalTaskHandle = osThreadCreate(osThread(StartPedalTask), NULL);

	osThreadDef(StartUSSTask, StartUSSTask, osPriorityAboveNormal, 1, 256);
	ussTaskHandle = osThreadCreate(osThread(StartUSSTask), NULL);

	osThreadDef(StartSCCTask, StartSCCTask, osPriorityAboveNormal, 1, 512);
	sccTaskHandle = osThreadCreate(osThread(StartSCCTask), NULL);
	osThreadSuspend(sccTaskHandle);  // 바로 중지


	// High ( CAN )
	osThreadDef(StartCANTask, StartCANTask, osPriorityHigh, 1, 512);
	canTaskHandle = osThreadCreate(osThread(StartCANTask), NULL);


	// Realtime ( MCB(충격감지) )
	osThreadDef(StartSISTask, StartSISTask, osPriorityRealtime, 1, 512);
	sisTaskHandle = osThreadCreate(osThread(StartSISTask), NULL);
	osThreadSuspend(sisTaskHandle);



	pedalQueueHandle = xQueueCreate(20, sizeof(PedalData_t)); // 20개까지 누적가능 큐생성
}
