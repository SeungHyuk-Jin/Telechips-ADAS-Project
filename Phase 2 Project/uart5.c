#include "uart5.h"

SemaphoreHandle_t uart_mutex = NULL;

uint8 gRxBuf[RX_BUF_SIZE];
UartParam_t uart_param;



void uart5_init(void)
{
    if (uart_mutex == NULL)
        uart_mutex = xSemaphoreCreateMutex();
}

void uart_printf(const char *fmt, ...)
{
    if (uart_mutex == NULL) return;
    xSemaphoreTake(uart_mutex, portMAX_DELAY);

    char buffer[256];  // 출력 버퍼
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    mcu_printf("%s", buffer);  // 반드시 포맷 지정해서 출력해야 함

    xSemaphoreGive(uart_mutex);
}



// UART 인터럽트 핸들러
void UART_UserISR(void *pArg)
{
    sint8 err;
    uint8 recv;

    recv = (uint8)UART_GetChar(UART_DEBUG_CH, 0, &err);  // ISR-safe

    if (err < 0) return;

    ProcessInput(recv);
}

// 초기화 함수
void UART_ExampleInit(void)
{
    uart_param.sCh = UART_CH;
    uart_param.sPriority = 5;
    uart_param.sBaudrate = UART_BAUD;
    uart_param.sMode = UART_INTR_MODE;
    uart_param.sPortCfg = 4;  // 보드에 맞게 조정
    uart_param.sCtsRts = UART_CTSRTS_OFF;
    uart_param.sWordLength = WORD_LEN_8;
    uart_param.sFIFO = ENABLE_FIFO;
    uart_param.s2StopBit = TWO_STOP_BIT_OFF;
    uart_param.sParity = PARITY_SPACE;
    uart_param.sFnCallback = (GICIsrFunc)&UART_UserISR;

    UART_Close(UART_CH);           // 기존 열려 있던 UART 닫기
    UART_Open(&uart_param);        // 다시 열기

    mcu_printf("UART 인터럽트 모드 초기화 완료\n");
    mcu_printf("1: SCC 플래그 설정, 2: LFA 플래그 설정, 3/4: 메시지 출력\n");
}

// 입력 처리 함수
void ProcessInput(uint8 key)
{
    switch (key) {
        case '1':
            scc_flag ^= 1;
            break;
        case '2':
            lfa_flag ^= 1;
            break;
        case '3':
            dir_flag ^= 1;
            break;
        case '4':
            if (scc_flag == 0){
                scc_distance = (scc_distance < 3) ? scc_distance + 1 : 1;
            }
            break;
        case '5':
            mcb_flag = 1;
            break;
        case 'q':
            accel_val = 3.33;
            break;
        case 'w':
            brake_val = 4.44;
            break;
    }
}