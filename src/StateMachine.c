#include "StateMachine.h"
#include "AssertHandler.h"
#include <stddef.h>
#include <string.h>
#define OFFSETOF_STATE(member) ((unsigned int)offsetof(ESCX_StateDescriptor,member))
static ESCX_StateFunction ReadStateFn(const ESCX_StateFunction*p){
#if defined(ESCX_PLATFORM_AVR)&&(ESCX_PLATFORM_AVR!=0)
 ESCX_StateFunction r;(void)ESCX_ROM_COPY(&r,p,sizeof(r));return r;
#else
 return *p;
#endif
}
static ESCX_ActionFunction ReadAction(const ESCX_ActionFunction*p){
#if defined(ESCX_PLATFORM_AVR)&&(ESCX_PLATFORM_AVR!=0)
 ESCX_ActionFunction r;(void)ESCX_ROM_COPY(&r,p,sizeof(r));return r;
#else
 return *p;
#endif
}
static ESCX_SelectorFunction ReadSelector(const ESCX_SelectorFunction*p){
#if defined(ESCX_PLATFORM_AVR)&&(ESCX_PLATFORM_AVR!=0)
 ESCX_SelectorFunction r;(void)ESCX_ROM_COPY(&r,p,sizeof(r));return r;
#else
 return *p;
#endif
}
static ESCX_BYTE_T StateU8(const ESCX_StateMachine*m,ESCX_BYTE_T id,unsigned int off){return ESCX_ROM_READ_U8(((const ESCX_BYTE_T*)&m->definition->states[id])+off);}
static ESCX_UINT16_T StateU16(const ESCX_StateMachine*m,ESCX_BYTE_T id,unsigned int off){return ESCX_ROM_READ_U16((const ESCX_UINT16_T*)(((const ESCX_BYTE_T*)&m->definition->states[id])+off));}
static ESCX_BYTE_T Parent(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(parent_state));}
static ESCX_BYTE_T Initial(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(initial_child));}
static ESCX_BYTE_T Type(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(state_type));}
static ESCX_BYTE_T DoneEvent(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(done_event));}
static ESCX_UINT16_T EventlessSelector(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU16(m,s,OFFSETOF_STATE(eventless_selector));}
static void SetActive(ESCX_StateMachine*m,ESCX_BYTE_T s,ESCX_BOOL_T on){ESCX_BYTE_T i,mask;i=(ESCX_BYTE_T)(s>>3);mask=(ESCX_BYTE_T)(1U<<(s&7U));if(i>=m->active_byte_count)return;if(on)m->active_bits[i]=(ESCX_BYTE_T)(m->active_bits[i]|mask);else m->active_bits[i]=(ESCX_BYTE_T)(m->active_bits[i]&((ESCX_BYTE_T)~mask));}
static ESCX_BOOL_T IsActive(const ESCX_StateMachine*m,ESCX_BYTE_T s){ESCX_BYTE_T i,mask;i=(ESCX_BYTE_T)(s>>3);if(i>=m->active_byte_count)return ESCX_FALSE;mask=(ESCX_BYTE_T)(1U<<(s&7U));return((m->active_bits[i]&mask)!=0U)?ESCX_TRUE:ESCX_FALSE;}
static ESCX_BOOL_T AddAtomic(ESCX_StateMachine*m,ESCX_BYTE_T s){ESCX_BYTE_T i;for(i=0U;i<m->active_atomic_count;++i)if(m->active_atomic_states[i]==s)return ESCX_TRUE;if(m->active_atomic_count>=ESCX_MAX_ACTIVE_ATOMIC_STATES)return ESCX_FALSE;m->active_atomic_states[m->active_atomic_count++]=s;return ESCX_TRUE;}
static void RemoveAtomic(ESCX_StateMachine*m,ESCX_BYTE_T s){ESCX_BYTE_T i;for(i=0U;i<m->active_atomic_count;++i){if(m->active_atomic_states[i]==s){--m->active_atomic_count;m->active_atomic_states[i]=m->active_atomic_states[m->active_atomic_count];return;}}}
ESCX_BOOL_T ESCX_StateMachineIsInState(const ESCX_StateMachine*m,ESCX_BYTE_T s){if((m==(const ESCX_StateMachine*)0)||(m->running==ESCX_FALSE)||(s==ESCX_NO_STATE))return ESCX_FALSE;return IsActive(m,s);}
ESCX_BOOL_T ESCX_StateMachineIsFinished(const ESCX_StateMachine*m){return((m!=(const ESCX_StateMachine*)0)&&(m->finished!=ESCX_FALSE))?ESCX_TRUE:ESCX_FALSE;}
static void CallState(ESCX_StateMachine*m,ESCX_BYTE_T s,void*d,int which){const ESCX_StateDescriptor*x;ESCX_StateFunction f;x=&m->definition->states[s];f=(which==0)?ReadStateFn(&x->on_entry):((which==1)?ReadStateFn(&x->on_exit):ReadStateFn(&x->on_state));if(f!=(ESCX_StateFunction)0)f(m,d);}
static ESCX_ProcessStatus NotifyFinalEntered(ESCX_StateMachine*m,ESCX_BYTE_T final_state);
static ESCX_UINT16_T FirstChild(const ESCX_StateMachine*m,ESCX_BYTE_T s){return ESCX_ROM_READ_U16(&m->definition->states[s].first_child_index);}
static ESCX_BYTE_T ChildCount(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(child_count));}
static ESCX_ProcessStatus EnterStateTree(ESCX_StateMachine*m,ESCX_BYTE_T s,void*d,ESCX_BYTE_T depth){ESCX_BYTE_T type,count,child;ESCX_UINT16_T first,i;if(depth>ESCX_MAX_HIERARCHY_DEPTH)return ESCX_STATUS_HIERARCHY_LIMIT;if((s==ESCX_NO_STATE)||((ESCX_UINT16_T)s>=m->definition->state_count))return ESCX_STATUS_INVALID_ARGUMENT;if(IsActive(m,s)==ESCX_FALSE){SetActive(m,s,ESCX_TRUE);CallState(m,s,d,0);CallState(m,s,d,2);}type=Type(m,s);if(type==ESCX_STATE_COMPOUND){child=Initial(m,s);return EnterStateTree(m,child,d,(ESCX_BYTE_T)(depth+1U));}if(type==ESCX_STATE_PARALLEL){first=FirstChild(m,s);count=ChildCount(m,s);if((ESCX_UINT16_T)(first+count)>m->definition->child_count)return ESCX_STATUS_INVALID_ARGUMENT;for(i=0U;i<count;++i){child=ESCX_ROM_READ_U8(&m->definition->children[first+i]);if(EnterStateTree(m,child,d,(ESCX_BYTE_T)(depth+1U))!=ESCX_STATUS_PROCESSED)return ESCX_STATUS_INVALID_ARGUMENT;}return ESCX_STATUS_PROCESSED;}if(AddAtomic(m,s)==ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;if(type==ESCX_STATE_FINAL)return NotifyFinalEntered(m,s);return ESCX_STATUS_PROCESSED;}
static ESCX_ProcessStatus EnterInitial(ESCX_StateMachine*m,ESCX_BYTE_T s,void*d){return EnterStateTree(m,s,d,0U);}
void ESCX_StateMachineInit(ESCX_StateMachine*m,const ESCX_StateMachineDefinition*d,void*i,ESCX_BYTE_T*b,ESCX_BYTE_T z,ESCX_StartTimerFunction st,ESCX_CancelTimerFunction ct){if((m==(ESCX_StateMachine*)0)||(d==(const ESCX_StateMachineDefinition*)0)||(b==(ESCX_BYTE_T*)0)||(z<ESCX_ACTIVE_BYTES(d->state_count))){ESCX_ASSERT_TRUE(0);return;}m->definition=d;m->instance=i;m->active_bits=b;m->active_byte_count=z;memset(b,0,z);m->active_atomic_count=0U;m->current_event=ESCX_NO_EVENT;m->running=ESCX_FALSE;m->processing=ESCX_FALSE;m->finished=ESCX_FALSE;
#if ESCX_ENABLE_TIMERS
m->timer_id=ESCX_INVALID_TIMER_ID;m->start_timer=st;m->cancel_timer=ct;
#else
(void)st;(void)ct;
#endif
ESCX_EventQueueInit(&m->internal_queue);ESCX_EventQueueInit(&m->external_queue);}
static ESCX_UINT16_T FindSelector(const ESCX_StateMachine*m,ESCX_BYTE_T state,ESCX_BYTE_T event){ESCX_UINT16_T base,slot;if((state==ESCX_NO_STATE)||((ESCX_UINT16_T)state>=m->definition->state_count))return ESCX_NO_SELECTOR;base=ESCX_ROM_READ_U16(&m->definition->state_base[state]);if(base>(ESCX_UINT16_T)(65535U-event))return ESCX_NO_SELECTOR;slot=(ESCX_UINT16_T)(base+event);if(slot>=m->definition->slot_count)return ESCX_NO_SELECTOR;if(ESCX_ROM_READ_U8(&m->definition->slot_owner[slot])!=state)return ESCX_NO_SELECTOR;return ESCX_ROM_READ_U16(&m->definition->slot_selector[slot]);}
static ESCX_BOOL_T HasActiveFinalChild(const ESCX_StateMachine*m,ESCX_BYTE_T parent){ESCX_BYTE_T i,state;for(i=0U;i<m->active_atomic_count;++i){state=m->active_atomic_states[i];if((Type(m,state)==ESCX_STATE_FINAL)&&(Parent(m,state)==parent))return ESCX_TRUE;}return ESCX_FALSE;}
static ESCX_BOOL_T IsComplete(const ESCX_StateMachine*m,ESCX_BYTE_T state){ESCX_BYTE_T type,count,child;ESCX_UINT16_T first,i;type=Type(m,state);if(type==ESCX_STATE_COMPOUND)return HasActiveFinalChild(m,state);if(type!=ESCX_STATE_PARALLEL)return ESCX_FALSE;first=FirstChild(m,state);count=ChildCount(m,state);if(count==0U)return ESCX_FALSE;if((ESCX_UINT16_T)(first+count)>m->definition->child_count)return ESCX_FALSE;for(i=0U;i<count;++i){child=ESCX_ROM_READ_U8(&m->definition->children[first+i]);if(IsComplete(m,child)==ESCX_FALSE)return ESCX_FALSE;}return ESCX_TRUE;}
static ESCX_ProcessStatus NotifyFinalEntered(ESCX_StateMachine*m,ESCX_BYTE_T final_state){ESCX_BYTE_T state,event_id;state=Parent(m,final_state);while(state!=ESCX_NO_STATE){if(IsComplete(m,state)==ESCX_FALSE)break;if(Parent(m,state)==ESCX_NO_STATE){m->finished=ESCX_TRUE;return ESCX_STATUS_PROCESSED;}event_id=DoneEvent(m,state);if(event_id!=ESCX_NO_EVENT){if(ESCX_EventQueuePush(&m->internal_queue,event_id,(void*)0)==ESCX_FALSE)return ESCX_STATUS_QUEUE_FULL;}state=Parent(m,state);}if(Parent(m,final_state)==ESCX_NO_STATE)m->finished=ESCX_TRUE;return ESCX_STATUS_PROCESSED;}

static ESCX_ProcessStatus RunPlan(ESCX_StateMachine*m,ESCX_UINT16_T pid,void*d,ESCX_BOOL_T entering){const ESCX_StatePlan*p;ESCX_UINT16_T first,i;ESCX_BYTE_T count,s;if(pid==ESCX_NO_PLAN)return ESCX_STATUS_PROCESSED;if(pid>=m->definition->plan_count)return ESCX_STATUS_INVALID_ARGUMENT;p=&m->definition->plans[pid];first=ESCX_ROM_READ_U16(&p->first_state);count=ESCX_ROM_READ_U8(&p->state_count);if((ESCX_UINT16_T)(first+count)>m->definition->plan_state_count)return ESCX_STATUS_INVALID_ARGUMENT;for(i=0U;i<count;++i){s=ESCX_ROM_READ_U8(&m->definition->plan_states[first+i]);if(entering!=ESCX_FALSE){SetActive(m,s,ESCX_TRUE);CallState(m,s,d,0);CallState(m,s,d,2);if((Type(m,s)==ESCX_STATE_ATOMIC)||(Type(m,s)==ESCX_STATE_FINAL)){if(AddAtomic(m,s)==ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;if(Type(m,s)==ESCX_STATE_FINAL){ESCX_ProcessStatus cr;cr=NotifyFinalEntered(m,s);if(cr!=ESCX_STATUS_PROCESSED)return cr;}}}else{CallState(m,s,d,1);SetActive(m,s,ESCX_FALSE);RemoveAtomic(m,s);}}return ESCX_STATUS_PROCESSED;}
static ESCX_ProcessStatus Execute(ESCX_StateMachine*m,ESCX_UINT16_T tid,void*d){const ESCX_Transition*t;ESCX_UINT16_T xp,ep;ESCX_BYTE_T type;ESCX_ActionFunction a;ESCX_ProcessStatus r;if((tid==ESCX_NO_TRANSITION)||(tid>=m->definition->transition_count))return ESCX_STATUS_IGNORED;t=&m->definition->transitions[tid];type=ESCX_ROM_READ_U8(&t->type);xp=ESCX_ROM_READ_U16(&t->exit_plan);ep=ESCX_ROM_READ_U16(&t->entry_plan);if(type!=ESCX_TRANSITION_TARGETLESS){r=RunPlan(m,xp,d,ESCX_FALSE);if(r!=ESCX_STATUS_PROCESSED)return r;}a=ReadAction(&t->action);if(a!=(ESCX_ActionFunction)0)a(m,d);if(type!=ESCX_TRANSITION_TARGETLESS){r=RunPlan(m,ep,d,ESCX_TRUE);if(r!=ESCX_STATUS_PROCESSED)return r;}return ESCX_STATUS_PROCESSED;}
static ESCX_BOOL_T PlanHasState(const ESCX_StateMachine*m,ESCX_UINT16_T pid,ESCX_BYTE_T state){const ESCX_StatePlan*p;ESCX_UINT16_T first,i;ESCX_BYTE_T count;if(pid==ESCX_NO_PLAN)return ESCX_FALSE;if(pid>=m->definition->plan_count)return ESCX_FALSE;p=&m->definition->plans[pid];first=ESCX_ROM_READ_U16(&p->first_state);count=ESCX_ROM_READ_U8(&p->state_count);for(i=0U;i<count;++i)if(ESCX_ROM_READ_U8(&m->definition->plan_states[first+i])==state)return ESCX_TRUE;return ESCX_FALSE;}
static ESCX_BOOL_T Conflict(const ESCX_StateMachine*m,ESCX_UINT16_T a,ESCX_UINT16_T b){const ESCX_Transition*ta;const ESCX_Transition*tb;ESCX_UINT16_T pa,pb,i;const ESCX_StatePlan*p;ESCX_BYTE_T count,state;if(a==b)return ESCX_TRUE;ta=&m->definition->transitions[a];tb=&m->definition->transitions[b];pa=ESCX_ROM_READ_U16(&ta->exit_plan);pb=ESCX_ROM_READ_U16(&tb->exit_plan);if((pa==ESCX_NO_PLAN)||(pb==ESCX_NO_PLAN))return ESCX_FALSE;p=&m->definition->plans[pa];count=ESCX_ROM_READ_U8(&p->state_count);i=ESCX_ROM_READ_U16(&p->first_state);while(count--!=0U){state=ESCX_ROM_READ_U8(&m->definition->plan_states[i++]);if(PlanHasState(m,pb,state))return ESCX_TRUE;}return ESCX_FALSE;}
static ESCX_BYTE_T DepthOfTransition(const ESCX_StateMachine*m,ESCX_UINT16_T t){return ESCX_ROM_READ_U8(&m->definition->transitions[t].source_depth);}
static ESCX_BYTE_T OrderOfTransition(const ESCX_StateMachine*m,ESCX_UINT16_T t){return ESCX_ROM_READ_U8(&m->definition->transitions[t].document_order);}
static ESCX_ProcessStatus ExecuteSet(ESCX_StateMachine*m,ESCX_UINT16_T*ids,ESCX_BYTE_T count,void*d){ESCX_BYTE_T i;ESCX_ProcessStatus r;for(i=0U;i<count;++i){r=Execute(m,ids[i],d);if(r!=ESCX_STATUS_PROCESSED)return r;}return count?ESCX_STATUS_PROCESSED:ESCX_STATUS_IGNORED;}
static ESCX_ProcessStatus ProcessEvent(ESCX_StateMachine*m,ESCX_BYTE_T e,void*d){ESCX_UINT16_T selected[ESCX_MAX_ENABLED_TRANSITIONS];ESCX_BYTE_T selected_count=0U,leaf,s,depth,i,j;ESCX_UINT16_T sid,tid;ESCX_SelectorFunction f;if(e>=m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;m->current_event=e;for(i=0U;i<m->active_atomic_count;++i){leaf=m->active_atomic_states[i];s=leaf;depth=0U;while(s!=ESCX_NO_STATE){sid=FindSelector(m,s,e);if((sid!=ESCX_NO_SELECTOR)&&(sid<m->definition->selector_count)){f=ReadSelector(&m->definition->selectors[sid]);if(f==(ESCX_SelectorFunction)0)return ESCX_STATUS_INVALID_ARGUMENT;tid=f(m,d);if(tid!=ESCX_NO_TRANSITION){ESCX_BOOL_T keep=ESCX_TRUE;for(j=0U;j<selected_count;++j){if(Conflict(m,tid,selected[j])){if((DepthOfTransition(m,tid)>DepthOfTransition(m,selected[j]))||((DepthOfTransition(m,tid)==DepthOfTransition(m,selected[j]))&&(OrderOfTransition(m,tid)<OrderOfTransition(m,selected[j])))){selected[j]=tid;}keep=ESCX_FALSE;break;}}if(keep!=ESCX_FALSE){if(selected_count>=ESCX_MAX_ENABLED_TRANSITIONS)return ESCX_STATUS_INVALID_ARGUMENT;selected[selected_count++]=tid;}break;}}s=Parent(m,s);if(++depth>ESCX_MAX_HIERARCHY_DEPTH)return ESCX_STATUS_HIERARCHY_LIMIT;}}return ExecuteSet(m,selected,selected_count,d);}
static ESCX_ProcessStatus ProcessEventless(ESCX_StateMachine*m){ESCX_BYTE_T i,s,depth;ESCX_UINT16_T sid,tid;ESCX_SelectorFunction f;for(i=0U;i<m->active_atomic_count;++i){s=m->active_atomic_states[i];depth=0U;while(s!=ESCX_NO_STATE){sid=EventlessSelector(m,s);if((sid!=ESCX_NO_SELECTOR)&&(sid<m->definition->selector_count)){f=ReadSelector(&m->definition->selectors[sid]);if(f==(ESCX_SelectorFunction)0)return ESCX_STATUS_INVALID_ARGUMENT;tid=f(m,(void*)0);if(tid!=ESCX_NO_TRANSITION){m->current_event=ESCX_NO_EVENT;return Execute(m,tid,(void*)0);}}s=Parent(m,s);if(++depth>ESCX_MAX_HIERARCHY_DEPTH)return ESCX_STATUS_HIERARCHY_LIMIT;}}return ESCX_STATUS_IGNORED;}
static ESCX_ProcessStatus Stabilize(ESCX_StateMachine*m){ESCX_UINT16_T steps=0U;ESCX_ProcessStatus r;ESCX_QueuedEvent q;for(;;){r=ProcessEventless(m);if(r==ESCX_STATUS_PROCESSED){if(++steps>ESCX_MAX_MICROSTEPS)return ESCX_STATUS_MICROSTEP_LIMIT;continue;}if(r!=ESCX_STATUS_IGNORED)return r;if(ESCX_EventQueuePop(&m->internal_queue,&q)!=ESCX_FALSE){
#if ESCX_ENABLE_EVENT_DATA
r=ProcessEvent(m,q.event_id,q.event_data);
#else
r=ProcessEvent(m,q.event_id,(void*)0);
#endif
if((r!=ESCX_STATUS_PROCESSED)&&(r!=ESCX_STATUS_IGNORED)){return r;}
if(++steps>ESCX_MAX_MICROSTEPS){return ESCX_STATUS_MICROSTEP_LIMIT;}
continue;}return ESCX_STATUS_PROCESSED;}}
static ESCX_ProcessStatus Macrostep(ESCX_StateMachine*m,ESCX_BYTE_T e,void*d){ESCX_ProcessStatus r;r=ProcessEvent(m,e,d);if((r!=ESCX_STATUS_PROCESSED)&&(r!=ESCX_STATUS_IGNORED))return r;r=Stabilize(m);return r;}
ESCX_ProcessStatus ESCX_StateMachineStart(ESCX_StateMachine*m){ESCX_ProcessStatus r;if((m==(ESCX_StateMachine*)0)||(m->definition==(const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT;if(m->running!=ESCX_FALSE)return ESCX_STATUS_IGNORED;m->finished=ESCX_FALSE;m->running=ESCX_TRUE;r=EnterInitial(m,m->definition->initial_state,(void*)0);if(r!=ESCX_STATUS_PROCESSED)return r;m->processing=ESCX_TRUE;r=Stabilize(m);m->processing=ESCX_FALSE;return r;}
ESCX_ProcessStatus ESCX_StateMachineRaiseInternal(ESCX_StateMachine*m,ESCX_BYTE_T e,void*d){if((m==(ESCX_StateMachine*)0)||(m->running==ESCX_FALSE))return ESCX_STATUS_NOT_RUNNING;if(e>=m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;return ESCX_EventQueuePush(&m->internal_queue,e,d)?ESCX_STATUS_PROCESSED:ESCX_STATUS_QUEUE_FULL;}
ESCX_ProcessStatus ESCX_StateMachineSend(ESCX_StateMachine*m,ESCX_BYTE_T e,void*d){ESCX_ProcessStatus r;ESCX_QueuedEvent q;if((m==(ESCX_StateMachine*)0)||(m->definition==(const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT;if(m->running==ESCX_FALSE)return ESCX_STATUS_NOT_RUNNING;if(e>=m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;if(m->processing!=ESCX_FALSE)return ESCX_EventQueuePush(&m->external_queue,e,d)?ESCX_STATUS_PROCESSED:ESCX_STATUS_QUEUE_FULL;m->processing=ESCX_TRUE;r=Macrostep(m,e,d);while((r==ESCX_STATUS_PROCESSED)&&(ESCX_EventQueuePop(&m->external_queue,&q)!=ESCX_FALSE)){
#if ESCX_ENABLE_EVENT_DATA
r=Macrostep(m,q.event_id,q.event_data);
#else
r=Macrostep(m,q.event_id,(void*)0);
#endif
}m->processing=ESCX_FALSE;return r;}
void ESCX_StateMachineStop(ESCX_StateMachine*m){ESCX_BYTE_T state;if((m==(ESCX_StateMachine*)0)||(m->running==ESCX_FALSE))return;while(m->active_atomic_count!=0U){state=m->active_atomic_states[m->active_atomic_count-1U];CallState(m,state,(void*)0,1);SetActive(m,state,ESCX_FALSE);--m->active_atomic_count;}memset(m->active_bits,0,m->active_byte_count);m->running=ESCX_FALSE;m->processing=ESCX_FALSE;ESCX_EventQueueInit(&m->internal_queue);ESCX_EventQueueInit(&m->external_queue);}
ESCX_ProcessStatus ESCX_StateMachineTimerExpired(ESCX_StateMachine*m,int id,void*d){
#if ESCX_ENABLE_TIMERS
if(m==(ESCX_StateMachine*)0){return ESCX_STATUS_INVALID_ARGUMENT;}
if((m->timer_id==ESCX_INVALID_TIMER_ID)||(id!=m->timer_id)){return ESCX_STATUS_IGNORED;}
return ESCX_StateMachineSend(m,ESCX_TIMER_EVENT,d);
#else
(void)m;(void)id;(void)d;return ESCX_STATUS_IGNORED;
#endif
}
