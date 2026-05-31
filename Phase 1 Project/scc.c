/*
 * scc.c
 *
 *  Created on: Jul 2, 2025
 *      Author: user
 */
#include "scc.h"

float TARGET_SPEED;
float SAFE_DISTANCE;
float BRAKE_DISTANCE;
volatile uint8_t scc_flag = 0;
volatile uint8_t scc_distance = 1;
int cnt = 0;

static float prev_dist_m = 0.0f;
static float rel_speed_mps = 0.0f;
static float TET = 0.0f;
static float TIT = 0.0f;
static float safe_time = 0.0f;  // 안전 상태 지속 시간

void StartSCCTask(void const *argument) {
    for (;;) {
        if (accel_pedal_pressed || brake_pedal_pressed) {
            scc_flag = 0;
        }

        float dist_cm = shared_distance;
        float curr_dist_m = dist_cm;
        float target_speed = TARGET_SPEED;
        float ratio = 1.0f;

        // 상대 속도 계산
        rel_speed_mps = (prev_dist_m - curr_dist_m) / 0.1f;
        prev_dist_m = curr_dist_m;

        // TTC 계산
        float TTC = 9999.0f;
        if (rel_speed_mps < -0.1f) {
            TTC = curr_dist_m / (-rel_speed_mps);
        } else {
            TTC = 9999.0f;  // 상대속도 거의 없으니 TTC는 무한
        }

        // TET / TIT 누적 또는 초기화 조건 체크
        if (TTC < 2.5f && TTC > 0.1f) {
            TET += 0.1f;
            TIT += (2.5f - TTC) * 0.1f;
            safe_time = 0.0f;  // 위험상태이므로 초기화 안 함
        } else {
            safe_time += 0.1f;
            if (safe_time >= 2.0f) {  // 2초 이상 안전 상태 유지 시 초기화
                TET = 0.0f;
                TIT = 0.0f;
                safe_time = 0.0f;
            }
        }

        // 속도 조절
        if (dist_cm <= BRAKE_DISTANCE) {
            target_speed = 0;
            ratio = 0.0f;
        } else if (dist_cm < SAFE_DISTANCE) {
            ratio = (dist_cm - BRAKE_DISTANCE) / (SAFE_DISTANCE - BRAKE_DISTANCE);
            target_speed = TARGET_SPEED * ratio;
        }

        float accel_step = power_accel / 10.0f * ratio;
        float decel_step = power_brake / 10.0f * (1 - ratio);

        if (current_speed < target_speed) {
            current_speed += accel_step;
            if (current_speed > target_speed)
                current_speed = target_speed;
        } else if (current_speed > target_speed) {
            current_speed -= decel_step;
            if (current_speed < target_speed)
                current_speed = target_speed;
        }

        // 주기 출력
//        cnt++;
//        if (cnt == 10) {
            Uart3_Printf("거리: %.2f cm, 속도: %.2f km/h, rel: %.4f m/s, TTC: %.2f s, TET: %.2f s, TIT: %.2f s², safe_time : %.2f \r\n",
                         dist_cm, current_speed, rel_speed_mps * 100 , TTC, TET, TIT, safe_time);
//            cnt = 0;
//        }

        osDelay(100);  // 100ms 주기
    }
}

