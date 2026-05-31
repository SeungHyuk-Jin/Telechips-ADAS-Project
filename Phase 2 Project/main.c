// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : main.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/
#if (MCU_BSP_SUPPORT_APP_BASE == 1)

# include <main.h>
#undef _WIZCHIP_IO_MODE_
#define _WIZCHIP_IO_MODE_ _WIZCHIP_IO_MODE_SPI_VDM_

// # if (APLT_LINUX_SUPPORT_SPI_DEMO == 1)
// #  include <spi_eccp.h>
// # endif
// # if (APLT_LINUX_SUPPORT_POWER_CTRL == 1)
// #  include <power_app.h>
// # endif
// # if (MCU_BSP_SUPPORT_APP_KEY == 1)
// #  include <key.h>
// # endif // ( MCU_BSP_SUPPORT_APP_KEY == 1 )

// # if (MCU_BSP_SUPPORT_APP_CONSOLE == 1)
// #  include <console.h>
// # endif // ( MCU_BSP_SUPPORT_APP_CONSOLE == 1 )

# if (MCU_BSP_SUPPORT_CAN_DEMO == 1)
#  include <can_demo.h>
# endif // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

# if (MCU_BSP_SUPPORT_APP_IDLE == 1)
#  include <idle.h>
# endif // ( MCU_BSP_SUPPORT_APP_IDLE == 1 )

// # if (MCU_BSP_SUPPORT_APP_SPI_LED == 1)
// #  include <spi_led.h>
// # endif // ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )

# if (MCU_BSP_SUPPORT_APP_FW_UPDATE == 1)
#  include "fwupdate.h"
# elif (MCU_BSP_SUPPORT_APP_FW_UPDATE_ECCP == 1)
#  include "fwupdate.h"
# endif

/*
***************************************************************************************************
*                                         GLOBAL VARIABLES
***************************************************************************************************
*/
uint32			gALiveMsgOnOff;
static uint32	gALiveCount;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void		Main_StartTask(void *pArg);
static void		AppTaskCreate(void);
static void		DisplayAliveLog(void);
static void		DisplayOTPInfo(void);

/*
***************************************************************************************************
*                                         FUNCTIONS
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          cmain
*
* This is the standard entry point for C code.
*
* Notes
*   It is assumed that your code will call main() once you have performed all necessary
*   initialization.
*
***************************************************************************************************
*/

// 함수 프로토타입 (필요한 경우)
void ECCP_Complete(uint32 uiCh, uint32 iEvent, void *pArg);

// 간단한 구현 예시
void ECCP_Complete(uint32 uiCh, uint32 iEvent, void *pArg)
{
    // 이벤트 종류에 따라 로그 출력만 하는 최소 구현
    uart_printf("[ECCP_Complete] ch=%u event=0x%08X\n", uiCh, iEvent);
    // 필요하면 상태 플래그 세팅, 신호/세마포어 등 처리 추가
}

void	cmain(void)
{
	static uint32		AppTaskStartID = 0;
	static uint32		AppTaskStartStk[1024];
	SALRetCode_t		err;
	SALMcuVersionInfo_t	versionInfo = {0, 0, 0, 0};

	(void)SAL_Init();
	BSP_PreInit(); /* Initialize basic BSP functions */
# if (MCU_BSP_SUPPORT_CAN_DEMO == 1)
	(void)CAN_DemoInitialize();
# endif // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )
	BSP_Init(); /* Initialize BSP functions */
	(void)SAL_GetVersion(&versionInfo);
	// mcu_printf("\n===============================\n");
	// mcu_printf("    MCU BSP Version: V%d.%d.%d\n", versionInfo.viMajorVersion,
	// 	versionInfo.viMinorVersion, versionInfo.viPatchVersion);
	// mcu_printf("-------------------------------\n");
	// DisplayOTPInfo();
	mcu_printf("===============================\n\n");
	UART_ExampleInit();

	// create the first app task...
	err = (SALRetCode_t)SAL_TaskCreate(&AppTaskStartID,
			(const uint8 *)"App Task Start", (SALTaskFunc)&Main_StartTask,
			&AppTaskStartStk[0], 1024, SAL_PRIO_APP_CFG + 3, NULL);
	if (err == SAL_RET_SUCCESS)
	{
		// start woring os.... never return from this function
		(void)SAL_OsStart();
	}
}

