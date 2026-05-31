#include "uss.h"

uint32			ultrasonicTaskId;
uint32			ultrasonicTaskStk[512];

TaskHandle_t ultrasonicTaskHandle = NULL;

volatile float	shared_distance = 0;

// 중간값 필터용 비교 함수
static int	compare_float(const void *a, const void *b)
{
	float	fa;
	float	fb;

	fa = *(float *)a;
	fb = *(float *)b;
	return (fa > fb) - (fa < fb);
}

// 타이머 초기화 및 시작 (Free-run, 1us 단위)
void	timer_init_for_us(void)
{
	uart_printf("[USS] 타이머 초기화 시작\n");
	TIMER_EnableWithMode(TIMER_CH_USED,
							0,                // 비교값 불필요
							TIMER_OP_FREERUN, // Free-run mode
							NULL,
							NULL // 인터럽트 사용 안함
	);
	uart_printf("[USS] 타이머 초기화 완료\n");
}

// 현재 시간(us) 단위로 읽기
static uint32_t	get_time_us(void)
{
	uint32_t reg = MCU_BSP_TIMER_BASE + (TIMER_CH_USED * 0x100UL) + 0x014UL;
	return (SAL_ReadReg(reg));
}

// 시작 시간으로부터 경과 시간(us) 계산
static uint32_t	get_elapsed_us(uint32_t start)
{
	uint32_t	now;

	now = get_time_us();
	return (now - start); // 32bit unsigned → overflow 자동 처리
}

// 마이크로초(us) 단위 지연
static void	delay_us(uint32_t us)
{
	uint32_t	start;

	start = get_time_us();
	while ((get_time_us() - start) < us)
		;
}

// ECHO 펄스 시간 측정 (us 단위)
static uint32_t	measure_echo_time_us(void)
{
	uint32_t	start_us = 0, end_us;
	uint32_t	timeout;

	start_us = 0, end_us = 0;
	timeout = 0;
	// ECHO가 HIGH 될 때까지 대기 (최대 30ms)
	while (GPIO_Get(ECHO_PIN) == 0 && timeout++ < 30000)
	{
		delay_us(1);
	}
	if (timeout >= 30000)
	{
		//uart_printf("[USS] ECHO HIGH 대기 타임아웃\n");
		return (0);
	}
	start_us = get_time_us();
	timeout = 0;
	while (GPIO_Get(ECHO_PIN) == 1 && timeout++ < 30000)
	{
		delay_us(1);
	}
	end_us = get_time_us();
	return (end_us - start_us);
}

static float	read_distance_cm(void)
{
	uint32_t	echo_time_us;
	float		distance;
	float		f_time;
	float		f_product;

	GPIO_Set(TRIG_PIN, 0);
	delay_us(2);
	GPIO_Set(TRIG_PIN, 1);
	delay_us(20);
	GPIO_Set(TRIG_PIN, 0);
	echo_time_us = measure_echo_time_us();
	if (echo_time_us == 0)
	{
		//uart_printf("[USS] echo_time_us == 0 → 거리 측정 실패\n");
		return (0.0f);
	}
	//uart_printf("[USS] echo_time_us = %d us\n", (int)echo_time_us);
	f_time = (float)echo_time_us;
	f_product = f_time * 0.0343f;
	//uart_printf("[USS] f_time * 0.0343 = %d\n", (int)f_product);
	distance = f_product / 2.0f;
	//uart_printf("[USS] distance = %d cm\n", (int)distance);
	return (distance);
}

void	USS_Task(void *pArg)
{
	(void)pArg;
	uart_printf("[USS] Ultrasonic_Task 시작\n");

	// GPIO 설정
	GPIO_Config(TRIG_PIN, GPIO_FUNC(0) | GPIO_OUTPUT);
	GPIO_Set(TRIG_PIN, 0);
	GPIO_Config(ECHO_PIN, GPIO_FUNC(0) | GPIO_INPUT | GPIO_INPUTBUF_EN);
	//uart_printf("[USS] GPIO 설정 완료\n");
	while (1)
	{
		float samples[9];

	    for (int i = 0; i < 9; i++) {
	        samples[i] = read_distance_cm();
	        SAL_TaskSleep(20);  // 20ms 간격 ( 9번 체크 : 180ms)
	    }

	    // 정렬 후 중간값 반환
	    qsort(samples, 9, sizeof(float), compare_float);
	    shared_distance = samples[4];
	}
}
