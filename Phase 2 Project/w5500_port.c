#define _WIZCHIP_ W5500
#define _WIZCHIP_IO_MODE_ _WIZCHIP_IO_MODE_SPI_VDM_

#include "w5500_port.h"

// 이더넷 뮤텍스
SemaphoreHandle_t xSPIMutex;
SemaphoreHandle_t xWizchipMutex;

// 이더넷 태스크 핸들
uint32 eth0TaskId;
uint32 eth0TaskStk[512];

uint32 ethTaskId;
uint32 ethTaskStk[512];

uint32 eth2TaskId;
uint32 eth2TaskStk[512];

// 뮤텍스
void wiz_lock(void)   { xSemaphoreTake(xWizchipMutex, portMAX_DELAY); }
void wiz_unlock(void) { xSemaphoreGive(xWizchipMutex); }

// 시스템 초기화 시 한 번만 호출
void SPI_MutexInit(void)
{
    xSPIMutex = xSemaphoreCreateMutex();
    configASSERT(xSPIMutex);
    xWizchipMutex = xSemaphoreCreateMutex();
    configASSERT(xWizchipMutex);
}

/* SPI 전송 함수 */
static void SPI_Transfer(uint8_t ch, const uint8_t *txBuf, uint8_t *rxBuf, uint32_t len)
{
    /* 뮤텍스 획득 (무한 대기) */
    if (xSemaphoreTake(xSPIMutex, portMAX_DELAY) == pdTRUE) {
        SALRetCode_t ret = GPSB_Xfer((uint32)ch, txBuf, rxBuf, len, GPSB_XFER_MODE_WITHOUT_INTERRUPT);
        if (ret != SAL_RET_SUCCESS) {
            uart_printf("[SPI_Transfer] GPSB_Xfer failed: 0x%08X\n", ret);
        }
        /* 전송 끝나면 뮤텍스 해제 */
        xSemaphoreGive(xSPIMutex);
    }
}

// CS핀 조작하여 칩 셀렉트 타이밍 ( CS핀이 0(low)일때 셀렉트 가능 )
void wizchip_select(void)
{
    GPIO_Set(W5500_CS_GPIO, 0); 
}

void wizchip_deselect(void)
{
    GPIO_Set(W5500_CS_GPIO, 1);
}

// SPI 바이트 단위 콜백 (wizchip_conf에서 사용)
static uint8_t wizchip_spi_readbyte_cb(void)
{
    uint8_t tx = 0x00;
    uint8_t rx = 0x00;
    SPI_Transfer(W5500_SPI_CH, &tx, &rx, 1);
    return rx;
}

static void wizchip_spi_writebyte_cb(uint8_t wb)
{
    uint8_t dummy = 0;
    SPI_Transfer((uint8_t)W5500_SPI_CH, &wb, &dummy, 1);
}

// burst모드 사용시 사용하는 함수 ( 한 번에 여러 바이트 가져옴 ( len 길이 만큼 ))
static void wizchip_spi_writeburst_cb(uint8_t* pBuf, uint16_t len)
{
    SPI_Transfer(W5500_SPI_CH, pBuf, NULL, len);  // 쓰기니까 rx는 NULL
}

static void wizchip_spi_readburst_cb(uint8_t* pBuf, uint16_t len)
{
    static uint8_t tx_dummy[256];  // 충분히 넉넉한 길이
    if (len > sizeof(tx_dummy)) {
        uart_printf("[W5500 BURST] 요청 길이 초과: %d\n", len);
        return;
    }
    memset(tx_dummy, 0x00, len);
    SPI_Transfer(W5500_SPI_CH, tx_dummy, pBuf, len);
}

/// W5500 Port 초기화 함수
void W5500_Port_Init(void)
{

    // cs핀과 spi 콜백함수 등록
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
    reg_wizchip_spi_cbfunc(wizchip_spi_readbyte_cb, wizchip_spi_writebyte_cb);
    reg_wizchip_spiburst_cbfunc(wizchip_spi_readburst_cb, wizchip_spi_writeburst_cb);

    // 디버깅 용
    // uart_printf(">>> SPI burst read func: %p\n", WIZCHIP.IF.SPI._read_burst);
    // uart_printf(">>> SPI burst write func: %p\n", WIZCHIP.IF.SPI._write_burst);

    // 칩 내부 버퍼 크기 설정 ( 8소켓까지 존재 )
    uint8_t txsize[8] = {2,2,2,2,2,2,2,2};
    uint8_t rxsize[8] = {2,2,2,2,2,2,2,2};

    if (wizchip_init(txsize, rxsize) != 0)
    {
        uart_printf("[W5500] wizchip_init failed\n");
        return;
    }

    uart_printf("[W5500] wizchip_init OK\n");
}