/*
***************************************************************************************************
*                                          Main_StartTask
*
* This is an example of a startup task.
*
* Notes
*   As mentioned in the book's text,
	you MUST initialize the ticker only once multitasking has
*   started.
*
*   1) The first line of code is used to prevent a compiler warning because 'pArg' is not used.
*      The compiler should not generate any code for this statement.
*
***************************************************************************************************
*/
static void	Main_StartTask(void *pArg)
{
	uint8_t	prev_scc_flag;
	uint8_t prev_dir_flag;
	uint8_t prev_lfa_flag;
	mcu_printf("hi\n");

	(void)pArg;
	(void)SAL_OsInitFuncs();
	timer_init_for_us();
	/* Service Init*/
	/* Create application tasks */
	uart5_init();
	SPI_MutexInit();
	WDT_Init(); // WatchDog 시간, 모드 설정

	PDM_Init();
    GPIO_PerichSel(GPIO_PERICH_SEL_PWMSEL_0, GPIO_PERICH_CH0); // A10을 PWM으로 설정


	// GPSB_Init(); // IRQ 등록 등 기본 설정
	// GPSB_Default_Init();
    // W5500_Init();      // wizchip 콜백 등록 + 네트워크 설정
    mcu_printf("W5500 initialized, entering main loop\n");

	AppTaskCreate();

    
	uart_printf("Start\n");
	while (1)
	{ /* Task body, always written as an infinite loop.       */
		if (scc_flag != prev_scc_flag)
		{
			if (scc_flag)
			{
				if (current_speed >= 30) {
					TARGET_SPEED = current_speed;
					SAFE_DISTANCE = (current_speed / 3.6) * (1 + 0.3 * (scc_distance - 1));
					BRAKE_DISTANCE = 5;
					uart_printf("SCC 활성화\n");
					uart_printf("현재속도 : %d.%02d km/h, 안전거리 단계 : %d 단계, 안전거리 : %d.%02d cm\n", (int)current_speed, (int)(((current_speed - (int)current_speed)) * 100), scc_distance, (int)SAFE_DISTANCE, (int)(((SAFE_DISTANCE - (int)SAFE_DISTANCE)) * 100));
					SAL_TaskResume(sccTaskId);
				} else {
					TARGET_SPEED = 30;
					SAFE_DISTANCE = (30 / 3.6) * (1 + 0.3 * (scc_distance - 1));
					BRAKE_DISTANCE = 5;
					uart_printf("SCC 활성화\n");
					uart_printf("현재속도 : 30.00 km/h, 안전거리 단계 : %d 단계, 안전거리 : %d.%02d\n", scc_distance, (int)SAFE_DISTANCE, (int)(((SAFE_DISTANCE - (int)SAFE_DISTANCE)) * 100));
					SAL_TaskResume(sccTaskId);		
				}
			}
			else
			{
				SAL_TaskSuspend(sccTaskId);
				uart_printf("SCC 비활성화\n");
			}
			prev_scc_flag = scc_flag;
		}

		if (dir_flag != prev_dir_flag) {
			if (dir_flag) {
				uart_printf("방향등 활성화\n");
				if (lfa_flag) {
					lfa_on_flag = 1;
					lfa_flag = !lfa_flag;
				}
			} else {
				uart_printf("방향등. 비활성화\n");
				if (lfa_on_flag == 1) {
					lfa_on_flag = 0;
					lfa_1st_flag = 0;
					lfa_flag = !lfa_flag;
				}
			}
			prev_dir_flag = dir_flag;
		}

		if (lfa_flag != prev_lfa_flag) {

		    if (lfa_flag && ( dir_flag == 0 ) && ( lfa_1st_flag == 0 ) && ( lfa_2nd_flag == 0 )&& ( offset_val != 150 )) {
		        uart_printf("LFA 활성화\n");
				SAL_TaskResume(lfaTaskId);
		    } else {
		        uart_printf("LFA 비활성화\n");
				SG90_SetAngle(90);
				offset_val = 50;
				SAL_TaskSuspend(lfaTaskId);
			}
			prev_lfa_flag = lfa_flag;
		}

		if (mcb_flag) {
			if(gWdtStatus==WDT_STS_INITIALIZED || gWdtStatus==WDT_STS_STOPPED){
				WDT_Start();
			}
			SAL_TaskResume(mcbTaskId);
		}

		SAL_TaskSleep(100);
	}
}

