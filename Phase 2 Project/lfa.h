/*
 * scc.h
 *
 *  Created for TOPST VCP RTOS environment
 */

#ifndef __LFA_H__
#define __LFA_H__

// 공용 헤더
#include <stdio.h>
#include <gpio.h>
#include <pdm.h>

// 기능 헤더
#include <uart5.h>

extern volatile uint8_t lfa_flag;
extern volatile uint8_t lfa_on_flag;
extern volatile uint8_t lfa_1st_flag;
extern volatile uint8_t lfa_2nd_flag;
extern volatile uint8_t dir_flag;
extern volatile uint8_t dir_flag_ai;
extern volatile float angle;
extern volatile int offset_val;

extern uint32 lfaTaskId;
extern uint32 lfaTaskStk[512];

void LFA_Task(void *pArg);
void SG90_SetAngle(uint32 angle_deg);

#endif /* __LFA_H__ */