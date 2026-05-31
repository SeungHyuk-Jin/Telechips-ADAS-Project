#include "mcb.h"

uint32 mcbTaskId;
uint32 mcbTaskStk[512];

volatile uint8_t mcb_flag  = 0;
int pedal_input_enabled = 1;

void MCB_Task(void *pArg)
{
    (void)pArg;
    uint8 ucCh = 0;
    uint8 ucTxBufferIndex;
    while (1)
    {
        WDT_KickPing();
        uart_printf("mcb 작동\n모든 기능 정지\n");
        pedal_input_enabled = 0;
        scc_flag = 0;
        lfa_flag = 0;
        dir_flag = 0;
        
        brake_val = power_brake / 10.0f;
		accel_val = 0;
		if (current_speed == 0) {
            uart_printf("현재속도: 0 km/h, mcb 기능 종료\r\n");
            can_send_null(ucCh, &tempCanMsg);
            (void)CAN_SendMessage(ucCh, &tempCanMsg, &ucTxBufferIndex);
            mcb_flag = 0;
            pedal_input_enabled = 1;
            WDT_Stop();
			SAL_TaskSuspend(mcbTaskId);
		}

        SAL_TaskSleep(50);
    }
}