static void	AppTaskCreate(void)
{
// # if (APLT_LINUX_SUPPORT_SPI_DEMO == 1)
// 	ECCP_InitSPIManager();
// # endif
// # if (APLT_LINUX_SUPPORT_POWER_CTRL == 1)
// 	POWER_APP_StartDemo();
// # endif
// # if (MCU_BSP_SUPPORT_APP_CONSOLE == 1)
// 	CreateConsoleTask();
// # endif // ( MCU_BSP_SUPPORT_APP_CONSOLE == 1 )
// # if (MCU_BSP_SUPPORT_APP_KEY == 1)
// 	KEY_AppCreate();
// # endif // ( MCU_BSP_SUPPORT_APP_KEY == 1 )
# if (MCU_BSP_SUPPORT_CAN_DEMO == 1)
	CAN_DemoCreateApp();
# endif // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )
# if (MCU_BSP_SUPPORT_APP_FW_UPDATE == 1)
	CreateFWUDTask();
# elif (MCU_BSP_SUPPORT_APP_FW_UPDATE_ECCP == 1)
	CreateFWUDTask();
# endif
# if (MCU_BSP_SUPPORT_APP_IDLE == 1)
	IDLE_CreateTask();
# endif // ( MCU_BSP_SUPPORT_APP_IDLE == 1 )
// # if (MCU_BSP_SUPPORT_APP_SPI_LED == 1)
// 	SPILED_CreateAppTask();
// # endif // ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )

	// 1순위 : MCB ( 모든기능정지 - 속도 0 도달시 STOP 신호 )
	SAL_TaskCreate(&mcbTaskId, (const uint8 *)"MCB_Task",
		(SALTaskFunc)&MCB_Task, &mcbTaskStk[0], 512, SAL_PRIO_APP_CFG, NULL);
	SAL_TaskSuspend(mcbTaskId);

	// 2순위 : CAN ( MCB 데이터 값 받아오기 - 전달 STOP 신호 전달 )
	
	SAL_TaskCreate(&cantTaskId, (const uint8 *)"CANT_Task",
		(SALTaskFunc)&CANT_Task, &cantTaskStk[0], 512, SAL_PRIO_APP_CFG+1, NULL);

	// 3순위: 페달 ( current_speed 변경 ) , 초음파 ( shared_distance 갱신 ), SCC ( current_speed 변경 )
	SAL_TaskCreate(&pedalTaskId, (const uint8 *)"PEDAL_Task",
		(SALTaskFunc)&PEDAL_Task, &pedalTaskStk[0], 512, SAL_PRIO_APP_CFG+2, NULL);
	
	SAL_TaskCreate(&ultrasonicTaskId, (const uint8 *)"USS_Task",
		(SALTaskFunc)&USS_Task, &ultrasonicTaskStk[0], 512, SAL_PRIO_APP_CFG+2,
		NULL);

	SAL_TaskCreate(&sccTaskId, (const uint8 *)"SCC_Task",
		(SALTaskFunc)&SCC_Task, &sccTaskStk[0], 512, SAL_PRIO_APP_CFG+2, NULL);
	SAL_TaskSuspend(sccTaskId);
	
	// 4순위: Main ( flag로 태스크 on/off ) , 모터제어 ( 바퀴 속도 current_speed에 따라 변경 ), 
	// 차선유지 ( offset값을 받아 핸들 조작 - 속도 변경 x )
	SAL_TaskCreate(&lfaTaskId, (const uint8 *)"LFA_Task",
		(SALTaskFunc)&LFA_Task, &lfaTaskStk[0], 512, SAL_PRIO_APP_CFG+3, NULL);
	SAL_TaskSuspend(lfaTaskId);
		
	SAL_TaskCreate(&speedTaskId, (const uint8 *)"SPEED_Task",
		(SALTaskFunc)&SPEED_Task, &speedTaskStk[0], 512, SAL_PRIO_APP_CFG+3, NULL);

	// // 5순위:  TCP 0, 1, 2
	// SAL_TaskCreate(&ethTaskId, (const uint8 *)"ETH_Task",
	// 	(SALTaskFunc)&ETH1_Task, &ethTaskStk[0], 512, SAL_PRIO_APP_CFG+2, NULL);
	
	// SAL_TaskCreate(&eth2TaskId, (const uint8 *)"ETH2_Task",
	// 	(SALTaskFunc)&ETH2_Task, &eth2TaskStk[0], 512, SAL_PRIO_APP_CFG+3, NULL);
	
	// SAL_TaskCreate(&eth0TaskId, (const uint8 *)"ETH0_Task",
	// 	(SALTaskFunc)&ETH0_Task, &eth0TaskStk[0], 512, SAL_PRIO_APP_CFG+4, NULL);

}