// CS 핀 GPIO 설정
void W5500_GPIO_Init(void)
{
    GPIO_Config(W5500_CS_GPIO, GPIO_FUNC(0) | GPIO_OUTPUT);
    GPIO_Set(W5500_CS_GPIO, 1); /* Deselected */
}

// 네트워크 설정 ( IP, MAC 등등 )
void W5500_Init_DefaultNet(void)
{
    wiz_NetInfo netinfo = {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56},
        .ip =  {192, 168, 0, 28},
        .sn =  {255, 255, 255, 0},
        .gw =  {192, 168, 0, 8},
        .dns = {8, 8, 8, 8},
        .dhcp = NETINFO_STATIC
    };

    wizchip_setnetinfo(&netinfo);
    
    // 설정한 IP 출력 확인
    uart_printf("[W5500] IP: %d.%d.%d.%d\n",
        netinfo.ip[0], netinfo.ip[1], netinfo.ip[2], netinfo.ip[3]);
}

void GPSB_Default_Init(void){

    // GPSB 채널 열기 / 설정
    GPSBOpenParam_t openParam = {0};

    openParam.uiSdo        = w5500_gpsbport[0][GPSB_SDO];   // MOSI
    openParam.uiSdi        = w5500_gpsbport[0][GPSB_SDI];   // MISO
    openParam.uiSclk       = w5500_gpsbport[0][GPSB_SCLK];  // SCLK
    openParam.pDmaAddrTx   = (uint32 *)MPU_GetDMABaseAddress(); // DMA 버퍼
    openParam.pDmaAddrRx   = (uint32 *)((uint32 *)MPU_GetDMABaseAddress() + ECCP_GPSB_DMA_SIZE);
    openParam.uiDmaBufSize = ECCP_GPSB_DMA_SIZE;
    openParam.fbCallback   = ECCP_Complete;
    openParam.pArg         = NULL_PTR;
    openParam.uiIsSlave    = GPSB_MASTER_MODE; // 마스터 모드

    if (GPSB_Open(0, openParam) != SAL_RET_SUCCESS)
    {
        uart_printf("GPSB_Open failed!\n");
        while(1);
    }

    // 속도/bit-per-word/CS 설정
    (void)GPSB_SetSpeed(0, ECCP_GPSB_SPEED); // 예: 10MHz
    (void)GPSB_SetBpw(0, ECCP_GPSB_BPW);     // 예: 8bit
    // 반드시 logic level을 일관되게 설정
	GPSB_CsInit(0, w5500_gpsbport[0][GPSB_CS], 0);         // low active
	GPSB_CsActivate(0, w5500_gpsbport[0][GPSB_CS], 0);     // CS low
	GPSB_CsDeactivate(0, w5500_gpsbport[0][GPSB_CS], 0);   // CS high
}

void W5500_Init(void){

    // 기본 init 함수  
    W5500_Port_Init();
    W5500_Init_DefaultNet();

    // chip version 읽기 (W5500은 getVERSIONR)
    #if (_WIZCHIP_ == W5500)
        uint8_t ver = getVERSIONR();
    #else
        uint8_t ver = getVER();
    #endif
    
    // VERSION 디버깅용
    //uart_printf("[W5500 DEBUG] version reg: 0x%02X\n\n\n\n\n\n\n\n", ver);

    uint8_t test = WIZCHIP_READ(0x3900);
    uart_printf("TEST: 0x%02X\n\n\n\n\n\n\n", test);

    // PHY 링크가 올라오기를 잠시 기다림 (최대 약 1초)
    for (int i = 0; i < 10; ++i) {
        uint8_t phy = getPHYCFGR();
        if (phy & 0x01) break; // Link up 비트 확인
        SAL_TaskSleep(100);
    }

    // 현재 설정된 네트워크 정보 읽어서 출력 (디버그용)
    wiz_NetInfo current;
    wizchip_getnetinfo(&current);
    uart_printf("[W5500 DEBUG] IP : %d.%d.%d.%d\n",
        current.ip[0], current.ip[1], current.ip[2], current.ip[3]);
    uart_printf("[W5500 DEBUG] MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
        current.mac[0], current.mac[1], current.mac[2],
        current.mac[3], current.mac[4], current.mac[5]);

    // 필요시 duplex도 체크 가능
    uint8_t phy = getPHYCFGR();
    uart_printf("[W5500 DEBUG] PHYCFGR: 0x%02X (Link %s, Speed %s)\n",
        phy,
        (phy & (1 << 0)) ? "Up" : "Down",
        (phy & (1 << 2)) ? "100Mbps" : "10Mbps");
}

