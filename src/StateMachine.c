#include "StateMachine.h"
#include "AssertHandler.h"
#include <stddef.h>

static ESCX_ProcessStatus ESCX_Validate(const ESCX_StateMachine *machine)
{
    if ((machine == (const ESCX_StateMachine *)0) ||
        (machine->definition == (const ESCX_StateMachineDefinition *)0))
    {
        return ESCX_STATUS_INVALID_ARGUMENT;
    }
    return ESCX_STATUS_PROCESSED;
}

void ESCX_StateMachineInit(ESCX_StateMachine *machine, const ESCX_StateMachineDefinition *definition,
                          void *instance, ESCX_StartTimerFunction start_timer,
                          ESCX_CancelTimerFunction cancel_timer)
{
    ESCX_ASSERT_TRUE(machine != (ESCX_StateMachine *)0);
    ESCX_ASSERT_TRUE(definition != (const ESCX_StateMachineDefinition *)0);
    ESCX_ASSERT_TRUE(definition->states != (const ESCX_StateDescriptor *)0);
    ESCX_ASSERT_TRUE(definition->state_count > 1U);
    ESCX_ASSERT_TRUE(definition->initial_state > ESCX_NO_STATE);
    ESCX_ASSERT_TRUE((unsigned int)definition->initial_state < definition->state_count);
    machine->definition = definition;
    machine->instance = instance;
    machine->current_state = ESCX_NO_STATE;
    machine->current_event = ESCX_NO_EVENT;
    machine->running = ESCX_FALSE;
    machine->processing = ESCX_FALSE;
    machine->timer_id = ESCX_INVALID_TIMER_ID;
    machine->start_timer = start_timer;
    machine->cancel_timer = cancel_timer;
    ESCX_EventQueueInit(&machine->event_queue);
}

static void ESCX_EnterState(ESCX_StateMachine *machine, ESCX_BYTE_T state, void *event_data)
{
    const ESCX_StateDescriptor *descriptor;
    descriptor = &machine->definition->states[state];
    machine->current_state = state;
    if (descriptor->on_entry != (ESCX_StateFunction)0) descriptor->on_entry(machine, event_data);
    if (descriptor->on_state != (ESCX_StateFunction)0) descriptor->on_state(machine, event_data);
}

ESCX_ProcessStatus ESCX_StateMachineStart(ESCX_StateMachine *machine)
{
    ESCX_ProcessStatus status;
    status = ESCX_Validate(machine);
    if (status != ESCX_STATUS_PROCESSED) return status;
    if (machine->running != ESCX_FALSE) return ESCX_STATUS_IGNORED;
    machine->running = ESCX_TRUE;
    machine->current_event = ESCX_NO_EVENT;
    ESCX_EnterState(machine, machine->definition->initial_state, (void *)0);
    return ESCX_STATUS_PROCESSED;
}

void ESCX_StateMachineStop(ESCX_StateMachine *machine)
{
    const ESCX_StateDescriptor *descriptor;
    if ((machine == (ESCX_StateMachine *)0) || (machine->definition == (const ESCX_StateMachineDefinition *)0)) return;
    if ((machine->running != ESCX_FALSE) && (machine->current_state != ESCX_NO_STATE))
    {
        descriptor = &machine->definition->states[machine->current_state];
        if (descriptor->on_exit != (ESCX_StateFunction)0) descriptor->on_exit(machine, (void *)0);
    }
    machine->running = ESCX_FALSE;
    machine->processing = ESCX_FALSE;
    machine->current_state = ESCX_NO_STATE;
    machine->current_event = ESCX_NO_EVENT;
    ESCX_EventQueueInit(&machine->event_queue);
}

