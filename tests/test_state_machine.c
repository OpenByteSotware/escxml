#include "SimpleMachine.h"
#include <stdio.h>

static int TestModeRunning(void)
{
    ESCX_StateMachine machine;
    SimpleData data;
    data.entry_count = 0;
    data.transition_count = 0;
    data.mode = 0;
    SimpleMachine_Init(&machine, &data, (ESCX_StartTimerFunction)0, (ESCX_CancelTimerFunction)0);
    if (ESCX_StateMachineStart(&machine) != ESCX_STATUS_PROCESSED) return 1;
    if (ESCX_StateMachineSend(&machine, SimpleMachine_StartEvent, (void *)0) != ESCX_STATUS_PROCESSED) return 2;
    if (ESCX_StateMachineIsInState(&machine, SimpleMachine_Running) == ESCX_FALSE) return 3;
    if (ESCX_StateMachineSend(&machine, SimpleMachine_StopEvent, (void *)0) != ESCX_STATUS_PROCESSED) return 4;
    if (ESCX_StateMachineIsInState(&machine, SimpleMachine_Idle) == ESCX_FALSE) return 5;
    if ((data.entry_count != 2) || (data.transition_count != 2)) return 6;
    return 0;
}

static int TestModeError(void)
{
    ESCX_StateMachine machine;
    SimpleData data;
    data.entry_count = 0;
    data.transition_count = 0;
    data.mode = 1;
    SimpleMachine_Init(&machine, &data, (ESCX_StartTimerFunction)0, (ESCX_CancelTimerFunction)0);
    if (ESCX_StateMachineStart(&machine) != ESCX_STATUS_PROCESSED) return 10;
    if (ESCX_StateMachineSend(&machine, SimpleMachine_StartEvent, (void *)0) != ESCX_STATUS_PROCESSED) return 11;
    if (ESCX_StateMachineIsInState(&machine, SimpleMachine_Error) == ESCX_FALSE) return 12;
    return 0;
}

static int TestNoGuardMatch(void)
{
    ESCX_StateMachine machine;
    SimpleData data;
    data.entry_count = 0;
    data.transition_count = 0;
    data.mode = 99;
    SimpleMachine_Init(&machine, &data, (ESCX_StartTimerFunction)0, (ESCX_CancelTimerFunction)0);
    if (ESCX_StateMachineStart(&machine) != ESCX_STATUS_PROCESSED) return 20;
    if (ESCX_StateMachineSend(&machine, SimpleMachine_StartEvent, (void *)0) != ESCX_STATUS_PROCESSED) return 21;
    if (ESCX_StateMachineIsInState(&machine, SimpleMachine_Idle) == ESCX_FALSE) return 22;
    return 0;
}

int main(void)
{
    int result;
    result = TestModeRunning();
    if (result != 0) return result;
    result = TestModeError();
    if (result != 0) return result;
    result = TestNoGuardMatch();
    if (result != 0) return result;
    (void)printf("selector state machine tests passed\n");
    return 0;
}
