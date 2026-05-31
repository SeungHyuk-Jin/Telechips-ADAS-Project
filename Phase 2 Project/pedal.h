/*
 * pedal.h
 *
 *  Created for TOPST VCP RTOS environment
 */

#ifndef __PEDAL_H__
#define __PEDAL_H__

// 공용 헤더
#include <sal_api.h>
#include <adc.h>

// 기능 헤더 
#include <mcb.h>
#include <uart5.h>


// ADC 컨트롤러 베이스 주소
#define ADC_TEST_REG(N)   ((volatile ADCTest_t *)(0xA0080000UL + (N * 0x10000UL)))

// ADC 명령어 설정 레지스터
typedef union {
    uint32_t u32;
    struct {
        uint32_t cuNreg : 32;  // 채널 선택 비트 (1 << 채널 번호)
    } bReg;
} ADCCmdUnion_t;

// 클럭 설정 레지스터
typedef union {
    uint32_t u32;
    struct {
        uint32_t clDiv    : 8;  // 클럭 분주기
        uint32_t clIrqEn  : 1;  // 인터럽트 enable
        uint32_t clReqEn  : 1;  // DMA 요청 enable
        uint32_t reserved : 22;
    } bReg;
} ADCTestAdcclkU_t;

// 인터럽트 클리어 레지스터 (optional)
typedef union {
    uint32_t u32;
    struct {
        uint32_t acClrIrq : 1;
        uint32_t acClrReq : 1;
        uint32_t reserved : 30;
    } bReg;
} ADCTestAdcClrU_t;

// ADC 전체 레지스터 맵 구조체
typedef struct {
    ADCCmdUnion_t        atAdcCmd;       // 0x00
    ADCTestAdcClrU_t     atAdcClr;       // 0x04
    ADCTestAdcclkU_t     atAdcClk;       // 0x08
    uint32_t             atReserved0x0C[13];  // 0x0C ~ 0x3C

    uint32_t             atAdcTime;      // 0x40 (optional timing)
    uint32_t             atReserved0x44[15]; // 0x44 ~ 0x7C

    uint32_t             atAdcAin00;     // 0x80
    uint32_t             atAdcAin01;     // 0x84
    uint32_t             atAdcAin02;     // 0x88
    uint32_t             atAdcAin03;     // 0x8C
    uint32_t             atAdcAin04;     // 0x90
    uint32_t             atAdcAin05;     // 0x94
    uint32_t             atAdcAin06;     // 0x98
    uint32_t             atAdcAin07;     // 0x9C
    uint32_t             atAdcAin08;     // 0xA0
    uint32_t             atAdcAin09;     // 0xA4
    uint32_t             atAdcAin10;     // 0xA8
    uint32_t             atAdcAin11;     // 0xAC
    uint32_t             atAdcAin12;     // 0xB0
    uint32_t             atAdcAin13;     // 0xB4
    uint32_t             atAdcAin14;     // 0xB8
    uint32_t             atAdcAin15;     // 0xBC
} ADCTest_t;

#define MAX_speed 200.0f            // 최대 속도 km/h 단위

// 페달 압력 -> 속도 변환 (pedalTask)
#define pressure_range_MAX 4095.0f
// 가속
#define power_accel (100.0f/3.5f)
#define velocity_per_pressure_accel (power_accel / pressure_range_MAX)
// 제동
#define power_brake (100.0f/2.67f)
#define velocity_per_pressure_brake (power_brake / pressure_range_MAX)

#define PEDAL_HOLD_DURATION_TICK 2

extern float current_speed;

extern volatile uint32_t accel_pedal_hold_counter;  // 유지 타이머 (tick 단위)
extern volatile uint32_t brake_pedal_hold_counter;  // 유지 타이머 (tick 단위)
extern volatile uint8_t accel_pedal_pressed;  // 1: 밟힘, 0: 안 밟힘
extern volatile uint8_t brake_pedal_pressed;
extern float accel_val;
extern float brake_val;

extern uint32 pedalTaskId;
extern uint32 pedalTaskStk[512];

// 태스크 본체
void PEDAL_Task(void *pArg);

// Pedal Init 함수
void FSR_Pedal_Init(void);

// 페달 압력값 함수
uint16_t accel_Pedal_ReadRaw(void);
uint16_t brake_Pedal_ReadRaw(void);


#endif /* __PEDAL_H__ */