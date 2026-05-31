/*
 * mcb.h
 *
 *  Created for TOPST VCP RTOS environment
 */

#ifndef __MCB_H__
#define __MCB_H__

// 공용 헤더 
#include <sal_api.h>
#include <wdt.h>

// 기능 헤더
#include <scc.h>
#include <lfa.h>
#include <uart5.h>
#include <pedal.h>
#include "can_config.h"
#include <can.h>
#include <can_demo.h>

// 태스크 ID 및 스택 정의
extern uint32 mcbTaskId;
extern uint32 mcbTaskStk[512];

extern volatile uint8_t mcb_flag;
extern int pedal_input_enabled;

void MCB_Task(void *pArg);

#endif /* __MCB_H__ */