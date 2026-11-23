#include "ParallelMachine.h"
#include <stdio.h>
int main(void)
{
	ESCX_StateMachine m; 
	ParallelData d;
	ESCX_BYTE_T bits[ESCX_ACTIVE_BYTES(ParallelMachine_STATE_COUNT)]; 
	d.count = 0;
	ParallelMachine_Init(&m, &d, bits, (ESCX_BYTE_T)sizeof(bits), 
		(ESCX_StartTimerFunction)0, (ESCX_CancelTimerFunction)0);
	if (ESCX_StateMachineStart(&m) != ESCX_STATUS_PROCESSED)return 1;
	if (!ESCX_StateMachineIsInState(&m, PM_A1) || !ESCX_StateMachineIsInState(&m, PM_B1))return 2; 
	if (ESCX_StateMachineSend(&m, PM_Go, (void*)0) != ESCX_STATUS_PROCESSED)return 3; 
	if (!ESCX_StateMachineIsInState(&m, PM_A2) || !ESCX_StateMachineIsInState(&m, PM_B2))return 4; 
	if (ESCX_StateMachineSend(&m, PM_Reset, (void*)0) != ESCX_STATUS_PROCESSED)return 5;
	if (!ESCX_StateMachineIsInState(&m, PM_A1) || ESCX_StateMachineIsInState(&m, PM_Out))return 6; 
	printf("parallel conflict tests passed\n"); return 0;
}
