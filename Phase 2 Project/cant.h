/*
 * cant.h
 *
 *  Created for TOPST VCP RTOS environment
 */

#ifndef __CANT_H__
#define __CANT_H__

// 공용 헤더
#include <stdio.h>

// 기능 헤더
#include <uart5.h>

extern uint32 cantTaskId;
extern uint32 cantTaskStk[512];

void CANT_Task(void *pArg);

#endif /* __CANT_H__ */