// 공용 라이브러리
#include <lka.h>
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "stm32f4xx_hal_uart.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// 기능 헤더
#include "pedal.h"
#include "task_create.h"
#include "scc.h"


int offset_val = 50;
int dir_flag_pi;
int idx;

void tcp_client_send_to_rpi(const char *ip, uint16_t port, const char *message)
{
    struct netconn *conn;
    ip_addr_t server_ip;
    err_t err;

    conn = netconn_new(NETCONN_TCP);
    if (conn == NULL) {
    	Uart3_Printf("Client netconn_new() failed\r\n");
        return;
    }

    ipaddr_aton(ip, &server_ip);  // 문자열 IP → IP 구조체 변환

    err = netconn_connect(conn, &server_ip, port);
    if (err != ERR_OK) {
    	Uart3_Printf("Client connect failed: %d\r\n", err);
        netconn_delete(conn);
        return;
    }

    netconn_write(conn, message, strlen(message), NETCONN_COPY);
    Uart3_Printf("Sent to RPi: %s\r\n", message);

    netconn_close(conn);
    netconn_delete(conn);
}



void tcp_server_thread(void const *argument)
{
    struct netconn *conn, *newconn;
    err_t err;

    conn = netconn_new(NETCONN_TCP);
    if (conn == NULL) {
    	Uart3_Printf("netconn_new() failed\r\n");
        return;
    }

    err = netconn_bind(conn, NULL, (u16_t)(uint32_t)argument);
    if (err != ERR_OK) {
        Uart3_Printf("netconn_bind() failed: %d\r\n", err);
        netconn_delete(conn);
        return;
    }

    err = netconn_listen(conn);
    if (err != ERR_OK) {
    	Uart3_Printf("netconn_listen() failed: %d\r\n", err);
        netconn_delete(conn);
        return;
    }

    Uart3_Printf("TCP server started on port %d\r\n", (u16_t)(uint32_t)argument);

    while (1) {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK) {
        	Uart3_Printf("Client connected\r\n");

            struct netbuf *buf;
            void *data;
            u16_t len;

            static char recv_buffer[512];
            static int recv_len = 0;

            while ((netconn_recv(newconn, &buf)) == ERR_OK) {

            	do {
                    netbuf_data(buf, &data, &len);

                    // 버퍼 누적 저장
                    if (recv_len + len < sizeof(recv_buffer)) {
                        memcpy(recv_buffer + recv_len, data, len);
                        recv_len += len;

                        // 줄 단위 파싱 (\n 기준)
                        char *newline;
                        while ((newline = memchr(recv_buffer, '\n', recv_len)) != NULL) {
                        	uint32_t receive_tick = HAL_GetTick();
                        	int line_len = newline - recv_buffer;
                            if (line_len <= 0 || line_len >= 256) {
                                // 이상한 줄이면 건너뜀
                                memmove(recv_buffer, newline + 1, recv_len - line_len - 1);
                                recv_len -= (line_len + 1);
                                continue;
                            }

                            // 한 줄 꺼냄
                            char temp[256] = {0};
                            memcpy(temp, recv_buffer, line_len);
                            temp[line_len] = '\0';

                            // 처리한 줄 제거
                            memmove(recv_buffer, newline + 1, recv_len - line_len - 1);
                            recv_len -= (line_len + 1);

                            static int sync_offset = 0;

                            if (strncmp(temp, "SYNC:|", 6) == 0) {
                                uint32_t rpi_time = (uint32_t)atoi(temp + 6);
                                uint32_t stm_tick = HAL_GetTick();
                                sync_offset = rpi_time - stm_tick;
                                Uart3_Printf("[SYNC] offset set to %d ms\r\n", sync_offset);
                                continue;
                            }

                            // 파싱 시작
                            PedalData_t pedal_data = { 0 };
                            char *cval1 = strstr(temp, "a:");
                            char *cval2 = strstr(temp, "b:");

                            if (cval1 && cval2 && cval2 > cval1) {
                                char a_buf[16] = {0};
                                int len1 = cval2 - (cval1 + 2);
                                if (len1 > 0 && len1 < sizeof(a_buf)) {
                                    strncpy(a_buf, cval1 + 2, len1);
                                    a_buf[len1] = '\0';
                                    pedal_data.accel = atof(a_buf);
                                }

                                char b_buf[16] = {0};
                                strncpy(b_buf, cval2 + 2, sizeof(b_buf) - 1);
                                b_buf[sizeof(b_buf) - 1] = '\0';
                                pedal_data.brake = atof(b_buf);

                              //  Uart3_Printf("accel_val: %.2f\r\n", pedal_data.accel);
                                //Uart3_Printf("brake_val: %.2f\r\n", pedal_data.brake);
                                xQueueSend(pedalQueueHandle, &pedal_data, 0);
                            }

                            // 특수 문자 플래그 처리
                            if (strchr(temp, '!')) {
                                scc_flag ^= 1;
                                Uart3_Printf("flag_scc %d\r\n", scc_flag);
                            }
                            if (strchr(temp, '@')) {
                            	if ( dir_flag == 0 ){
                            		lka_flag ^= 1;
                            		Uart3_Printf("flag_lka %d\r\n", lka_flag);
                            	}
                            }
                            if (strchr(temp, '#')) {
                                dir_flag ^= 1;
                                Uart3_Printf("flag_dir %d\r\n", dir_flag);
                            }
                            if (strchr(temp, '$') && scc_flag == 0) {
                                scc_distance = (scc_distance < 3) ? scc_distance + 1 : 1;
                                Uart3_Printf("scc_distance %d\r\n", scc_distance);
                            }
                            char *ts = strrchr(temp, '|');
                            if (ts) {
                                uint32_t rpi_time_ms = (uint32_t)atoi(ts + 1);
                                int delay_ms = receive_tick + sync_offset - rpi_time_ms;  // ✅ 변경된 부분
                                Uart3_Printf("Delay: %d ms\r\n", delay_ms);
                            }

                            // 수신 데이터 UART 출력
                            sprintf(temp + strlen(temp), "\r\n");
                            //HAL_UART_Transmit(&huart3, (uint8_t *)temp, strlen(temp), HAL_MAX_DELAY);
                        }
                    }

                } while (netbuf_next(buf) >= 0);

                netbuf_delete(buf);
                osDelay(1);
            }

            Uart3_Printf("Client disconnected\r\n");
            netconn_close(newconn);
            netconn_delete(newconn);
        } else {
        	Uart3_Printf("netconn_accept() failed: %d\r\n", err);
            osDelay(1);
        }
    }
}



