#include "StateMachine.h"
#include "AssertHandler.h"

static ESCX_ProcessStatus ESCX_Validate(const ESCX_StateMachine *m)
{
    if ((m == (const ESCX_StateMachine *)0) ||
        (m->definition == (const ESCX_StateMachineDefinition *)0))
    {
        return ESCX_STATUS_INVALID_ARGUMENT;
    }
    return ESCX_STATUS_PROCESSED;
}

void ESCX_StateMachineInit(ESCX_StateMachine *m, const ESCX_StateMachineDefinition *d,
                           void *instance, ESCX_StartTimerFunction start_timer,
                           ESCX_CancelTimerFunction cancel_timer)
{
    if ((m == (ESCX_StateMachine *)0) ||
        (d == (const ESCX_StateMachineDefinition *)0))
    {
        ESCX_ASSERT_TRUE(0);
        return;
    }
    m->definition = d;
    m->instance = instance;
    m->current_state = ESCX_NO_STATE;
    m->current_event = ESCX_NO_EVENT;
    m->running = ESCX_FALSE;
    m->processing = ESCX_FALSE;
#if ESCX_ENABLE_TIMERS
    m->timer_id = ESCX_INVALID_TIMER_ID;
    m->start_timer = start_timer;
    m->cancel_timer = cancel_timer;
#else
    (void)start_timer;
    (void)cancel_timer;
#endif
    ESCX_EventQueueInit(&m->event_queue);
}

static ESCX_StateFunction ESCX_ReadStateFunction(const ESCX_StateFunction *p)
{
#if defined(ESCX_PLATFORM_AVR) && (ESCX_PLATFORM_AVR != 0)
    ESCX_StateFunction result;
    (void)ESCX_ROM_COPY(&result, p, sizeof(result));
    return result;
#else
    return *p;
#endif
}

static ESCX_ActionFunction ESCX_ReadAction(const ESCX_ActionFunction *p)
{
#if defined(ESCX_PLATFORM_AVR) && (ESCX_PLATFORM_AVR != 0)
    ESCX_ActionFunction result;
    (void)ESCX_ROM_COPY(&result, p, sizeof(result));
    return result;
#else
    return *p;
#endif
}

static ESCX_SelectorFunction ESCX_ReadSelector(const ESCX_SelectorFunction *p)
{
#if defined(ESCX_PLATFORM_AVR) && (ESCX_PLATFORM_AVR != 0)
    ESCX_SelectorFunction result;
    (void)ESCX_ROM_COPY(&result, p, sizeof(result));
    return result;
#else
    return *p;
#endif
}

static void ESCX_Enter(ESCX_StateMachine *m, ESCX_BYTE_T state, void *data)
{
    const ESCX_StateDescriptor *descriptor;
    ESCX_StateFunction function;
    descriptor = &m->definition->states[state];
    m->current_state = state;
    function = ESCX_ReadStateFunction(&descriptor->on_entry);
    if (function != (ESCX_StateFunction)0) function(m, data);
    function = ESCX_ReadStateFunction(&descriptor->on_state);
    if (function != (ESCX_StateFunction)0) function(m, data);
}

ESCX_ProcessStatus ESCX_StateMachineStart(ESCX_StateMachine *m)
{
    if (ESCX_Validate(m) != ESCX_STATUS_PROCESSED) return ESCX_STATUS_INVALID_ARGUMENT;
    if (m->running != ESCX_FALSE) return ESCX_STATUS_IGNORED;
    if ((m->definition->initial_state == ESCX_NO_STATE) ||
        ((ESCX_UINT16_T)m->definition->initial_state >= m->definition->state_count))
    {
        ESCX_ASSERT_TRUE(0);
        return ESCX_STATUS_INVALID_ARGUMENT;
    }
    m->running = ESCX_TRUE;
    ESCX_Enter(m, m->definition->initial_state, (void *)0);
    return ESCX_STATUS_PROCESSED;
}

void ESCX_StateMachineStop(ESCX_StateMachine *m)
{
    ESCX_StateFunction function;
    if ((m == (ESCX_StateMachine *)0) ||
        (m->definition == (const ESCX_StateMachineDefinition *)0)) return;
    if ((m->running != ESCX_FALSE) &&
        (m->current_state != ESCX_NO_STATE) &&
        ((ESCX_UINT16_T)m->current_state < m->definition->state_count))
    {
        function = ESCX_ReadStateFunction(&m->definition->states[m->current_state].on_exit);
        if (function != (ESCX_StateFunction)0) function(m, (void *)0);
    }
    m->running = ESCX_FALSE;
    m->processing = ESCX_FALSE;
    m->current_state = ESCX_NO_STATE;
    m->current_event = ESCX_NO_EVENT;
    ESCX_EventQueueInit(&m->event_queue);
}

static ESCX_UINT16_T ESCX_FindSelector(const ESCX_StateMachine *m, ESCX_BYTE_T event_id)
{
    ESCX_UINT16_T base;
    ESCX_UINT16_T slot;
    if ((m->current_state == ESCX_NO_STATE) ||
        ((ESCX_UINT16_T)m->current_state >= m->definition->state_count))
    {
        return ESCX_NO_SELECTOR;
    }
    base = ESCX_ROM_READ_U16(&m->definition->state_base[m->current_state]);
    if (base > (ESCX_UINT16_T)(65535U - (ESCX_UINT16_T)event_id))
    {
        return ESCX_NO_SELECTOR;
    }
    slot = (ESCX_UINT16_T)(base + (ESCX_UINT16_T)event_id);
    if (slot >= m->definition->slot_count) return ESCX_NO_SELECTOR;
    if (ESCX_ROM_READ_U8(&m->definition->slot_owner[slot]) != m->current_state)
    {
        return ESCX_NO_SELECTOR;
    }
    return ESCX_ROM_READ_U16(&m->definition->slot_selector[slot]);
}