// 디버깅용 VERSION 체크1
uint8_t getVERSIONR_burst(void)
{
    uint8_t tx[3] = {
        (VERSIONR & 0x00FF0000) >> 16,
        (VERSIONR & 0x0000FF00) >> 8,
        (VERSIONR & 0x000000FF)
    };
    uint8_t rx[1] = {0};
    uint8_t dummy_tx[1] = {0x00};

    wizchip_select();
    SPI_Transfer(W5500_SPI_CH, tx, NULL, 3);          // 주소 전송
    SPI_Transfer(W5500_SPI_CH, dummy_tx, rx, 1);      // 응답 수신
    wizchip_deselect();

    return rx[0];
}

// 디버깅용 VERSION 체크2
uint8_t read_version_raw(void)
{
    uint8_t tx[4];
    uint8_t rx[4];
    uint16_t addr = 0x0039; // VERSIONR address
    uint8_t control = 0x00; // Read, Common Register Block, OM=0

    tx[0] = (addr >> 8) & 0xFF;
    tx[1] = addr & 0xFF;
    tx[2] = control;
    tx[3] = 0x00; // dummy read

    uart_printf("SPI TX: %02X %02X %02X %02X\n", tx[0], tx[1], tx[2], tx[3]);

    wizchip_select();
    SPI_Transfer(W5500_SPI_CH, tx, rx, 4);
    wizchip_deselect();
    

    uart_printf("SPI RX: %02X %02X %02X %02X\n", rx[0], rx[1], rx[2], rx[3]);

    uint8_t ver = getVERSIONR();
    uart_printf("[W5500 DEBUG] version reg: 0x%02X\n\n\n\n\n\n\n", ver);

    return rx[3];
}

// 이더넷 태스크0 - 스위치값 
void ETH0_Task(void *pArg)
{
    (void)pArg;
    int8_t sock;
    uint8_t rx_buffer[128];

    // TCP 서버용 소켓 생성 (포트 5001)
    wiz_lock();
    sock = socket(SOCKET_CH0, Sn_MR_TCP, TCP_SERVER_PORT0, 0);
    wiz_unlock();
    if (sock < 0) {
        uart_printf("Server0 Socket 생성 실패: %d\r\n", sock);
        vTaskDelete(NULL);
    }

    // listen 모드 진입
    wiz_lock();
    if (listen(sock) != SOCK_OK) {
        wiz_unlock();
        uart_printf("listen 실패\r\n");
        close(sock);
        vTaskDelete(NULL);
    }
    wiz_unlock();
    uart_printf("TCP 서버 시작 → 포트 %d\n", TCP_SERVER_PORT0);

    for (;;) {
        uint8_t status;
        wiz_lock();
        status = getSn_SR(sock);
        wiz_unlock();

        if (status == SOCK_ESTABLISHED) {
            uart_printf("서버0 클라이언트 연결됨\r\n");

            // 연결 유지 중 100ms마다 recv 처리
            while (1) {
                wiz_lock();
                status = getSn_SR(sock);
                wiz_unlock();
                if (status != SOCK_ESTABLISHED) break;

                // recv 처리 (있으면 읽어서 에코)
                wiz_lock();
                uint16_t avail = getSn_RX_RSR(sock);
                wiz_unlock();
                if (avail > 0) {
                    if (avail > sizeof(rx_buffer)) avail = sizeof(rx_buffer);
                    wiz_lock();
                    int32_t recvd = recv(sock, rx_buffer, avail);
                    wiz_unlock();
                    if (recvd > 0) {
                        uart_printf("서버0 수신 %ld 바이트: ", recvd);
                        for (int i = 0; i < recvd; i++) {
                            uart_printf("%c", rx_buffer[i]);
                        }
                        uart_printf("\n");
                        char cmd = rx_buffer[0];
                        // 특수 문자 플래그 처리
                        if (cmd == '!') {
                            scc_flag ^= 1;
                            uart_printf("flag_scc %d\n", scc_flag);
                        }
                        if (cmd == '@') {
                        	if ( dir_flag == 0 ){
                        		lfa_flag ^= 1;
                        		uart_printf("flag_lfa %d\n", lfa_flag);
                        	}
                        }
                        if (cmd == '#') {
                            dir_flag ^= 1;
                            uart_printf("flag_dir %d\n", dir_flag);
                        }
                        if ((cmd == '$') && (scc_flag == 0)) {
                            scc_distance = (scc_distance < 3) ? scc_distance + 1 : 1;
                            uart_printf("scc_distance %d\n", scc_distance);
                        } 
                    }
                }
                // 100ms 대기
                SAL_TaskSleep(100);
            }

            uart_printf("서버0 클라이언트 연결 종료\r\n");
            wiz_lock();
            disconnect(sock);
            wiz_unlock();

            wiz_lock();
            if (listen(sock) != SOCK_OK) {
                wiz_unlock();
                uart_printf("서버0 listen 재진입 실패\r\n");
                break;
            }
            wiz_unlock();
        }
        else if (status == SOCK_CLOSED) {
            wiz_lock();
            listen(sock);
            wiz_unlock();
        }

        SAL_TaskSleep(200);
    }

    // 에러 발생 시 소켓 닫고 태스크 삭제
    wiz_lock();
    close(sock);
    wiz_unlock();
    vTaskDelete(NULL);
}

