// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : main.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_MAIN_HEADER
#define MCU_BSP_MAIN_HEADER

// 공용 헤더
#include <app_cfg.h>
#include <bsp.h>
#include <debug.h>
#include <gpio.h> 
#include <sal_api.h>
#include <timer.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <wdt.h>

// 기능 헤더
#include <uss.h>
#include <scc.h>
#include <uart5.h>
#include <lfa.h>
#include <mcb.h>
#include <pedal.h>
#include <speed.h>

// CAN
#include <cant.h>

// spi
#include "gpsb.h" 

// 이더넷 ( W5500 - ioLiabrary)
#include "w5500_port.h"
#include "wizchip_conf.h"
#include "w5500.h"
#include "socket.h"


#if ( MCU_BSP_SUPPORT_APP_BASE == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define MAIN_UINT_MAX_NUM               (4294967295U)



/*
***************************************************************************************************
*                                             GLOBAL VARIABLES
***************************************************************************************************
*/
extern uint32                           gALiveMsgOnOff;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
extern void cmain
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_APP_BASE == 1 )

#endif  //  MCU_BSP_MAIN_HEADER