//void tcp_server_thread2(void const *argument)
//{
//    struct netconn *conn, *newconn;
//    err_t err;
//    uint32_t total_bytes_sent = 0; //추가
//    uint32_t start_tick = osKernelSysTick(); //추가
//    uint32_t total_bytes_received = 0;  // 추가 recv 바이트 누적 변수
//    uint32_t start_tick_recv = osKernelSysTick();  // 추가 수신 시작 시각
//
//    uint32_t lastSendTick = osKernelSysTick();
//
//    conn = netconn_new(NETCONN_TCP);
//    if (conn == NULL) {
//    	Uart3_Printf("netconn_new() failed\r\n");
//        return;
//    }
//
//    err = netconn_bind(conn, NULL, (u16_t)(uint32_t)argument);
//    if (err != ERR_OK) {
//    	Uart3_Printf("netconn_bind() failed: %d\r\n", err);
//        netconn_delete(conn);
//        return;
//    }
//
//    err = netconn_listen(conn);
//    if (err != ERR_OK) {
//    	Uart3_Printf("netconn_listen() failed: %d\r\n", err);
//        netconn_delete(conn);
//        return;
//    }
//
//    Uart3_Printf("TCP server started on port %d\r\n", (u16_t)(uint32_t)argument);
//
//    while (1) {
//        err = netconn_accept(conn, &newconn);
//        if (err == ERR_OK) {
//        	Uart3_Printf("Client connected\r\n");
//
//            struct netbuf *buf;
//            void *data;
//            u16_t len;
//            lastSendTick = osKernelSysTick();
//
//            while (1) {
//                // 수신 (non-blocking으로 구현하거나, 타임아웃 설정 권장)
//                err = netconn_recv(newconn, &buf);
//                if (err == ERR_OK) {
//                    do {
//                        netbuf_data(buf, &data, &len);
//                        total_bytes_received += len;
//
//                        char temp[256];
//                        if (len < sizeof(temp) - 2) {
//                            memcpy(temp, data, len);
//                            temp[len] = '\0';
//
//                            char *newline = strchr(temp, '\n');
//                            if (newline)
//                                *newline = '\0';
//
//                            offset_val = atoi(temp);
//
//                            if ( lka_1st_flag == 0 && offset_val == 150) {
//                            	lka_1st_flag = 1;
//                            	lka_flag = 0;
//                            }
//                            else lka_1st_flag = 0;
//
//                            Uart3_Printf("offset= %d\r\n", offset_val);
//                            HAL_UART_Transmit(&huart3, (uint8_t *)temp, strlen(temp), HAL_MAX_DELAY);
//                        }
//                    } while (netbuf_next(buf) >= 0);
//
//                    netbuf_delete(buf);
//                }
//
//                // 주기적인 송신 처리 (수신 여부와 무관하게) (이걸 바꿈)
////                if ((osKernelSysTick() - lastSendTick) >= 1000) {
////                    char send_buf[128];
////                    sprintf(send_buf, "speed:%d, accel:%d, brake:%d, scc_flag:%d, lka_flag:%d, scc_distance:%d, dir_flag:%d\r\n",
////                            (int)current_speed,
////                            accel_pedal_pressed,
////                            brake_pedal_pressed,
////                            scc_flag,
////                            lka_flag,
////                            scc_distance,
////                            dir_flag);
////                    netconn_write(newconn, send_buf, strlen(send_buf), NETCONN_COPY);
////                    lastSendTick = osKernelSysTick();
////                }
//                //위의 내용을 아래로
//
//                if ((osKernelSysTick() - lastSendTick) >= 1000) {
//                    char send_buf[128];
//                    int len = sprintf(send_buf,
//                            "speed:%d, accel:%d, brake:%d, scc_flag:%d, lka_flag:%d, scc_distance:%d, dir_flag:%d\r\n",
//                            (int)current_speed,
//                            accel_pedal_pressed,
//                            brake_pedal_pressed,
//                            scc_flag,
//                            lka_flag,
//                            scc_distance,
//                            dir_flag);
//
//                    netconn_write(newconn, send_buf, len, NETCONN_COPY);
//                    total_bytes_sent += len;  // 보낸 바이트 누적
//
//                    uint32_t now = osKernelSysTick();
//                    float elapsed = (now - start_tick) / 1000.0f;
//
//                    if (elapsed >= 5.0f) {  // 5초마다 속도 계산
//                        float speed_kbps = (total_bytes_sent / 1024.0f) / elapsed;
//                        Uart3_Printf("[TCP 송신 속도] %.2f KB/s (총 %lu Bytes, %.2f초)\r\n",
//                                    speed_kbps, total_bytes_sent, elapsed);
//
//                        // 초기화
//                        total_bytes_sent = 0;
//                        start_tick = now;
//                    }
//
//                    //----------
//                    // 수신 속도 계산
//                    float elapsed_recv = (now - start_tick_recv) / 1000.0f;
//                    if (elapsed_recv >= 5.0f) {
//                        float recv_speed_kbps = (total_bytes_received / 1024.0f) / elapsed_recv;
//                        Uart3_Printf("[TCP 수신 속도] %.2f KB/s (총 %lu Bytes, %.2f초)\r\n",
//                                    recv_speed_kbps, total_bytes_received, elapsed_recv);
//
//                        total_bytes_received = 0;
//                        start_tick_recv = now;
//                    }
//                    // ----------넣음
//
//
//
//                    lastSendTick = now;
//                }
// //여기까지
//
//                osDelay(1);
//
//                // 클라이언트 연결 확인 (필요 시 더 정밀하게 연결 확인 가능)
//                if (err != ERR_OK) {
//                	Uart3_Printf("Client recv error, closing\r\n");
//                    break;
//                }
//            }
//
//            Uart3_Printf("Client disconnected\r\n");
//            netconn_close(newconn);
//            netconn_delete(newconn);
//        } else {
//        	Uart3_Printf("netconn_accept() failed: %d\r\n", err);
//            osDelay(1);
//        }
//    }
//}

