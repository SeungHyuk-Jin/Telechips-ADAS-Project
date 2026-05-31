#ifndef __W5500_PORT_H__
#define __W5500_PORT_H__

// 공용 헤더
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gpio.h"
#include "gpsb.h" 
#include <app_cfg.h>

// 이더넷 ( W5500 - ioLiabrary)
#include "wizchip_conf.h" 
#include "socket.h"
#include "w5500.h"

// 기능 헤더
#include "uart5.h"
#include <lfa.h>
#include <pedal.h>
#include <scc.h>


// GPSB 포트 인덱스
#define GPSB_SDO    0   // MOSI 
#define GPSB_SDI    1   // MISO 
#define GPSB_SCLK   2   // SCLK 
#define GPSB_CS     3   // CS 
#define GPSB_FUNC   4 

static const uint32_t w5500_gpsbport[][4] = {
    { GPIO_GPB(6), GPIO_GPB(7), GPIO_GPB(4), GPIO_GPB(11) }
};

// ECCP 관련 부족한 매크로 (main.c에서 사용) 
#define ECCP_GPSB_DMA_SIZE   1024U           /* DMA 버퍼 크기, 필요시 조정 */
#define ECCP_GPSB_SPEED      10000000UL      /* 10MHz 예시 */
#define ECCP_GPSB_BPW        8UL             /* 8bit per word */

void ECCP_Complete(uint32 uiCh, uint32 iEvent, void *pArg);

// CS 핀 / SPI 채널 (환경에 맞춰 조정) 
#define W5500_CS_GPIO   GPIO_GPB(11)              // SCS 핀
#define W5500_SPI_CH    0UL 

#define TCP_SERVER_IP     {192,168,2,100}         // 서버 IP

#define SOCKET_CH0        1                      // 사용할 소켓 채널
#define TCP_SERVER_PORT0   5001                    // 서버 포트

#define SOCKET_CH         2                       // 사용할 소켓 채널
#define TCP_SERVER_PORT   5002                    // 서버 포트

#define SOCKET_CH2        3                      // 사용할 소켓 채널
#define TCP_SERVER_PORT2   5003                    // 서버 포트


extern _WIZCHIP WIZCHIP;

extern uint32 eth0TaskId;
extern uint32 eth0TaskStk[512];

extern uint32 ethTaskId;
extern uint32 ethTaskStk[512];

extern uint32 eth2TaskId;
extern uint32 eth2TaskStk[512];

extern SemaphoreHandle_t xSPIMutex;
extern SemaphoreHandle_t xWizchipMutex;

/* 콜백/유틸 선언 */


void W5500_Init(void);
void W5500_GPIO_Init(void);
void W5500_Port_Init(void);
void W5500_Init_DefaultNet(void);
void SPI_MutexInit(void);
void GPSB_Default_Init(void);

// 
void wizchip_select(void);
void wizchip_deselect(void);

// 연결 체크 ( 버젼 확인 )
uint8_t read_version_raw(void);
uint8_t getVERSIONR_burst(void);

// 이더넷 태스크 ( 포트 5001, 5002, 5003)
void ETH0_Task(void *pArg);
void ETH1_Task(void *pArg);
void ETH2_Task(void *pArg);

// 뮤텍스
void wiz_lock(void);
void wiz_unlock(void);

#endif /* __W5500_PORT_H__ */