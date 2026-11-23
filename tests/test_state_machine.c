#include "SimpleMachine.h"
#include <stdio.h>

int main(void)
{
    ESCX_StateMachine machine;
    SimpleData data;
    ESCX_ProcessStatus status;
    data.entry_count = 0;
    data.transition_count = 0;
    SimpleMachine_Init(&machine, &data, (ESCX_StartTimerFunction)0, (ESCX_CancelTimerFunction)0);
    status = ESCX_StateMachineStart(&machine);
    if (status != ESCX_STATUS_PROCESSED) return 1;
    if (ESCX_StateMachineIsInState(&machine, SimpleMachine_Idle) == ESCX_FALSE) return 2;
    status = ESCX_StateMachineSend(&machine, SimpleMachine_StartEvent, (void *)0);
    if (status != ESCX_STATUS_PROCESSED) return 3;
    if (ESCX_StateMachineIsInState(&machine, SimpleMachine_Running) == ESCX_FALSE) return 4;
    status = ESCX_StateMachineSend(&machine, SimpleMachine_StopEvent, (void *)0);
    if (status != ESCX_STATUS_PROCESSED) return 5;
    if ((data.entry_count != 2) || (data.transition_count != 2)) return 6;
    ESCX_StateMachineStop(&machine);
    (void)printf("state machine tests passed\n");
    return 0;
}