void ETH1_Task(void *pArg)
{
    (void)pArg;
    int8_t sock;
    uint8_t rx_buffer[128];

    TickType_t lastTick = xTaskGetTickCount();
    const TickType_t interval = 500;   // 1000틱 = 1초

    // W5500 메모리 할당 (각 소켓 Tx/Rx 2KB)
    {
        uint8_t txMem[8] = {2,2,2,2,2,2,2,2};
        uint8_t rxMem[8] = {2,2,2,2,2,2,2,2};
        wiz_lock();
        wizchip_init(txMem, rxMem);
        wiz_unlock();
    }

    // TCP 서버용 소켓 생성 (포트 5002)
    wiz_lock();
    sock = socket(SOCKET_CH, Sn_MR_TCP, TCP_SERVER_PORT, 0);
    wiz_unlock();
    if (sock < 0) {
        uart_printf("Server Socket 생성 실패: %d\r\n", sock);
        vTaskDelete(NULL);
    }

    // listen 모드 진입
    wiz_lock();
    if (listen(sock) != SOCK_OK) {
        wiz_unlock();
        uart_printf("listen 실패\r\n");
        close(sock);
        vTaskDelete(NULL);
    }
    wiz_unlock();
    uart_printf("TCP 서버 시작 → 포트 %d\n", TCP_SERVER_PORT);

    for (;;) {
        uint8_t status;
        wiz_lock();
        status = getSn_SR(sock);
        wiz_unlock();

        if (status == SOCK_ESTABLISHED) {
            uart_printf("서버1 클라이언트 연결됨\r\n");

            // 연결 유지 중 100ms마다 recv 처리 + 무조건 send
            while (1) {
                wiz_lock();
                status = getSn_SR(sock);
                wiz_unlock();
                if (status != SOCK_ESTABLISHED) break;

                TickType_t now   = xTaskGetTickCount();
                TickType_t delta = now - lastTick;
                
                if (delta >= interval)
                {
                    {
                        lastTick = now;

                        uint8_t send_buf[128];
                        uint16_t len = sprintf((char*)send_buf,
                            "speed:%d, accel:%d, brake:%d, scc_flag:%d, lfa_flag:%d, scc_distance:%d, dir_flag:%d\r\n",
                            (int)current_speed,
                            accel_pedal_pressed,
                            brake_pedal_pressed,
                            scc_flag,
                            lfa_flag,
                            scc_distance,
                            dir_flag);

                        wiz_lock();
                        send(sock, send_buf, len);
                        wiz_unlock();
                        uart_printf("송신: %s", send_buf);
                    }
                }

                // 100ms 대기
                SAL_TaskSleep(100);
            }

            uart_printf("서버1 클라이언트 연결 종료\r\n");
            wiz_lock();
            disconnect(sock);
            wiz_unlock();

            wiz_lock();
            if (listen(sock) != SOCK_OK) {
                wiz_unlock();
                uart_printf("서버1 listen 재진입 실패\r\n");
                break;
            }
            wiz_unlock();
        }
        else if (status == SOCK_CLOSED) {
            wiz_lock();
            listen(sock);
            wiz_unlock();
        }

        SAL_TaskSleep(200);
    }

    // 에러 발생 시 소켓 닫고 태스크 삭제
    wiz_lock();
    close(sock);
    wiz_unlock();
    vTaskDelete(NULL);
}

