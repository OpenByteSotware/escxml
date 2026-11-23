#include "FinalMachine.h"
#include <stdio.h>
int main(void){ESCX_StateMachine m;FinalData d;
ESCX_BYTE_T bits[ESCX_ACTIVE_BYTES(FinalMachine_STATE_COUNT)];d.value=0;
FinalMachine_Init(&m,&d,bits,(ESCX_BYTE_T)sizeof(bits),(ESCX_StartTimerFunction)0,
	(ESCX_CancelTimerFunction)0);if(ESCX_StateMachineStart(&m)!=ESCX_STATUS_PROCESSED)return 1;
if(!ESCX_StateMachineIsInState(&m,FM_Running))return 2;
if(ESCX_StateMachineSend(&m,FM_Go,(void*)0)!=ESCX_STATUS_PROCESSED)return 3;
if(!ESCX_StateMachineIsInState(&m,FM_TopFinal))return 4;if(!ESCX_StateMachineIsFinished(&m))return 5;
printf("final and done.state tests passed\n");return 0;}
