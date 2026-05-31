/*
 * scc.h
 *
 *  Created for TOPST VCP RTOS environment
 */

#ifndef __SCC_H__
#define __SCC_H__

// 공용 헤더
#include <sal_api.h>

// 기능 헤더
#include <uss.h>
#include <uart5.h>
#include <pedal.h>


// SCC 제어 파라미터
extern float TARGET_SPEED;
extern float SAFE_DISTANCE;
extern float BRAKE_DISTANCE;
extern volatile uint8_t scc_flag;
extern volatile uint8_t scc_distance;

// 태스크 ID 및 스택 정의
extern uint32 sccTaskId;
extern uint32 sccTaskStk[512];

extern TaskHandle_t sccTaskHandle;

// 태스크 본체
void SCC_Task(void *pArg);

#endif /* __SCC_H__ */