#include "SimpleMachine.h"

static void Simple_IdleEntry(ESCX_StateMachine *machine, void *event_data)
{
    SimpleData *data;
    (void)event_data;
    data = (SimpleData *)machine->instance;
    ++data->entry_count;
}
static ESCX_BOOL_T Simple_CanStart(ESCX_StateMachine *machine, void *event_data)
{
    (void)machine;
    (void)event_data;
    return ESCX_TRUE;
}
static void Simple_TransitionAction(ESCX_StateMachine *machine, void *event_data)
{
    SimpleData *data;
    (void)event_data;
    data = (SimpleData *)machine->instance;
    ++data->transition_count;
}

ESCX_STATES_BEGIN(SimpleMachine)
    ESCX_STATE((ESCX_StateFunction)0, (ESCX_StateFunction)0, (ESCX_StateFunction)0)
    ESCX_STATE(Simple_IdleEntry, (ESCX_StateFunction)0, (ESCX_StateFunction)0)
    ESCX_STATE((ESCX_StateFunction)0, (ESCX_StateFunction)0, (ESCX_StateFunction)0)
ESCX_STATES_END()

ESCX_TRANSITIONS_BEGIN(SimpleMachine)
    ESCX_TRANSITION(SimpleMachine_Idle, SimpleMachine_StartEvent, SimpleMachine_Running, Simple_CanStart, Simple_TransitionAction)
    ESCX_TRANSITION(SimpleMachine_Running, SimpleMachine_StopEvent, SimpleMachine_Idle, (ESCX_GuardFunction)0, Simple_TransitionAction)
ESCX_TRANSITIONS_END()

ESCX_DEFINE_MACHINE(SimpleMachine, SimpleMachine_Idle);
ESCX_IMPLEMENT_MACHINE(SimpleMachine)
