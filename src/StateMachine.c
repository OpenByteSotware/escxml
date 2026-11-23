#include "StateMachine.h"
#include "AssertHandler.h"

static ESCX_ProcessStatus ESCX_Validate(const ESCX_StateMachine *m)
{
    if ((m == (const ESCX_StateMachine *)0) || (m->definition == (const ESCX_StateMachineDefinition *)0)) return ESCX_STATUS_INVALID_ARGUMENT;
    return ESCX_STATUS_PROCESSED;
}

void ESCX_StateMachineInit(ESCX_StateMachine *m, const ESCX_StateMachineDefinition *d, void *instance,
                           ESCX_StartTimerFunction start_timer, ESCX_CancelTimerFunction cancel_timer)
{
    if ((m == (ESCX_StateMachine *)0) ||
        (d == (const ESCX_StateMachineDefinition *)0))
    {
        ESCX_ASSERT_TRUE(0);
        return;
    }
    m->definition=d; m->instance=instance; m->current_state=ESCX_NO_STATE; m->current_event=ESCX_NO_EVENT;
    m->running=ESCX_FALSE; m->processing=ESCX_FALSE;
#if ESCX_ENABLE_TIMERS
    m->timer_id=ESCX_INVALID_TIMER_ID; m->start_timer=start_timer; m->cancel_timer=cancel_timer;
#else
    (void)start_timer; (void)cancel_timer;
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
static ESCX_GuardFunction ESCX_ReadGuard(const ESCX_GuardFunction *p)
{
#if defined(ESCX_PLATFORM_AVR) && (ESCX_PLATFORM_AVR != 0)
    ESCX_GuardFunction result;
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
static void ESCX_Enter(ESCX_StateMachine *m, ESCX_BYTE_T state, void *data)
{
    const ESCX_StateDescriptor *d=&m->definition->states[state]; ESCX_StateFunction f;
    m->current_state=state; f=ESCX_ReadStateFunction(&d->on_entry); if (f!=(ESCX_StateFunction)0) f(m,data);
    f=ESCX_ReadStateFunction(&d->on_state); if (f!=(ESCX_StateFunction)0) f(m,data);
}
ESCX_ProcessStatus ESCX_StateMachineStart(ESCX_StateMachine *m)
{
    if (ESCX_Validate(m)!=ESCX_STATUS_PROCESSED) return ESCX_STATUS_INVALID_ARGUMENT;
    if (m->running!=ESCX_FALSE) return ESCX_STATUS_IGNORED;
    m->running=ESCX_TRUE; ESCX_Enter(m,m->definition->initial_state,(void *)0); return ESCX_STATUS_PROCESSED;
}
void ESCX_StateMachineStop(ESCX_StateMachine *m)
{
    ESCX_StateFunction f;
    if ((m==(ESCX_StateMachine *)0)||(m->definition==(const ESCX_StateMachineDefinition *)0)) return;
    if ((m->running!=ESCX_FALSE)&&(m->current_state!=ESCX_NO_STATE)) {
        f=ESCX_ReadStateFunction(&m->definition->states[m->current_state].on_exit); if(f!=(ESCX_StateFunction)0) f(m,(void *)0);
    }
    m->running=ESCX_FALSE; m->processing=ESCX_FALSE; m->current_state=ESCX_NO_STATE; ESCX_EventQueueInit(&m->event_queue);
}

/* Fixed-cost row-displacement lookup: no state, event, or transition scan. */
static ESCX_UINT16_T ESCX_FindRange(const ESCX_StateMachine *m, ESCX_BYTE_T event_id)
{
    ESCX_UINT16_T base, slot;
    if ((m->current_state == ESCX_NO_STATE) ||
        ((ESCX_UINT16_T)m->current_state >= m->definition->state_count))
    {
        return ESCX_NO_RANGE;
    }
    base=ESCX_ROM_READ_U16(&m->definition->state_base[m->current_state]);
    slot=(ESCX_UINT16_T)(base+(ESCX_UINT16_T)event_id);
    if (slot>=m->definition->slot_count) return ESCX_NO_RANGE;
    if (ESCX_ROM_READ_U8(&m->definition->slot_owner[slot])!=m->current_state) return ESCX_NO_RANGE;
    return ESCX_ROM_READ_U16(&m->definition->slot_range[slot]);
}
static ESCX_ProcessStatus ESCX_ProcessOne(ESCX_StateMachine *m, ESCX_BYTE_T event_id, void *data)
{
    ESCX_UINT16_T range_id, first, i; ESCX_BYTE_T count, target; const ESCX_TransitionRange *r;
    const ESCX_Transition *t; ESCX_GuardFunction guard; ESCX_ActionFunction action; ESCX_StateFunction f;
    if(event_id>=m->definition->event_count) return ESCX_STATUS_INVALID_EVENT;
    m->current_event=event_id; range_id=ESCX_FindRange(m,event_id);
    if((range_id==ESCX_NO_RANGE)||(range_id>=m->definition->range_count)) return ESCX_STATUS_IGNORED;
    r=&m->definition->ranges[range_id]; first=ESCX_ROM_READ_U16(&r->first_transition); count=ESCX_ROM_READ_U8(&r->transition_count);
    if ((first >= m->definition->transition_count) ||
        ((ESCX_UINT16_T)count > (ESCX_UINT16_T)(m->definition->transition_count - first)))
    {
        ESCX_ASSERT_TRUE(0);
        return ESCX_STATUS_INVALID_ARGUMENT;
    }
    for(i=0U;i<(ESCX_UINT16_T)count;++i) {
        t=&m->definition->transitions[first+i]; guard=ESCX_ReadGuard(&t->guard);
        if((guard==(ESCX_GuardFunction)0)||(guard(m,data)!=ESCX_FALSE)) {
            target=ESCX_ROM_READ_U8(&t->target_state);
            if ((target == ESCX_NO_STATE) ||
                ((ESCX_UINT16_T)target >= m->definition->state_count))
            {
                ESCX_ASSERT_TRUE(0);
                return ESCX_STATUS_INVALID_ARGUMENT;
            }
            if(target!=m->current_state) { f=ESCX_ReadStateFunction(&m->definition->states[m->current_state].on_exit); if(f!=(ESCX_StateFunction)0) f(m,data); }
            action=ESCX_ReadAction(&t->action); if(action!=(ESCX_ActionFunction)0) action(m,data);
            if(target!=m->current_state) ESCX_Enter(m,target,data); else { f=ESCX_ReadStateFunction(&m->definition->states[m->current_state].on_state); if(f!=(ESCX_StateFunction)0) f(m,data); }
            return ESCX_STATUS_PROCESSED;
        }
    }
    return ESCX_STATUS_IGNORED;
}
ESCX_ProcessStatus ESCX_StateMachineSend(ESCX_StateMachine *m, ESCX_BYTE_T event_id, void *data)
{
    ESCX_ProcessStatus result,one; ESCX_QueuedEvent q;
    result=ESCX_Validate(m); if(result!=ESCX_STATUS_PROCESSED) return result;
    if(m->running==ESCX_FALSE) return ESCX_STATUS_NOT_RUNNING;
    if(event_id>=m->definition->event_count) return ESCX_STATUS_INVALID_EVENT;
    if(m->processing!=ESCX_FALSE) return ESCX_EventQueuePush(&m->event_queue,event_id,data)!=ESCX_FALSE?ESCX_STATUS_PROCESSED:ESCX_STATUS_QUEUE_FULL;
    m->processing=ESCX_TRUE; result=ESCX_ProcessOne(m,event_id,data);
    while(ESCX_EventQueuePop(&m->event_queue,&q)!=ESCX_FALSE) {
#if ESCX_ENABLE_EVENT_DATA
        one=ESCX_ProcessOne(m,q.event_id,q.event_data);
#else
        one=ESCX_ProcessOne(m,q.event_id,(void *)0);
#endif
        if((result==ESCX_STATUS_IGNORED)&&(one!=ESCX_STATUS_IGNORED)) result=one;
    }
    m->processing=ESCX_FALSE; return result;
}
ESCX_ProcessStatus ESCX_StateMachineTimerExpired(ESCX_StateMachine *m,int timer_id,void *data)
{
#if ESCX_ENABLE_TIMERS
    if(m==(ESCX_StateMachine *)0) return ESCX_STATUS_INVALID_ARGUMENT;
    if((m->timer_id==ESCX_INVALID_TIMER_ID)||(timer_id!=m->timer_id)) return ESCX_STATUS_IGNORED;
    return ESCX_StateMachineSend(m,ESCX_TIMER_EVENT,data);
#else
    (void)m;(void)timer_id;(void)data;return ESCX_STATUS_IGNORED;
#endif
}
ESCX_BOOL_T ESCX_StateMachineIsInState(const ESCX_StateMachine *m,ESCX_BYTE_T state)
{ return ((m!=(const ESCX_StateMachine *)0)&&(m->running!=ESCX_FALSE)&&(m->current_state==state))?ESCX_TRUE:ESCX_FALSE; }
