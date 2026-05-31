#include "scc.h"

float TARGET_SPEED;    // km/h 고정 목표 속도
float SAFE_DISTANCE;    // 안전 거리 (cm)
float BRAKE_DISTANCE = 5.0f;    // 최소 안전 거리 (cm)

uint32 sccTaskId;
uint32 sccTaskStk[512];

TaskHandle_t sccTaskHandle =    NULL;

volatile uint8_t scc_flag     = 0;
volatile uint8_t scc_distance = 1;

void SCC_Task(void *pArg)
{
    (void)pArg;

    int dist_int, dist_frac, speed_int, speed_frac;
    
    uart_printf("[SCC] SCC_TASK 시작\n");

    while (1)
    {
        if (accel_pedal_pressed || brake_pedal_pressed) {
            scc_flag = 0;
        }

        float dist_cm      = shared_distance;
        float target_speed = TARGET_SPEED;
        float ratio        = 1.0f;

        if (dist_cm <= BRAKE_DISTANCE) {
            target_speed = 0;
            ratio = 0.0f;
        } else if (dist_cm < SAFE_DISTANCE) {
            ratio = (dist_cm - BRAKE_DISTANCE) / (SAFE_DISTANCE - BRAKE_DISTANCE);
            target_speed = TARGET_SPEED * ratio;
        }

        float accel_step = power_accel / 10.0f * ratio;
        float decel_step = power_brake / 10.0f * (1.0f - ratio);

        if (current_speed < target_speed) {
            current_speed += accel_step;
            if (current_speed > target_speed)
                current_speed = target_speed;
        } else if (current_speed > target_speed) {
            current_speed -= decel_step;
            if (current_speed < target_speed)
                current_speed = target_speed;
        }

        dist_int = (int)dist_cm;
        dist_frac = (int)((dist_cm - dist_int) * 100);  // 소수점 둘째자리까지

        speed_int = (int)current_speed;
        speed_frac = (int)((current_speed - speed_int) * 100);

        // 출력
        uart_printf("[SCC] 거리: %d.%02d cm, 속도: %d.%02d km/h\n", dist_int, dist_frac, speed_int, speed_frac);

        SAL_TaskSleep(100); // 100ms 주기
    }
}
