#include <pedal.h>

volatile uint32_t	accel_pedal_hold_counter = 0; // 유지 타이머 (tick 단위)
volatile uint32_t	brake_pedal_hold_counter = 0; // 유지 타이머 (tick 단위)
volatile uint8_t	accel_pedal_pressed = 0;       // 1: 밟힘, 0: 안 밟힘
volatile uint8_t	brake_pedal_pressed = 0;       // 1: 밟힘, 0: 안 밟힘

// 주행 속도
float				current_speed = 0.0f;

float accel_val, brake_val;

uint32				pedalTaskId;
uint32				pedalTaskStk[512];

void FSR_Pedal_Init(void)
{
    // ADC0 클럭: 120MHz → 20MHz (분주기 설정)
    ADC_TEST_REG(0)->atAdcClk.bReg.clDiv = ((120 / 20) / 2) - 1;
    ADC_TEST_REG(0)->atAdcClk.bReg.clIrqEn = 0;
}

uint16_t accel_Pedal_ReadRaw(void)
{
    // 변환 시작
    ADC_TEST_REG(0)->atAdcCmd.bReg.cuNreg = (1 << 1);

    // 간단한 변환 대기 시간 (정밀하지 않지만 충분히 짧게)
    for (volatile int i = 0; i < 10000; i++);

    // 결과 값 반환 (12bit 유효, 0~4095)
    return (uint16_t)(ADC_TEST_REG(0)->atAdcAin00 & 0x0FFF);
}

uint16_t brake_Pedal_ReadRaw(void)
{
    // 변환 시작
    ADC_TEST_REG(0)->atAdcCmd.bReg.cuNreg = (1 << 2);

    // 간단한 변환 대기 시간 (정밀하지 않지만 충분히 짧게)
    for (volatile int i = 0; i < 10000; i++);

    // 결과 값 반환 (12bit 유효, 0~4095)
    return (uint16_t)(ADC_TEST_REG(0)->atAdcAin00 & 0x0FFF);
}

void	PEDAL_Task(void *pArg)
{
	(void)pArg;

    int cnt;

	FSR_Pedal_Init();

	while (1)
	{
		uint32 raw_val = accel_Pedal_ReadRaw();

		uint32 raw_val2 = brake_Pedal_ReadRaw();

		float voltage = (raw_val / 4095.0f) * 3.3f;
		float voltage2 = (raw_val2 / 4095.0f) * 3.3f;

		if (pedal_input_enabled) {
			// 값 출력
      		// uart_printf("[accel] raw: %d, voltage: %d.%02d V\n", (int)raw_val, (int)voltage, (int)(((voltage - (int)voltage)) * 100));
			// uart_printf("[brake] raw: %d, voltage: %d.%02d V\n", (int)raw_val2, (int)voltage2, (int)(((voltage2 - (int)voltage2)) * 100));
			if (raw_val > 50.0f) accel_val = raw_val * velocity_per_pressure_accel / 10.0f;
			if (raw_val2 > 50.0f) brake_val = raw_val2 * velocity_per_pressure_brake / 10.0f;
		}
		if (raw_val > 50.0f)
		{
			accel_pedal_pressed = 1;
			accel_pedal_hold_counter = PEDAL_HOLD_DURATION_TICK;
		}
		if (raw_val2 > 50.0f)
		{
			brake_pedal_pressed = 1;
			brake_pedal_hold_counter = PEDAL_HOLD_DURATION_TICK;
		}

        if (accel_pedal_hold_counter > 0) {
			accel_pedal_hold_counter--;
			if (accel_pedal_hold_counter == 0) {
				accel_pedal_pressed = 0;
			}
		}
		if (brake_pedal_hold_counter > 0) {
			brake_pedal_hold_counter--;
			if (brake_pedal_hold_counter == 0) {
				brake_pedal_pressed = 0;
			}
		}

        current_speed += accel_val - brake_val;
        if(current_speed > MAX_speed) current_speed = MAX_speed;
        else if(current_speed < 0) current_speed = 0;

        cnt++;

        if (cnt>9) {
            uart_printf("현재 속도 : %d.%02d km/h\n", (int)current_speed, (int)(((current_speed - (int)current_speed)) * 100));
            cnt = 0;
        }
        //uart_printf("엑셀 눌림: %d , 브레이크 눌림: %d\n", accel_pedal_pressed, brake_pedal_pressed);

        accel_val = 0;
        brake_val = 0;
		
        SAL_TaskSleep(100);
	}
}