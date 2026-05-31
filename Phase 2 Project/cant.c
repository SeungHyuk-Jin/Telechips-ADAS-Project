#include "cant.h"

uint32 cantTaskId;
uint32 cantTaskStk[512];

void CANT_Task(void *pArg)
{
    ( void ) pArg;

    while( 1 )
    {
        ( void ) SAL_TaskSleep( 200 );
    }
}