void tcp_server_thread2(void const *argument)
{
    struct netconn *conn, *newconn;
    err_t err;
    uint32_t total_bytes_sent = 0; //추가
    uint32_t start_tick = osKernelSysTick(); //추가
    uint32_t total_bytes_received = 0;  // 추가 recv 바이트 누적 변수
    uint32_t start_tick_recv = osKernelSysTick();  // 추가 수신 시작 시각

    uint32_t lastSendTick = osKernelSysTick();

    conn = netconn_new(NETCONN_TCP);
    if (conn == NULL) {
    	Uart3_Printf("netconn_new() failed\r\n");
        return;
    }

    err = netconn_bind(conn, NULL, (u16_t)(uint32_t)argument);
    if (err != ERR_OK) {
    	Uart3_Printf("netconn_bind() failed: %d\r\n", err);
        netconn_delete(conn);
        return;
    }

    err = netconn_listen(conn);
    if (err != ERR_OK) {
    	Uart3_Printf("netconn_listen() failed: %d\r\n", err);
        netconn_delete(conn);
        return;
    }

    Uart3_Printf("TCP server started on port %d\r\n", (u16_t)(uint32_t)argument);

    while (1) {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK) {
        	Uart3_Printf("Client connected\r\n");

            struct netbuf *buf;
            void *data;
            u16_t len;
            lastSendTick = osKernelSysTick();

            while (1) {
                // 수신 (non-blocking으로 구현하거나, 타임아웃 설정 권장)
                err = netconn_recv(newconn, &buf);
                if (err == ERR_OK) {
                    do {
                        netbuf_data(buf, &data, &len);
                        total_bytes_received += len;

                        char temp[256];
                        if (len < sizeof(temp) - 2) {
                            memcpy(temp, data, len);
                            temp[len] = '\0';

                            char *newline = strchr(temp, '\n');
                            if (newline)
                                *newline = '\0';

//                            offset_val = atoi(temp);

//                            char *cval1 = strstr(temp, "offset:");
//							char *cval2 = strstr(temp, "dir_flag:");
//
//							if (cval1 && cval2 && cval2 > cval1) {
//								char a_buf[32] = { 0 };
//								int len1 = cval2 - (cval1 + 7);
//								if (len1 > 0 && len1 < sizeof(a_buf)) {
//									strncpy(a_buf, cval1 + 7, len1);
//									a_buf[len1] = '\0';
//									offset_val = atoi(a_buf);
//
//								}
//
//								char b_buf[32] = { 0 };
//								strncpy(b_buf, cval2 + 9, sizeof(b_buf) - 1);
//								b_buf[sizeof(b_buf) - 1] = '\0';
//								dir_flag_pi = atoi(b_buf);
//								Uart3_Printf("offset= %d, dir_flag_pi= %d\r\n", offset_val, dir_flag_pi );
//							}

							char *cval1 = strstr(temp, "offset:");
							char *cval2 = strstr(temp, "dir_flag:");

							if (cval1 && cval2 && cval2 > cval1) {
							    char a_buf[32] = { 0 };
							    char b_buf[32] = { 0 };

							    // offset 파싱
							    strncpy(a_buf, cval1 + 7, cval2 - (cval1 + 7));
							    a_buf[cval2 - (cval1 + 7)] = '\0';  // 널 종료
							    offset_val = atoi(a_buf);

							    // dir_flag 파싱
							    strncpy(b_buf, cval2 + 9, sizeof(b_buf) - 1);  // 9글자 뒤부터 숫자
							    b_buf[sizeof(b_buf) - 1] = '\0';
							    dir_flag_pi = atoi(b_buf);

//							    Uart3_Printf("offset= %d, dir_flag_pi= %d\r\n", offset_val, dir_flag_pi);
							}

							// 방향지시등
                            if ( lka_1st_flag == 0 && offset_val == 200 && dir_flag_pi == 0 ) {
                            	lka_1st_flag = 1;
                            	lka_flag = 0;
                            }
                            // 차선인식이 안될경우
                            else if ( lka_2nd_flag == 0 && offset_val == 150 ){
                            	lka_2nd_flag = 1;
                            	lka_flag = 0;
                            }
                            else {
                            	lka_1st_flag = 0;
                            	lka_2nd_flag = 0;
                            }

//                            Uart3_Printf("offset= %d, dir_flag_pi= %d\r\n", offset_val, dir_flag_pi);
                            char uart_out[260];
                            snprintf(uart_out, sizeof(uart_out), "%s\r\n", temp);  // 개행 추가
                            //HAL_UART_Transmit(&huart3, (uint8_t *)uart_out, strlen(uart_out), HAL_MAX_DELAY);
                        }
                    } while (netbuf_next(buf) >= 0);

                    netbuf_delete(buf);
                }

                // 주기적인 송신 처리 (수신 여부와 무관하게) (이걸 바꿈)
//                if ((osKernelSysTick() - lastSendTick) >= 1000) {
//                    char send_buf[128];
//                    sprintf(send_buf, "speed:%d, accel:%d, brake:%d, scc_flag:%d, lka_flag:%d, scc_distance:%d, dir_flag:%d\r\n",
//                            (int)current_speed,
//                            accel_pedal_pressed,
//                            brake_pedal_pressed,
//                            scc_flag,
//                            lka_flag,
//                            scc_distance,
//                            dir_flag);
//                    netconn_write(newconn, send_buf, strlen(send_buf), NETCONN_COPY);
//                    lastSendTick = osKernelSysTick();
//                }
                //위의 내용을 아래로

                if ((osKernelSysTick() - lastSendTick) >= 1000) {
                    char send_buf[128];
                    int len = sprintf(send_buf,
                            "speed:%d, accel:%d, brake:%d, scc_flag:%d, lka_flag:%d, scc_distance:%d, dir_flag:%d\r\n",
                            (int)current_speed,
                            accel_pedal_pressed,
                            brake_pedal_pressed,
                            scc_flag,
                            lka_flag,
                            scc_distance,
                            dir_flag);

                    netconn_write(newconn, send_buf, len, NETCONN_COPY);
                    total_bytes_sent += len;  // 보낸 바이트 누적

                    uint32_t now = osKernelSysTick();
                    float elapsed = (now - start_tick) / 1000.0f;

                    if (elapsed >= 5.0f) {  // 5초마다 속도 계산
                        float speed_kbps = (total_bytes_sent / 1024.0f) / elapsed;
                        //Uart3_Printf("[TCP 송신 속도] %.2f KB/s (총 %lu Bytes, %.2f초)\r\n",
                                    //speed_kbps, total_bytes_sent, elapsed);

                        // 초기화
                        total_bytes_sent = 0;
                        start_tick = now;
                    }

                    //----------
                    // 수신 속도 계산
                    float elapsed_recv = (now - start_tick_recv) / 1000.0f;
                    if (elapsed_recv >= 5.0f) {
                        float recv_speed_kbps = (total_bytes_received / 1024.0f) / elapsed_recv;
                        //Uart3_Printf("[TCP 수신 속도] %.2f KB/s (총 %lu Bytes, %.2f초)\r\n",
                          //          recv_speed_kbps, total_bytes_received, elapsed_recv);

                        total_bytes_received = 0;
                        start_tick_recv = now;
                    }
                    // ----------넣음



                    lastSendTick = now;
                }
 //여기까지

                osDelay(1);

                // 클라이언트 연결 확인 (필요 시 더 정밀하게 연결 확인 가능)
                if (err != ERR_OK) {
                	Uart3_Printf("Client recv error, closing\r\n");
                    break;
                }
            }

            Uart3_Printf("Client disconnected\r\n");
            netconn_close(newconn);
            netconn_delete(newconn);
        } else {
        	Uart3_Printf("netconn_accept() failed: %d\r\n", err);
            osDelay(1);
        }
    }
}


