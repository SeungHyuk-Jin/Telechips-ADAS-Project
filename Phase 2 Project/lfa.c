#include "lfa.h"

volatile uint8_t lfa_flag = 0;
volatile uint8_t lfa_on_flag = 0;
volatile uint8_t lfa_1st_flag = 0;
volatile uint8_t lfa_2nd_flag = 0;
volatile uint8_t dir_flag = 0;
volatile uint8_t dir_flag_ai = 0;
volatile float angle;
volatile int offset_val = 50;

uint32 lfaTaskId;
uint32 lfaTaskStk[512];

void SG90_SetAngle(uint32 angle_deg)
{
    PDMModeConfig_t pwm_cfg;
    uint32 wait_cnt;
    uint32 duty_ns;

    if (angle_deg > 180) angle_deg = 180;

    // 듀티 계산 (1ms ~ 2ms 사이)
    duty_ns = 1000000 + (angle_deg * 1000000) / 180;

    // PDM 설정
    pwm_cfg.mcPortNumber      = GPIO_PERICH_CH0;  // A10
    pwm_cfg.mcOperationMode   = PDM_OUTPUT_MODE_PHASE_1;
    pwm_cfg.mcInversedSignal  = 0;
    pwm_cfg.mcOutSignalInIdle = 0;
    pwm_cfg.mcLoopCount       = 0;
    pwm_cfg.mcOutputCtrl      = 0;
    pwm_cfg.mcPeriodNanoSec1  = 20000000;         // 20ms 주기
    pwm_cfg.mcDutyNanoSec1    = duty_ns;
    pwm_cfg.mcPeriodNanoSec2  = 0;
    pwm_cfg.mcDutyNanoSec2    = 0;

    PDM_Disable(0, PMM_ON);
    wait_cnt = 0;
    while (PDM_GetChannelStatus(0))
    {
        SAL_TaskSleep(1);
        if (++wait_cnt > 100) break;
    }

    PDM_SetConfig(0, &pwm_cfg);
    PDM_Enable(0, PMM_ON);
}

void LFA_Task(void *pArg)
{
    (void)pArg;

    
    while (1)
    {
        // SG90_SetAngle(0);    // 0도
        // SAL_TaskSleep(1000); // 1초 대기

        // SG90_SetAngle(90);   // 90도
        // SAL_TaskSleep(1000);

        // SG90_SetAngle(180);  // 180도
        // SAL_TaskSleep(1000);

        if (offset_val >= 0 && offset_val <= 100) {
			angle = ((float) offset_val - 50) * 1000 / (current_speed + 1) + 90;
		}
        if(angle > 180)	angle = 180;
		else if(angle < 0)	angle = 0;

        SG90_SetAngle(angle);
        
        SAL_TaskSleep(100);
    }
}