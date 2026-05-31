#include <speed.h>

uint32 speedTaskId;
uint32 speedTaskStk[512];

void Motor_SetPWM(uint32 duty_percent)
{
    if (duty_percent > 100) duty_percent = 100;

    uint32 duty_ns = (MOTOR_PWM_PERIOD_NS * duty_percent) / 100;

    PDMModeConfig_t pwm_cfg;
    pwm_cfg.mcPortNumber      = MOTOR_PWM_PORT;
    pwm_cfg.mcOperationMode   = PDM_OUTPUT_MODE_PHASE_1;
    pwm_cfg.mcInversedSignal  = 0;
    pwm_cfg.mcOutSignalInIdle = 0;
    pwm_cfg.mcLoopCount       = 0;
    pwm_cfg.mcOutputCtrl      = 0;
    pwm_cfg.mcPeriodNanoSec1  = MOTOR_PWM_PERIOD_NS;
    pwm_cfg.mcDutyNanoSec1    = duty_ns;
    pwm_cfg.mcPeriodNanoSec2  = 0;
    pwm_cfg.mcDutyNanoSec2    = 0;

    PDM_Disable(MOTOR_PWM_CH, PMM_ON);
    while (PDM_GetChannelStatus(MOTOR_PWM_CH)) SAL_TaskSleep(1);

    PDM_SetConfig(MOTOR_PWM_CH, &pwm_cfg);
    PDM_Enable(MOTOR_PWM_CH, PMM_ON);
}

void Motor_Init(void)
{
    // 방향 핀 설정
    GPIO_Config(MOTOR_IN1_PIN, GPIO_FUNC(0) | GPIO_OUTPUT);
    GPIO_Config(MOTOR_IN2_PIN, GPIO_FUNC(0) | GPIO_OUTPUT);

    GPIO_Set(MOTOR_IN1_PIN, 0);
    GPIO_Set(MOTOR_IN2_PIN, 0);

    // PWM 핀 설정
    GPIO_PerichSel(GPIO_PERICH_SEL_PWMSEL_0, MOTOR_PWM_PORT);
}

void Motor_Forward(uint32 duty_percent)
{
    GPIO_Set(MOTOR_IN1_PIN, 1);
    GPIO_Set(MOTOR_IN2_PIN, 0);
    Motor_SetPWM(duty_percent);
}

void Motor_Backward(uint32 duty_percent)
{
    GPIO_Set(MOTOR_IN1_PIN, 0);
    GPIO_Set(MOTOR_IN2_PIN, 1);
    Motor_SetPWM(duty_percent);
}

void Motor_Stop(void)
{
    GPIO_Set(MOTOR_IN1_PIN, 0);
    GPIO_Set(MOTOR_IN2_PIN, 0);
    Motor_SetPWM(0);
}

void SPEED_Task(void *pArg)
{
    (void)pArg;
    
    Motor_Init();

    while (1)
    {
        int speed = 0;

    	if ( current_speed == 0 ) {
    		speed = 0;
    	}
    	else {
    		speed = 20 + (current_speed * 80/200);
    	}

    	Motor_Forward(speed);   // 지정한 0~100% 속도로 전진

        SAL_TaskSleep(100); // 100ms 주기
    }
}