//void tcp_client_send_task(void const *argument)
//{
//	uart_printf("HI\n");
//    const char *server_ip_str = "192.168.2.100";
//    uint16_t port = 5002;
//
//    struct netconn *conn; //conn : TCP연결객체
//    ip_addr_t server_ip; //연결할 서버의 IP 주소를 저장하는 구조체
//    err_t err;
//
//    ipaddr_aton(server_ip_str, &server_ip); // 문자열 IP → ip_addr_t 구조체로 변환
//
//    conn = netconn_new(NETCONN_TCP);
//    if (conn == NULL) {
//        uart_printf("Client netconn_new() failed\r\n");
//        osThreadTerminate(NULL);
//        return;
//    }
//
//    uart_printf("Connecting to server %s:%d...\r\n", server_ip_str, port);
//    err = netconn_connect(conn, &server_ip, port);
//    if (err != ERR_OK) {
//        uart_printf("Connection failed: %d\r\n", err);
//        netconn_delete(conn);
//        osThreadTerminate(NULL);
//        return;
//    }
//
//    uart_printf("Connected to Raspberry Pi server\r\n");
//
//    while (1)
//    {
//        char send_buf[128];
//        sprintf(send_buf, "speed:%.1f, accel:%d, brake:%d\r\n",
//                current_speed, accel_pedal_pressed, brake_pedal_pressed);
//
//        err = netconn_write(conn, send_buf, strlen(send_buf), NETCONN_COPY);
//        if (err != ERR_OK) {
//            uart_printf("Send failed: %d\r\n", err);
//            break; // 연결 문제 발생 시 loop 종료
//        }
//
//        osDelay(1000); // 1초 주기
//    }
//
//    uart_printf("Connection closed\r\n");
//    netconn_close(conn);
//    netconn_delete(conn);
//    osThreadTerminate(NULL);
//}
