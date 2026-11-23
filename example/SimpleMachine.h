#ifndef SIMPLE_MACHINE_H
#define SIMPLE_MACHINE_H
#include "StateMachine.h"

typedef struct SimpleData
{
    int entry_count;
    int transition_count;
    int mode;
} SimpleData;

ESCX_STATE_ENUM_BEGIN(SimpleMachine)
    ESCX_STATE_ENUM(SimpleMachine_Idle)
    ESCX_STATE_ENUM(SimpleMachine_Running)
    ESCX_STATE_ENUM(SimpleMachine_Error)
ESCX_STATE_ENUM_END(SimpleMachine)

ESCX_EVENT_ENUM_BEGIN(SimpleMachine)
    ESCX_EVENT_ENUM(SimpleMachine_StartEvent)
    ESCX_EVENT_ENUM(SimpleMachine_StopEvent)
ESCX_EVENT_ENUM_END(SimpleMachine)

ESCX_DECLARE_MACHINE(SimpleMachine);
#endif