static void	DisplayAliveLog(void)
{
	if (gALiveMsgOnOff != 0U)
	{
		mcu_printf("\n %d", gALiveCount);
		gALiveCount++;
		if (gALiveCount >= MAIN_UINT_MAX_NUM)
		{
			gALiveCount = 0;
		}
	}
	else
	{
		gALiveCount = 0;
	}
}

# define LDT1_AREA_ADDR 0xA1011800U
# define PMU_REG_ADDR 0xA0F28000U

static void	DisplayOTPInfo(void)
{
	volatile uint32	*ldt1Addr;
	volatile uint32	*chipNameAddr;
	volatile uint32	*remapAddr;
	volatile uint32	*hsmStatusAddr;
	uint32			chipName;
	uint32			dualBankVal;
	uint32			dual_bank;
	uint32			expandFlashVal;
	uint32			expand_flash;
	uint32			remap_mode;
	uint32			hsm_ready;

	chipName = 0;
	dualBankVal = 0;
	dual_bank = 0;
	expandFlashVal = 0;
	expand_flash = 0;
	remap_mode = 0;
	hsm_ready = 0;
	//----------------------------------------------------------------
	// OTP LDT1 Read
	// [11:0]Dual_Bank_Selection, [59:48]EXPAND_FLASH
	// Dual_Bank_Sel: [0xC0][11: 0] & [0xD0][11: 0] & [0xE0][11: 0] & [0xF0][11: 0]
	// EXPAND_FLASH : [0xC4][27:16] & [0xD4][27:16] & [0xE4][27:16] & [0xF4][27:16]
	// HwMC_PRG_FLS_LDT1: 0xA1011800
	ldt1Addr = (volatile uint32 *)(LDT1_AREA_ADDR + 0x00C0);
	chipNameAddr = (volatile uint32 *)(LDT1_AREA_ADDR + 0x0300);
	remapAddr = (volatile uint32 *)(PMU_REG_ADDR);
	hsmStatusAddr = (volatile uint32 *)(PMU_REG_ADDR + 0x0020);
	chipName = *chipNameAddr;
	chipName &= 0x000FFFFF;
	dualBankVal = ldt1Addr[0];
	expandFlashVal = ldt1Addr[1];
	dualBankVal &= ldt1Addr[4];
	expandFlashVal &= ldt1Addr[5];
	dualBankVal &= ldt1Addr[8];
	expandFlashVal &= ldt1Addr[9];
	dualBankVal &= ldt1Addr[12];
	expandFlashVal &= ldt1Addr[13];
	dualBankVal = (dualBankVal >> 0) & 0x0FFF;
	expandFlashVal = (expandFlashVal >> 16) & 0x0FFF;
	dual_bank = (dualBankVal == 0x0FFF) ? 0 : 1;
	// (single_bank : dual_bank)
	expand_flash = (expandFlashVal == 0x0000) ? 0 : 1;
	// (only_eFlash : use_extSNOR)
	remap_mode = remapAddr[0];
	mcu_printf("    CHIP   NAME  : %x\n", chipName);
	mcu_printf("    DUAL   BANK  : %d\n", dual_bank);
	mcu_printf("    EXPAND FLASH : %d\n", expand_flash);
	mcu_printf("    REMAP  MODE  : %d\n", (remap_mode >> 16));
	hsm_ready = hsmStatusAddr[0];
	hsm_ready = (hsm_ready >> 2) & 0x0001;
# if 0
    if(hsm_ready)
    {
        mcu_printf("    HSM    READY : %d\n",    hsm_ready);
    }
    else
    {
        while(hsm_ready != 1)
        {
            mcu_printf("    HSM    READY : %d\n",    hsm_ready);
            mcu_printf("    wait...\n");
            hsm_ready = (hsm_ready >> 2) & 0x0001;
        }
    }
# else
	mcu_printf("    HSM    READY : %d\n", hsm_ready);
# endif
}

#endif // ( MCU_BSP_SUPPORT_APP_BASE == 1 )
