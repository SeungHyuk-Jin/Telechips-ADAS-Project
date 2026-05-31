#ifndef __UART5_H__
#define __UART5_H__

// 공용 헤더
#include <sal_api.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <stdarg.h>
#include <stdio.h>
#include <debug.h>

// 기능 헤더 
#include <uart.h>
#include <scc.h>
#include <lfa.h>
#include <mcb.h>
#include <pedal.h>

#define UART_CH       UART_DEBUG_CH
#define UART_BAUD     115200U
#define RX_BUF_SIZE   16

extern SemaphoreHandle_t uart_mutex;

extern uint8 gRxBuf[RX_BUF_SIZE];
extern UartParam_t uart_param;

void uart_printf(const char *fmt, ...);
void uart5_init(void);  // UART5 초기화 함수 (뮤텍스 포함)
void ProcessInput(uint8 key);
void UART_ExampleInit(void);
void UART_UserISR(void *pArg);


#endif /* __UART5_H__ */