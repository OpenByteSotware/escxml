#include "StateMachine.h"
#include "AssertHandler.h"
#include <string.h>
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
static ESCX_BYTE_T StateU8(const ESCX_StateMachine*m,ESCX_BYTE_T id,unsigned int offset){return ESCX_ROM_READ_U8(((const ESCX_BYTE_T*)&m->definition->states[id])+offset);}
#define OFFSETOF_STATE(member) ((unsigned int)offsetof(ESCX_StateDescriptor,member))
#include <stddef.h>
static ESCX_BYTE_T Parent(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(parent_state));}
static ESCX_BYTE_T Initial(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(initial_child));}
static ESCX_BYTE_T Type(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(state_type));}
static void SetActive(ESCX_StateMachine*m,ESCX_BYTE_T s,ESCX_BOOL_T on){ESCX_BYTE_T i,mask;i=(ESCX_BYTE_T)(s>>3);mask=(ESCX_BYTE_T)(1U<<(s&7U));if(i>=m->active_byte_count)return;if(on)m->active_bits[i]=(ESCX_BYTE_T)(m->active_bits[i]|mask);else m->active_bits[i]=(ESCX_BYTE_T)(m->active_bits[i]&((ESCX_BYTE_T)~mask));}
ESCX_BOOL_T ESCX_StateMachineIsInState(const ESCX_StateMachine*m,ESCX_BYTE_T s){ESCX_BYTE_T i,mask;if((m==(const ESCX_StateMachine*)0)||(m->running==ESCX_FALSE)||(s==ESCX_NO_STATE))return ESCX_FALSE;i=(ESCX_BYTE_T)(s>>3);if(i>=m->active_byte_count)return ESCX_FALSE;mask=(ESCX_BYTE_T)(1U<<(s&7U));return((m->active_bits[i]&mask)!=0U)?ESCX_TRUE:ESCX_FALSE;}
static void CallState(ESCX_StateMachine*m,ESCX_BYTE_T s,void*d,int which){const ESCX_StateDescriptor*x;ESCX_StateFunction f;x=&m->definition->states[s];f=(which==0)?ReadStateFn(&x->on_entry):((which==1)?ReadStateFn(&x->on_exit):ReadStateFn(&x->on_state));if(f!=(ESCX_StateFunction)0)f(m,d);}
static ESCX_ProcessStatus EnterInitial(ESCX_StateMachine*m,ESCX_BYTE_T s,void*d){ESCX_BYTE_T depth=0U,next;while(s!=ESCX_NO_STATE){if((ESCX_UINT16_T)s>=m->definition->state_count)return ESCX_STATUS_INVALID_ARGUMENT;SetActive(m,s,ESCX_TRUE);CallState(m,s,d,0);CallState(m,s,d,2);m->active_atomic_state=s;if(Type(m,s)!=ESCX_STATE_COMPOUND)break;next=Initial(m,s);if(next==ESCX_NO_STATE)return ESCX_STATUS_INVALID_ARGUMENT;s=next;++depth;if(depth>ESCX_MAX_HIERARCHY_DEPTH)return ESCX_STATUS_HIERARCHY_LIMIT;}return ESCX_STATUS_PROCESSED;}
void ESCX_StateMachineInit(ESCX_StateMachine*m,const ESCX_StateMachineDefinition*d,void*i,ESCX_BYTE_T*b,ESCX_BYTE_T z,ESCX_StartTimerFunction st,ESCX_CancelTimerFunction ct){if((m==(ESCX_StateMachine*)0)||(d==(const ESCX_StateMachineDefinition*)0)||(b==(ESCX_BYTE_T*)0)||(z<ESCX_ACTIVE_BYTES(d->state_count))){ESCX_ASSERT_TRUE(0);return;}m->definition=d;m->instance=i;m->active_bits=b;m->active_byte_count=z;memset(b,0,z);m->active_atomic_state=ESCX_NO_STATE;m->current_event=ESCX_NO_EVENT;m->running=ESCX_FALSE;m->processing=ESCX_FALSE;
#if ESCX_ENABLE_TIMERS
m->timer_id=ESCX_INVALID_TIMER_ID;m->start_timer=st;m->cancel_timer=ct;
#else
(void)st;(void)ct;
#endif
ESCX_EventQueueInit(&m->event_queue);}
ESCX_ProcessStatus ESCX_StateMachineStart(ESCX_StateMachine*m){if((m==(ESCX_StateMachine*)0)||(m->definition==(const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT;if(m->running!=ESCX_FALSE)return ESCX_STATUS_IGNORED;m->running=ESCX_TRUE;return EnterInitial(m,m->definition->initial_state,(void*)0);}
static ESCX_UINT16_T FindSelector(const ESCX_StateMachine*m,ESCX_BYTE_T state,ESCX_BYTE_T event){ESCX_UINT16_T base,slot;if((state==ESCX_NO_STATE)||((ESCX_UINT16_T)state>=m->definition->state_count))return ESCX_NO_SELECTOR;base=ESCX_ROM_READ_U16(&m->definition->state_base[state]);if(base>(ESCX_UINT16_T)(65535U-event))return ESCX_NO_SELECTOR;slot=(ESCX_UINT16_T)(base+event);if(slot>=m->definition->slot_count)return ESCX_NO_SELECTOR;if(ESCX_ROM_READ_U8(&m->definition->slot_owner[slot])!=state)return ESCX_NO_SELECTOR;return ESCX_ROM_READ_U16(&m->definition->slot_selector[slot]);}
static ESCX_ProcessStatus RunPlan(ESCX_StateMachine*m,ESCX_UINT16_T pid,void*d,ESCX_BOOL_T entering){const ESCX_StatePlan*p;ESCX_UINT16_T first,i;ESCX_BYTE_T count,s;if(pid==ESCX_NO_PLAN)return ESCX_STATUS_PROCESSED;if(pid>=m->definition->plan_count)return ESCX_STATUS_INVALID_ARGUMENT;p=&m->definition->plans[pid];first=ESCX_ROM_READ_U16(&p->first_state);count=ESCX_ROM_READ_U8(&p->state_count);if((ESCX_UINT16_T)(first+count)>m->definition->plan_state_count)return ESCX_STATUS_INVALID_ARGUMENT;for(i=0U;i<count;++i){s=ESCX_ROM_READ_U8(&m->definition->plan_states[first+i]);if(entering!=ESCX_FALSE){SetActive(m,s,ESCX_TRUE);CallState(m,s,d,0);CallState(m,s,d,2);m->active_atomic_state=s;}else{CallState(m,s,d,1);SetActive(m,s,ESCX_FALSE);}}return ESCX_STATUS_PROCESSED;}
static ESCX_ProcessStatus Execute(ESCX_StateMachine*m,ESCX_UINT16_T tid,void*d){const ESCX_Transition*t;ESCX_UINT16_T xp,ep;ESCX_BYTE_T type;ESCX_ActionFunction a;ESCX_ProcessStatus r;if((tid==ESCX_NO_TRANSITION)||(tid>=m->definition->transition_count))return ESCX_STATUS_IGNORED;t=&m->definition->transitions[tid];type=ESCX_ROM_READ_U8(&t->type);xp=ESCX_ROM_READ_U16(&t->exit_plan);ep=ESCX_ROM_READ_U16(&t->entry_plan);if(type!=ESCX_TRANSITION_TARGETLESS){r=RunPlan(m,xp,d,ESCX_FALSE);if(r!=ESCX_STATUS_PROCESSED)return r;}a=ReadAction(&t->action);if(a!=(ESCX_ActionFunction)0)a(m,d);if(type!=ESCX_TRANSITION_TARGETLESS){r=RunPlan(m,ep,d,ESCX_TRUE);if(r!=ESCX_STATUS_PROCESSED)return r;if((m->active_atomic_state!=ESCX_NO_STATE)&&(Type(m,m->active_atomic_state)==ESCX_STATE_COMPOUND))return EnterInitial(m,Initial(m,m->active_atomic_state),d);}return ESCX_STATUS_PROCESSED;}
static ESCX_ProcessStatus ProcessOne(ESCX_StateMachine*m,ESCX_BYTE_T e,void*d){ESCX_BYTE_T s,depth=0U;ESCX_UINT16_T sid,tid;ESCX_SelectorFunction f;if(e>=m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;m->current_event=e;s=m->active_atomic_state;while(s!=ESCX_NO_STATE){sid=FindSelector(m,s,e);if((sid!=ESCX_NO_SELECTOR)&&(sid<m->definition->selector_count)){f=ReadSelector(&m->definition->selectors[sid]);if(f==(ESCX_SelectorFunction)0)return ESCX_STATUS_INVALID_ARGUMENT;tid=f(m,d);if(tid!=ESCX_NO_TRANSITION)return Execute(m,tid,d);}s=Parent(m,s);++depth;if(depth>ESCX_MAX_HIERARCHY_DEPTH)return ESCX_STATUS_HIERARCHY_LIMIT;}return ESCX_STATUS_IGNORED;}
ESCX_ProcessStatus ESCX_StateMachineSend(ESCX_StateMachine*m,ESCX_BYTE_T e,void*d){ESCX_ProcessStatus r,one;ESCX_QueuedEvent q;if((m==(ESCX_StateMachine*)0)||(m->definition==(const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT;if(m->running==ESCX_FALSE)return ESCX_STATUS_NOT_RUNNING;if(m->processing!=ESCX_FALSE)return ESCX_EventQueuePush(&m->event_queue,e,d)?ESCX_STATUS_PROCESSED:ESCX_STATUS_QUEUE_FULL;m->processing=ESCX_TRUE;r=ProcessOne(m,e,d);while(ESCX_EventQueuePop(&m->event_queue,&q)!=ESCX_FALSE){
#if ESCX_ENABLE_EVENT_DATA
one=ProcessOne(m,q.event_id,q.event_data);
#else
one=ProcessOne(m,q.event_id,(void*)0);
#endif
if((r==ESCX_STATUS_IGNORED)&&(one!=ESCX_STATUS_IGNORED))r=one;}m->processing=ESCX_FALSE;return r;}
void ESCX_StateMachineStop(ESCX_StateMachine*m){ESCX_BYTE_T s,depth=0U;if((m==(ESCX_StateMachine*)0)||(m->running==ESCX_FALSE))return;s=m->active_atomic_state;while(s!=ESCX_NO_STATE){CallState(m,s,(void*)0,1);SetActive(m,s,ESCX_FALSE);s=Parent(m,s);if(++depth>ESCX_MAX_HIERARCHY_DEPTH)break;}m->running=ESCX_FALSE;m->active_atomic_state=ESCX_NO_STATE;ESCX_EventQueueInit(&m->event_queue);}
ESCX_ProcessStatus ESCX_StateMachineTimerExpired(ESCX_StateMachine*m,int id,void*d){
#if ESCX_ENABLE_TIMERS
if(m==(ESCX_StateMachine*)0){return ESCX_STATUS_INVALID_ARGUMENT;}
if((m->timer_id==ESCX_INVALID_TIMER_ID)||(id!=m->timer_id)){return ESCX_STATUS_IGNORED;}
return ESCX_StateMachineSend(m,ESCX_TIMER_EVENT,d);
#else
(void)m;(void)id;(void)d;return ESCX_STATUS_IGNORED;
#endif
}
