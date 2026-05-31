/*
 * uart.c
 *
 *  Created on: Jul 2, 2025
 *      Author: user
 */

#include "uart.h"

// UART 수신 버퍼
uint8_t rx_data;

osMutexId uartMutexHandle;
osMutexDef(uartMutex);

UART_HandleTypeDef huart3;

void UART_Printf_Init(void)
{
    uartMutexHandle = osMutexCreate(osMutex(uartMutex));
}

// Uart3 Printf 관련

int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

void Uart3_Printf(char *fmt,...)
{
	osMutexWait(uartMutexHandle, osWaitForever);

    va_list ap;
    char string[256];
    va_start(ap, fmt);
    vsprintf(string, fmt, ap);
    va_end(ap);
    HAL_UART_Transmit(&huart3, (uint8_t*)string, strlen(string), HAL_MAX_DELAY);

    osMutexRelease(uartMutexHandle);
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) {
        switch (rx_data) {
            case '1': scc_flag = !scc_flag; break;
            case '2': lka_flag = !lka_flag; break;
            case '3': dir_flag = !dir_flag; break;
            case 'q': accel_pedal_pressed ^= 1; break;  // accel 토글
            case 'w': brake_pedal_pressed ^= 1; break;  // brake 토글
        }

        // 다음 수신도 이어서 받도록 재설정
        HAL_UART_Receive_IT(&huart3, (uint8_t *)&rx_data, 1);
    }
}