static ESCX_ProcessStatus ESCX_ExecuteTransition(ESCX_StateMachine *m,
                                                 ESCX_UINT16_T transition_id,
                                                 void *data)
{
    const ESCX_Transition *transition;
    ESCX_BYTE_T target;
    ESCX_ActionFunction action;
    ESCX_StateFunction function;
    if ((transition_id == ESCX_NO_TRANSITION) ||
        (transition_id >= m->definition->transition_count))
    {
        return ESCX_STATUS_IGNORED;
    }
    transition = &m->definition->transitions[transition_id];
    target = ESCX_ROM_READ_U8(&transition->target_state);
    if ((target == ESCX_NO_STATE) ||
        ((ESCX_UINT16_T)target >= m->definition->state_count))
    {
        ESCX_ASSERT_TRUE(0);
        return ESCX_STATUS_INVALID_ARGUMENT;
    }
    if (target != m->current_state)
    {
        function = ESCX_ReadStateFunction(&m->definition->states[m->current_state].on_exit);
        if (function != (ESCX_StateFunction)0) function(m, data);
    }
    action = ESCX_ReadAction(&transition->action);
    if (action != (ESCX_ActionFunction)0) action(m, data);
    if (target != m->current_state)
    {
        ESCX_Enter(m, target, data);
    }
    else
    {
        function = ESCX_ReadStateFunction(&m->definition->states[m->current_state].on_state);
        if (function != (ESCX_StateFunction)0) function(m, data);
    }
    return ESCX_STATUS_PROCESSED;
}

static ESCX_ProcessStatus ESCX_ProcessOne(ESCX_StateMachine *m,
                                          ESCX_BYTE_T event_id,
                                          void *data)
{
    ESCX_UINT16_T selector_id;
    ESCX_UINT16_T transition_id;
    ESCX_SelectorFunction selector;
    if (event_id >= m->definition->event_count) return ESCX_STATUS_INVALID_EVENT;
    m->current_event = event_id;
    selector_id = ESCX_FindSelector(m, event_id);
    if ((selector_id == ESCX_NO_SELECTOR) ||
        (selector_id >= m->definition->selector_count))
    {
        return ESCX_STATUS_IGNORED;
    }
    selector = ESCX_ReadSelector(&m->definition->selectors[selector_id]);
    if (selector == (ESCX_SelectorFunction)0)
    {
        ESCX_ASSERT_TRUE(0);
        return ESCX_STATUS_INVALID_ARGUMENT;
    }
    transition_id = selector(m, data);
    return ESCX_ExecuteTransition(m, transition_id, data);
}

ESCX_ProcessStatus ESCX_StateMachineSend(ESCX_StateMachine *m,
                                         ESCX_BYTE_T event_id,
                                         void *data)
{
    ESCX_ProcessStatus result;
    ESCX_ProcessStatus one;
    ESCX_QueuedEvent queued;
    result = ESCX_Validate(m);
    if (result != ESCX_STATUS_PROCESSED) return result;
    if (m->running == ESCX_FALSE) return ESCX_STATUS_NOT_RUNNING;
    if (event_id >= m->definition->event_count) return ESCX_STATUS_INVALID_EVENT;
    if (m->processing != ESCX_FALSE)
    {
        return (ESCX_EventQueuePush(&m->event_queue, event_id, data) != ESCX_FALSE) ?
               ESCX_STATUS_PROCESSED : ESCX_STATUS_QUEUE_FULL;
    }
    m->processing = ESCX_TRUE;
    result = ESCX_ProcessOne(m, event_id, data);
    while (ESCX_EventQueuePop(&m->event_queue, &queued) != ESCX_FALSE)
    {
#if ESCX_ENABLE_EVENT_DATA
        one = ESCX_ProcessOne(m, queued.event_id, queued.event_data);
#else
        one = ESCX_ProcessOne(m, queued.event_id, (void *)0);
#endif
        if ((result == ESCX_STATUS_IGNORED) && (one != ESCX_STATUS_IGNORED)) result = one;
    }
    m->processing = ESCX_FALSE;
    return result;
}

ESCX_ProcessStatus ESCX_StateMachineTimerExpired(ESCX_StateMachine *m,
                                                  int timer_id,
                                                  void *data)
{
#if ESCX_ENABLE_TIMERS
    if (m == (ESCX_StateMachine *)0) return ESCX_STATUS_INVALID_ARGUMENT;
    if ((m->timer_id == ESCX_INVALID_TIMER_ID) || (timer_id != m->timer_id))
    {
        return ESCX_STATUS_IGNORED;
    }
    return ESCX_StateMachineSend(m, ESCX_TIMER_EVENT, data);
#else
    (void)m;
    (void)timer_id;
    (void)data;
    return ESCX_STATUS_IGNORED;
#endif
}

ESCX_BOOL_T ESCX_StateMachineIsInState(const ESCX_StateMachine *m,
                                       ESCX_BYTE_T state)
{
    return ((m != (const ESCX_StateMachine *)0) &&
            (m->running != ESCX_FALSE) &&
            (m->current_state == state)) ? ESCX_TRUE : ESCX_FALSE;
}