void ETH2_Task(void *pArg)
{
    (void)pArg;
    int8_t sock;
    uint8_t rx_buffer[128];

    TickType_t lastTick = xTaskGetTickCount();
    const TickType_t interval = 500;   // 1000틱 = 1초

    // TCP 서버용 소켓 생성 (포트 5003)
    wiz_lock();
    sock = socket(SOCKET_CH2, Sn_MR_TCP, TCP_SERVER_PORT2, 0);
    wiz_unlock();
    if (sock < 0) {
        uart_printf("Server2 Socket 생성 실패: %d\r\n", sock);
        vTaskDelete(NULL);
    }

    // listen 모드 진입
    wiz_lock();
    if (listen(sock) != SOCK_OK) {
        wiz_unlock();
        uart_printf("Server2 listen 실패\r\n");
        close(sock);
        vTaskDelete(NULL);
    }
    wiz_unlock();
    uart_printf("Server2 시작 → 포트 %d\n", TCP_SERVER_PORT2);

    for (;;) {
        uint8_t status;
        wiz_lock();
        status = getSn_SR(sock);
        wiz_unlock();

        if (status == SOCK_ESTABLISHED) {
            uart_printf("서버2 클라이언트 연결됨\r\n");

            while (1) {
                wiz_lock();
                status = getSn_SR(sock);
                wiz_unlock();
                if (status != SOCK_ESTABLISHED) break;

                uint16_t avail;
                wiz_lock();
                avail = getSn_RX_RSR(sock);
                wiz_unlock();

                if (avail > 0) {
                    if (avail > sizeof(rx_buffer)) avail = sizeof(rx_buffer);

                    int32_t recvd;
                    wiz_lock();
                    recvd = recv(sock, rx_buffer, avail);
                    wiz_unlock();

                    if (recvd > 0) {
                        if (recvd >= (int)sizeof(rx_buffer)) recvd = sizeof(rx_buffer) - 1;
                        rx_buffer[recvd] = '\0';  // C-string 보장

                        uart_printf("서버2 수신 %ld 바이트: %s\r\n", recvd, rx_buffer);

                        char *p;
                        long v;

                        // offset 파싱
                        p = strstr((char*)rx_buffer, "offset:");
                        if (p) {
                            v = strtol(p + 7, NULL, 10);
                            offset_val = (int)v;
                        }

                        // dir_flag 파싱
                        p = strstr((char*)rx_buffer, "dir_flag:");
                        if (p) {
                            v = strtol(p + 9, NULL, 10);
                            dir_flag_ai = (int)v;
                        }

                        uart_printf("offset=%d, dir_flag=%d \n",
                                    offset_val, dir_flag_ai);

                        // 기존 LFA 로직 유지
                        if ( lfa_1st_flag == 0 && offset_val == 200 && dir_flag_ai == 0 ) {
                            lfa_1st_flag = 1;
                            lfa_flag = 0;
                        } else if ( lfa_2nd_flag == 0 && offset_val == 150 ){
                            lfa_2nd_flag = 1;
                            lfa_flag = 0;
                        } else {
                            lfa_1st_flag = 0;
                            lfa_2nd_flag = 0;
                        }
                    }
                }

                TickType_t now   = xTaskGetTickCount();
                TickType_t delta = now - lastTick;
                
                if (delta >= interval)
                {
                    {
                        lastTick = now;

                        uint8_t send_buf[128];
                        uint16_t len = sprintf((char*)send_buf,
                            "lfa_flag:%d, dir_flag:%d\r\n",
                            lfa_flag,
                            dir_flag);

                        wiz_lock();
                        send(sock, send_buf, len);
                        wiz_unlock();
                        uart_printf("송신: %s", send_buf);
                    }
                }
                SAL_TaskSleep(1);
            }

            uart_printf("서버2 클라이언트 연결 종료\r\n");
            wiz_lock();
            disconnect(sock);
            wiz_unlock();

            wiz_lock();
            if (listen(sock) != SOCK_OK) {
                wiz_unlock();
                uart_printf("서버2 listen 재진입 실패\r\n");
                break;
            }
            wiz_unlock();
        }
        else if (status == SOCK_CLOSED) {
            wiz_lock();
            listen(sock);
            wiz_unlock();
        }

        SAL_TaskSleep(500);
    }

    // 에러 발생 시 소켓 닫고 태스크 삭제
    wiz_lock();
    close(sock);
    wiz_unlock();
    vTaskDelete(NULL);
}
