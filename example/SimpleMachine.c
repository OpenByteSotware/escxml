#include "SimpleMachine.h"

#define SIMPLE_TRANSITION_START_RUNNING 0U
#define SIMPLE_TRANSITION_START_ERROR   1U
#define SIMPLE_TRANSITION_STOP_IDLE     2U

static void IdleEntry(ESCX_StateMachine *machine, void *event_data)
{
    SimpleData *data;
    (void)event_data;
    data = (SimpleData *)machine->instance;
    ++data->entry_count;
}

static void TransitionAction(ESCX_StateMachine *machine, void *event_data)
{
    SimpleData *data;
    (void)event_data;
    data = (SimpleData *)machine->instance;
    ++data->transition_count;
}

ESCX_SELECTOR_BEGIN(Simple_SelectStart)
    SimpleData *data;
    (void)event_data;
    data = (SimpleData *)machine->instance;
    ESCX_SELECTOR_SWITCH(data->mode)
        ESCX_SELECTOR_CASE(0, SIMPLE_TRANSITION_START_RUNNING)
        ESCX_SELECTOR_CASE(1, SIMPLE_TRANSITION_START_ERROR)
        ESCX_SELECTOR_DEFAULT(ESCX_NO_TRANSITION)
    ESCX_SELECTOR_SWITCH_END()
ESCX_SELECTOR_END()

ESCX_SELECTOR_BEGIN(Simple_SelectStop)
    (void)machine;
    (void)event_data;
    ESCX_SELECTOR_RETURN(SIMPLE_TRANSITION_STOP_IDLE)
ESCX_SELECTOR_END()

ESCX_STATES_BEGIN(SimpleMachine)
    ESCX_STATE((ESCX_StateFunction)0, (ESCX_StateFunction)0, (ESCX_StateFunction)0)
    ESCX_STATE(IdleEntry, (ESCX_StateFunction)0, (ESCX_StateFunction)0)
    ESCX_STATE((ESCX_StateFunction)0, (ESCX_StateFunction)0, (ESCX_StateFunction)0)
    ESCX_STATE((ESCX_StateFunction)0, (ESCX_StateFunction)0, (ESCX_StateFunction)0)
ESCX_STATES_END()

ESCX_TRANSITIONS_BEGIN(SimpleMachine)
    ESCX_TRANSITION(SimpleMachine_Running, TransitionAction)
    ESCX_TRANSITION(SimpleMachine_Error, TransitionAction)
    ESCX_TRANSITION(SimpleMachine_Idle, TransitionAction)
ESCX_TRANSITIONS_END()

ESCX_SELECTORS_BEGIN(SimpleMachine)
    ESCX_SELECTOR(Simple_SelectStart)
    ESCX_SELECTOR(Simple_SelectStop)
ESCX_SELECTORS_END()

ESCX_STATE_BASE_BEGIN(SimpleMachine)
    ESCX_INDEX_U16(0U)
    ESCX_INDEX_U16(0U)
    ESCX_INDEX_U16(1U)
    ESCX_INDEX_U16(1U)
ESCX_STATE_BASE_END()

ESCX_SLOT_OWNER_BEGIN(SimpleMachine)
    ESCX_INDEX_U8(ESCX_NO_OWNER)
    ESCX_INDEX_U8(SimpleMachine_Idle)
    ESCX_INDEX_U8(ESCX_NO_OWNER)
    ESCX_INDEX_U8(SimpleMachine_Running)
ESCX_SLOT_OWNER_END()

ESCX_SLOT_SELECTOR_BEGIN(SimpleMachine)
    ESCX_INDEX_U16(ESCX_NO_SELECTOR)
    ESCX_INDEX_U16(0U)
    ESCX_INDEX_U16(ESCX_NO_SELECTOR)
    ESCX_INDEX_U16(1U)
ESCX_SLOT_SELECTOR_END()

ESCX_DEFINE_MACHINE(SimpleMachine, SimpleMachine_Idle);
ESCX_IMPLEMENT_MACHINE(SimpleMachine)