static const ESCX_Transition *ESCX_FindTransition(ESCX_StateMachine *machine, ESCX_BYTE_T event_id, void *event_data)
{
    unsigned int i;
    const ESCX_Transition *transition;
    for (i = 0U; i < machine->definition->transition_count; ++i)
    {
        transition = &machine->definition->transitions[i];
        if ((transition->source_state == machine->current_state) && (transition->event_id == event_id))
        {
            if ((transition->guard == (ESCX_GuardFunction)0) || (transition->guard(machine, event_data) != ESCX_FALSE))
            {
                return transition;
            }
        }
    }
    return (const ESCX_Transition *)0;
}

static ESCX_ProcessStatus ESCX_ProcessOne(ESCX_StateMachine *machine, ESCX_BYTE_T event_id, void *event_data)
{
    const ESCX_Transition *transition;
    const ESCX_StateDescriptor *old_state;
    if (event_id >= machine->definition->event_count) return ESCX_STATUS_INVALID_EVENT;
    machine->current_event = event_id;
    transition = ESCX_FindTransition(machine, event_id, event_data);
    if (transition == (const ESCX_Transition *)0) return ESCX_STATUS_IGNORED;
    old_state = &machine->definition->states[machine->current_state];
    if ((transition->target_state != machine->current_state) && (old_state->on_exit != (ESCX_StateFunction)0))
        old_state->on_exit(machine, event_data);
    if (transition->action != (ESCX_ActionFunction)0) transition->action(machine, event_data);
    if (transition->target_state != machine->current_state)
        ESCX_EnterState(machine, transition->target_state, event_data);
    else if (old_state->on_state != (ESCX_StateFunction)0)
        old_state->on_state(machine, event_data);
    return ESCX_STATUS_PROCESSED;
}

ESCX_ProcessStatus ESCX_StateMachineSend(ESCX_StateMachine *machine, ESCX_BYTE_T event_id, void *event_data)
{
    ESCX_ProcessStatus result;
    ESCX_ProcessStatus one_result;
    ESCX_QueuedEvent queued;
    result = ESCX_Validate(machine);
    if (result != ESCX_STATUS_PROCESSED) return result;
    if (machine->running == ESCX_FALSE) return ESCX_STATUS_NOT_RUNNING;
    if (event_id >= machine->definition->event_count) return ESCX_STATUS_INVALID_EVENT;
    if (machine->processing != ESCX_FALSE)
    {
        return (ESCX_EventQueuePush(&machine->event_queue, event_id, event_data) != ESCX_FALSE) ?
               ESCX_STATUS_PROCESSED : ESCX_STATUS_QUEUE_FULL;
    }
    machine->processing = ESCX_TRUE;
    result = ESCX_ProcessOne(machine, event_id, event_data);
    while (ESCX_EventQueuePop(&machine->event_queue, &queued) != ESCX_FALSE)
    {
        #if ESCX_ENABLE_EVENT_DATA
        one_result = ESCX_ProcessOne(machine, queued.event_id, queued.event_data);
#else
        one_result = ESCX_ProcessOne(machine, queued.event_id, (void *)0);
#endif
        if ((result == ESCX_STATUS_IGNORED) && (one_result != ESCX_STATUS_IGNORED)) result = one_result;
    }
    machine->processing = ESCX_FALSE;
    return result;
}

ESCX_ProcessStatus ESCX_StateMachineTimerExpired(ESCX_StateMachine *machine, int timer_id, void *event_data)
{
    if (machine == (ESCX_StateMachine *)0) return ESCX_STATUS_INVALID_ARGUMENT;
    if ((machine->timer_id == ESCX_INVALID_TIMER_ID) || (timer_id != machine->timer_id)) return ESCX_STATUS_IGNORED;
    return ESCX_StateMachineSend(machine, ESCX_TIMER_EVENT, event_data);
}

ESCX_BOOL_T ESCX_StateMachineIsInState(const ESCX_StateMachine *machine, ESCX_BYTE_T state_id)
{
    if (machine == (const ESCX_StateMachine *)0) return ESCX_FALSE;
    return ((machine->running != ESCX_FALSE) && (machine->current_state == state_id)) ? ESCX_TRUE : ESCX_FALSE;
}
