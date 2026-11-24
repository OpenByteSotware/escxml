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
static ESCX_BYTE_T StateDepth(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(depth));}
static ESCX_BYTE_T StateOrder(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(document_order));}
static ESCX_BYTE_T HistorySlot(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(history_slot));}
static ESCX_UINT16_T HistoryDefaultPlan(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU16(m,s,OFFSETOF_STATE(history_default_plan));}
static ESCX_BYTE_T DoneEvent(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(done_event));}
static ESCX_UINT16_T EventlessSelector(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU16(m,s,OFFSETOF_STATE(eventless_selector));}
static void SetActive(ESCX_StateMachine*m,ESCX_BYTE_T s,ESCX_BOOL_T on){ESCX_BYTE_T i,mask;i=(ESCX_BYTE_T)(s>>3);mask=(ESCX_BYTE_T)(1U<<(s&7U));if(i>=m->active_byte_count)return;if(on)m->active_bits[i]=(ESCX_BYTE_T)(m->active_bits[i]|mask);else m->active_bits[i]=(ESCX_BYTE_T)(m->active_bits[i]&((ESCX_BYTE_T)~mask));}
static ESCX_BOOL_T IsActive(const ESCX_StateMachine*m,ESCX_BYTE_T s){ESCX_BYTE_T i,mask;i=(ESCX_BYTE_T)(s>>3);if(i>=m->active_byte_count)return ESCX_FALSE;mask=(ESCX_BYTE_T)(1U<<(s&7U));return((m->active_bits[i]&mask)!=0U)?ESCX_TRUE:ESCX_FALSE;}
static ESCX_BOOL_T AddAtomic(ESCX_StateMachine*m,ESCX_BYTE_T s){ESCX_BYTE_T i;for(i=0U;i<m->active_atomic_count;++i)if(m->active_atomic_states[i]==s)return ESCX_TRUE;if(m->active_atomic_count>=ESCX_MAX_ACTIVE_ATOMIC_STATES)return ESCX_FALSE;m->active_atomic_states[m->active_atomic_count++]=s;return ESCX_TRUE;}
static void RemoveAtomic(ESCX_StateMachine*m,ESCX_BYTE_T s){ESCX_BYTE_T i;for(i=0U;i<m->active_atomic_count;++i){if(m->active_atomic_states[i]==s){--m->active_atomic_count;m->active_atomic_states[i]=m->active_atomic_states[m->active_atomic_count];return;}}}
ESCX_BOOL_T ESCX_StateMachineIsInState(const ESCX_StateMachine*m,ESCX_BYTE_T s){if((m==(const ESCX_StateMachine*)0)||(m->running==ESCX_FALSE)||(s==ESCX_NO_STATE))return ESCX_FALSE;return IsActive(m,s);}
ESCX_BOOL_T ESCX_StateMachineIsFinished(const ESCX_StateMachine*m){return((m!=(const ESCX_StateMachine*)0)&&(m->finished!=ESCX_FALSE))?ESCX_TRUE:ESCX_FALSE;}
static void ReadInvokeDescriptor(const ESCX_StateMachine*m,ESCX_BYTE_T index,ESCX_InvokeDescriptor*out){(void)ESCX_ROM_COPY(out,&m->invokes[index],sizeof(*out));}
static ESCX_ProcessStatus StartInvokesForState(ESCX_StateMachine*m,ESCX_BYTE_T state){ESCX_BYTE_T i;ESCX_InvokeDescriptor d;int status;if((m->invokes==(const ESCX_InvokeDescriptor*)0)||(m->invoke_instances==(ESCX_InvokeInstance*)0))return ESCX_STATUS_PROCESSED;for(i=0U;i<m->invoke_count;++i){ReadInvokeDescriptor(m,i,&d);if((d.owner_state==state)&&(m->invoke_instances[i].active==ESCX_FALSE)){if(d.start==(ESCX_InvokeStartFunction)0)return ESCX_STATUS_INVALID_ARGUMENT;status=d.start(m,d.child_storage);if(status!=(int)ESCX_STATUS_PROCESSED)return (ESCX_ProcessStatus)status;m->invoke_instances[i].active=ESCX_TRUE;}}return ESCX_STATUS_PROCESSED;}
static void CancelInvokesForState(ESCX_StateMachine*m,ESCX_BYTE_T state){ESCX_BYTE_T i;ESCX_InvokeDescriptor d;if((m->invokes==(const ESCX_InvokeDescriptor*)0)||(m->invoke_instances==(ESCX_InvokeInstance*)0))return;for(i=0U;i<m->invoke_count;++i){ReadInvokeDescriptor(m,i,&d);if((d.owner_state==state)&&(m->invoke_instances[i].active!=ESCX_FALSE)){if(d.cancel!=(ESCX_InvokeCancelFunction)0)d.cancel(m,d.child_storage);m->invoke_instances[i].active=ESCX_FALSE;}}}
static void CallState(ESCX_StateMachine*m,ESCX_BYTE_T s,void*d,int which){const ESCX_StateDescriptor*x;ESCX_StateFunction f;x=&m->definition->states[s];if(which==1)CancelInvokesForState(m,s);f=(which==0)?ReadStateFn(&x->on_entry):((which==1)?ReadStateFn(&x->on_exit):ReadStateFn(&x->on_state));if(f!=(ESCX_StateFunction)0)f(m,d);if(which==0)(void)StartInvokesForState(m,s);}
static ESCX_ProcessStatus NotifyFinalEntered(ESCX_StateMachine*m,ESCX_BYTE_T final_state);
static ESCX_ProcessStatus RunPlan(ESCX_StateMachine*m,ESCX_UINT16_T pid,void*d,ESCX_BOOL_T entering);
static ESCX_ProcessStatus RestoreHistory(ESCX_StateMachine*m,ESCX_BYTE_T history_state,void*d);
static ESCX_UINT16_T FirstChild(const ESCX_StateMachine*m,ESCX_BYTE_T s){return ESCX_ROM_READ_U16(&m->definition->states[s].first_child_index);}
static ESCX_BYTE_T ChildCount(const ESCX_StateMachine*m,ESCX_BYTE_T s){return StateU8(m,s,OFFSETOF_STATE(child_count));}
static ESCX_ProcessStatus EnterStateTree(ESCX_StateMachine*m,ESCX_BYTE_T s,void*d,ESCX_BYTE_T depth){ESCX_BYTE_T type,count,child;ESCX_UINT16_T first,i;if(depth>ESCX_MAX_HIERARCHY_DEPTH)return ESCX_STATUS_HIERARCHY_LIMIT;if((s==ESCX_NO_STATE)||((ESCX_UINT16_T)s>=m->definition->state_count))return ESCX_STATUS_INVALID_ARGUMENT;if((Type(m,s)==ESCX_STATE_HISTORY_SHALLOW)||(Type(m,s)==ESCX_STATE_HISTORY_DEEP))return RestoreHistory(m,s,d);if(IsActive(m,s)==ESCX_FALSE){SetActive(m,s,ESCX_TRUE);CallState(m,s,d,0);CallState(m,s,d,2);}type=Type(m,s);if(type==ESCX_STATE_COMPOUND){child=Initial(m,s);return EnterStateTree(m,child,d,(ESCX_BYTE_T)(depth+1U));}if(type==ESCX_STATE_PARALLEL){first=FirstChild(m,s);count=ChildCount(m,s);if((ESCX_UINT16_T)(first+count)>m->definition->child_count)return ESCX_STATUS_INVALID_ARGUMENT;for(i=0U;i<count;++i){child=ESCX_ROM_READ_U8(&m->definition->children[first+i]);if(EnterStateTree(m,child,d,(ESCX_BYTE_T)(depth+1U))!=ESCX_STATUS_PROCESSED)return ESCX_STATUS_INVALID_ARGUMENT;}return ESCX_STATUS_PROCESSED;}if(AddAtomic(m,s)==ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;if(type==ESCX_STATE_FINAL)return NotifyFinalEntered(m,s);return ESCX_STATUS_PROCESSED;}
static ESCX_ProcessStatus EnterInitial(ESCX_StateMachine*m,ESCX_BYTE_T s,void*d){return EnterStateTree(m,s,d,0U);}
void ESCX_StateMachineInit(ESCX_StateMachine*m,const ESCX_StateMachineDefinition*d,void*i,ESCX_BYTE_T*b,ESCX_BYTE_T z,ESCX_HistoryStorage*h,ESCX_StartTimerFunction st,ESCX_CancelTimerFunction ct){if((m==(ESCX_StateMachine*)0)||(d==(const ESCX_StateMachineDefinition*)0)||(b==(ESCX_BYTE_T*)0)||(z<ESCX_ACTIVE_BYTES(d->state_count))){ESCX_ASSERT_TRUE(0);return;}m->definition=d;m->instance=i;m->history=h;m->invokes=(const ESCX_InvokeDescriptor*)0;m->invoke_instances=(ESCX_InvokeInstance*)0;m->invoke_count=0U;m->parent_machine=(ESCX_StateMachine*)0;m->parent_invoke_id=ESCX_NO_INVOKE;
#if ESCX_ENABLE_DELAYED_SENDS
{ESCX_BYTE_T di;for(di=0U;di<ESCX_MAX_DELAYED_SENDS;++di){m->delayed_sends[di].active=ESCX_FALSE;m->delayed_sends[di].timer_id=ESCX_INVALID_TIMER_ID;}}
#endif
m->active_bits=b;m->active_byte_count=z;memset(b,0,z);m->active_atomic_count=0U;m->current_event=ESCX_NO_EVENT;m->error_execution_event=ESCX_NO_EVENT;m->error_communication_event=ESCX_NO_EVENT;m->last_error=ESCX_ERROR_NONE;m->last_error_status=ESCX_STATUS_PROCESSED;m->reporting_error=ESCX_FALSE;m->running=ESCX_FALSE;m->processing=ESCX_FALSE;m->finished=ESCX_FALSE;
#if ESCX_ENABLE_TIMERS
m->start_timer=st;m->cancel_timer=ct;m->timer_context=(void*)0;
#else
(void)st;(void)ct;
#endif
if(h!=(ESCX_HistoryStorage*)0){ESCX_BYTE_T hi;if((h->valid==(ESCX_BYTE_T*)0)||(h->bits==(ESCX_BYTE_T*)0)||(h->bytes_per_slot<z)){ESCX_ASSERT_TRUE(0);return;}for(hi=0U;hi<h->slot_count;++hi)h->valid[hi]=0U;}ESCX_EventQueueInit(&m->internal_queue);ESCX_EventQueueInit(&m->external_queue);
#if ESCX_ENABLE_DELAYED_SENDS
{ESCX_BYTE_T di;for(di=0U;di<ESCX_MAX_DELAYED_SENDS;++di){if((m->delayed_sends[di].active!=ESCX_FALSE)&&(m->cancel_timer!=(ESCX_CancelTimerFunction)0))(void)m->cancel_timer(m,m->delayed_sends[di].timer_id,m->timer_context);m->delayed_sends[di].active=ESCX_FALSE;m->delayed_sends[di].timer_id=ESCX_INVALID_TIMER_ID;}}
#endif
}
static ESCX_ProcessStatus Stabilize(ESCX_StateMachine*m);
ESCX_ProcessStatus ESCX_StateMachineConfigureInvokes(ESCX_StateMachine*m,const ESCX_InvokeDescriptor*d,ESCX_InvokeInstance*i,ESCX_BYTE_T count){ESCX_BYTE_T n;if((m==(ESCX_StateMachine*)0)||(count>ESCX_MAX_INVOKES))return ESCX_STATUS_INVALID_ARGUMENT;if((count!=0U)&&((d==(const ESCX_InvokeDescriptor*)0)||(i==(ESCX_InvokeInstance*)0)))return ESCX_STATUS_INVALID_ARGUMENT;m->invokes=d;m->invoke_instances=i;m->invoke_count=count;for(n=0U;n<count;++n)i[n].active=ESCX_FALSE;return ESCX_STATUS_PROCESSED;}
ESCX_ProcessStatus ESCX_StateMachineCompleteInvoke(ESCX_StateMachine*m,ESCX_BYTE_T invoke_id,void*data){ESCX_InvokeDescriptor d;if((m==(ESCX_StateMachine*)0)||(invoke_id>=m->invoke_count)||(m->invoke_instances[invoke_id].active==ESCX_FALSE))return ESCX_STATUS_INVALID_ARGUMENT;ReadInvokeDescriptor(m,invoke_id,&d);m->invoke_instances[invoke_id].active=ESCX_FALSE;if(d.finalize!=(ESCX_InvokeFinalizeFunction)0)d.finalize(m,d.child_storage,d.done_event,data);if(d.done_event==ESCX_NO_EVENT)return ESCX_STATUS_PROCESSED;if(ESCX_StateMachineRaiseInternal(m,d.done_event,data)!=ESCX_STATUS_PROCESSED)return ESCX_STATUS_QUEUE_FULL;if(m->processing==ESCX_FALSE){ESCX_ProcessStatus r;m->processing=ESCX_TRUE;r=Stabilize(m);m->processing=ESCX_FALSE;return r;}return ESCX_STATUS_PROCESSED;}
ESCX_ProcessStatus ESCX_StateMachineReceiveInvokeEvent(ESCX_StateMachine*m,ESCX_BYTE_T invoke_id,ESCX_BYTE_T event_id,void*data){ESCX_InvokeDescriptor d;if((m==(ESCX_StateMachine*)0)||(invoke_id>=m->invoke_count)||(m->invoke_instances[invoke_id].active==ESCX_FALSE))return ESCX_STATUS_INVALID_ARGUMENT;if(event_id>=m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;ReadInvokeDescriptor(m,invoke_id,&d);if(d.finalize!=(ESCX_InvokeFinalizeFunction)0)d.finalize(m,d.child_storage,event_id,data);if(ESCX_StateMachineRaiseInternal(m,event_id,data)!=ESCX_STATUS_PROCESSED)return ESCX_STATUS_QUEUE_FULL;if(m->processing==ESCX_FALSE){ESCX_ProcessStatus r;m->processing=ESCX_TRUE;r=Stabilize(m);m->processing=ESCX_FALSE;return r;}return ESCX_STATUS_PROCESSED;}
static ESCX_ProcessStatus ForwardExternalToInvokes(ESCX_StateMachine*m,ESCX_BYTE_T event_id,void*data){ESCX_BYTE_T i;ESCX_InvokeDescriptor d;int status;for(i=0U;i<m->invoke_count;++i){if(m->invoke_instances[i].active!=ESCX_FALSE){ReadInvokeDescriptor(m,i,&d);if((d.autoforward!=ESCX_FALSE)&&(d.forward!=(ESCX_InvokeForwardFunction)0)){status=d.forward(m,d.child_storage,event_id,data);if(status!=(int)ESCX_STATUS_PROCESSED)return (ESCX_ProcessStatus)status;}}}return ESCX_STATUS_PROCESSED;}
static ESCX_UINT16_T FindSelector(const ESCX_StateMachine*m,ESCX_BYTE_T state,ESCX_BYTE_T event){ESCX_UINT16_T base,slot;if((state==ESCX_NO_STATE)||((ESCX_UINT16_T)state>=m->definition->state_count))return ESCX_NO_SELECTOR;base=ESCX_ROM_READ_U16(&m->definition->state_base[state]);if(base>(ESCX_UINT16_T)(65535U-event))return ESCX_NO_SELECTOR;slot=(ESCX_UINT16_T)(base+event);if(slot>=m->definition->slot_count)return ESCX_NO_SELECTOR;if(ESCX_ROM_READ_U8(&m->definition->slot_owner[slot])!=state)return ESCX_NO_SELECTOR;return ESCX_ROM_READ_U16(&m->definition->slot_selector[slot]);}
static ESCX_BOOL_T HasActiveFinalChild(const ESCX_StateMachine*m,ESCX_BYTE_T parent){ESCX_BYTE_T i,state;for(i=0U;i<m->active_atomic_count;++i){state=m->active_atomic_states[i];if((Type(m,state)==ESCX_STATE_FINAL)&&(Parent(m,state)==parent))return ESCX_TRUE;}return ESCX_FALSE;}
static ESCX_BOOL_T IsComplete(const ESCX_StateMachine*m,ESCX_BYTE_T state){ESCX_BYTE_T type,count,child;ESCX_UINT16_T first,i;type=Type(m,state);if(type==ESCX_STATE_COMPOUND)return HasActiveFinalChild(m,state);if(type!=ESCX_STATE_PARALLEL)return ESCX_FALSE;first=FirstChild(m,state);count=ChildCount(m,state);if(count==0U)return ESCX_FALSE;if((ESCX_UINT16_T)(first+count)>m->definition->child_count)return ESCX_FALSE;for(i=0U;i<count;++i){child=ESCX_ROM_READ_U8(&m->definition->children[first+i]);if(IsComplete(m,child)==ESCX_FALSE)return ESCX_FALSE;}return ESCX_TRUE;}
static ESCX_ProcessStatus NotifyFinalEntered(ESCX_StateMachine*m,ESCX_BYTE_T final_state){ESCX_BYTE_T state,event_id;state=Parent(m,final_state);while(state!=ESCX_NO_STATE){if(IsComplete(m,state)==ESCX_FALSE)break;if(Parent(m,state)==ESCX_NO_STATE){m->finished=ESCX_TRUE;return ESCX_STATUS_PROCESSED;}event_id=DoneEvent(m,state);if(event_id!=ESCX_NO_EVENT){if(ESCX_EventQueuePush(&m->internal_queue,event_id,(void*)0)==ESCX_FALSE)return ESCX_STATUS_QUEUE_FULL;}state=Parent(m,state);}if(Parent(m,final_state)==ESCX_NO_STATE)m->finished=ESCX_TRUE;return ESCX_STATUS_PROCESSED;}

static ESCX_BOOL_T IsDescendantOf(const ESCX_StateMachine*m,ESCX_BYTE_T state,ESCX_BYTE_T ancestor){ESCX_BYTE_T depth=0U;while(state!=ESCX_NO_STATE){if(state==ancestor)return ESCX_TRUE;state=Parent(m,state);if(++depth>ESCX_MAX_HIERARCHY_DEPTH)break;}return ESCX_FALSE;}
static ESCX_BYTE_T *HistoryBits(ESCX_StateMachine*m,ESCX_BYTE_T slot){return m->history->bits+((unsigned int)slot*(unsigned int)m->history->bytes_per_slot);}
static void HistorySetBit(ESCX_BYTE_T*bits,ESCX_BYTE_T state){bits[state>>3]=(ESCX_BYTE_T)(bits[state>>3]|(ESCX_BYTE_T)(1U<<(state&7U)));}
static ESCX_BOOL_T HistoryHasBit(const ESCX_BYTE_T*bits,ESCX_BYTE_T state){return((bits[state>>3]&(ESCX_BYTE_T)(1U<<(state&7U)))!=0U)?ESCX_TRUE:ESCX_FALSE;}
static ESCX_ProcessStatus CaptureHistoryState(ESCX_StateMachine*m,ESCX_BYTE_T history_state){ESCX_BYTE_T slot,type,owner,i,state;ESCX_BYTE_T*bits;if(m->history==(ESCX_HistoryStorage*)0)return ESCX_STATUS_INVALID_ARGUMENT;slot=HistorySlot(m,history_state);if(slot>=m->history->slot_count)return ESCX_STATUS_INVALID_ARGUMENT;bits=HistoryBits(m,slot);memset(bits,0,m->history->bytes_per_slot);type=Type(m,history_state);owner=Parent(m,history_state);if(type==ESCX_STATE_HISTORY_SHALLOW){for(i=0U;i<m->active_atomic_count;++i){state=m->active_atomic_states[i];while((state!=ESCX_NO_STATE)&&(Parent(m,state)!=owner))state=Parent(m,state);if(state!=ESCX_NO_STATE)HistorySetBit(bits,state);}}else{for(state=1U;(ESCX_UINT16_T)state<m->definition->state_count;++state)if((IsActive(m,state)!=ESCX_FALSE)&&(IsDescendantOf(m,state,owner)!=ESCX_FALSE))HistorySetBit(bits,state);}m->history->valid[slot]=1U;return ESCX_STATUS_PROCESSED;}
static ESCX_ProcessStatus CaptureHistoriesForOwner(ESCX_StateMachine*m,ESCX_BYTE_T owner){ESCX_UINT16_T first,i;ESCX_BYTE_T count,child,type;first=FirstChild(m,owner);count=ChildCount(m,owner);if((ESCX_UINT16_T)(first+count)>m->definition->child_count)return ESCX_STATUS_INVALID_ARGUMENT;for(i=0U;i<count;++i){child=ESCX_ROM_READ_U8(&m->definition->children[first+i]);type=Type(m,child);if((type==ESCX_STATE_HISTORY_SHALLOW)||(type==ESCX_STATE_HISTORY_DEEP)){ESCX_ProcessStatus r;r=CaptureHistoryState(m,child);if(r!=ESCX_STATUS_PROCESSED)return r;}}return ESCX_STATUS_PROCESSED;}
static ESCX_ProcessStatus RestoreDeep(ESCX_StateMachine*m,ESCX_BYTE_T owner,const ESCX_BYTE_T*bits,void*d){ESCX_BYTE_T depth,state,type;for(depth=1U;depth<=ESCX_MAX_HIERARCHY_DEPTH;++depth){for(state=1U;(ESCX_UINT16_T)state<m->definition->state_count;++state){if((HistoryHasBit(bits,state)!=ESCX_FALSE)&&(IsDescendantOf(m,state,owner)!=ESCX_FALSE)&&(StateU8(m,state,OFFSETOF_STATE(depth))==depth)){if(IsActive(m,state)==ESCX_FALSE){SetActive(m,state,ESCX_TRUE);CallState(m,state,d,0);CallState(m,state,d,2);}type=Type(m,state);if((type==ESCX_STATE_ATOMIC)||(type==ESCX_STATE_FINAL)){if(AddAtomic(m,state)==ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;if(type==ESCX_STATE_FINAL){ESCX_ProcessStatus r;r=NotifyFinalEntered(m,state);if(r!=ESCX_STATUS_PROCESSED)return r;}}}}}return ESCX_STATUS_PROCESSED;}
static ESCX_ProcessStatus RestoreHistory(ESCX_StateMachine*m,ESCX_BYTE_T history_state,void*d){ESCX_BYTE_T slot,owner,state;ESCX_UINT16_T default_plan;ESCX_BYTE_T*bits;if(m->history==(ESCX_HistoryStorage*)0)return ESCX_STATUS_INVALID_ARGUMENT;slot=HistorySlot(m,history_state);if(slot>=m->history->slot_count)return ESCX_STATUS_INVALID_ARGUMENT;default_plan=HistoryDefaultPlan(m,history_state);if(m->history->valid[slot]==0U)return RunPlan(m,default_plan,d,ESCX_TRUE);bits=HistoryBits(m,slot);owner=Parent(m,history_state);if(Type(m,history_state)==ESCX_STATE_HISTORY_SHALLOW){for(state=1U;(ESCX_UINT16_T)state<m->definition->state_count;++state)if(HistoryHasBit(bits,state)!=ESCX_FALSE){ESCX_ProcessStatus r;r=EnterStateTree(m,state,d,0U);if(r!=ESCX_STATUS_PROCESSED)return r;}return ESCX_STATUS_PROCESSED;}return RestoreDeep(m,owner,bits,d);}
static ESCX_ProcessStatus CaptureHistoriesForExitPlan(ESCX_StateMachine*m,ESCX_UINT16_T pid){const ESCX_StatePlan*p;ESCX_UINT16_T first,i;ESCX_BYTE_T count,state,type;if(pid==ESCX_NO_PLAN)return ESCX_STATUS_PROCESSED;
 if(pid>=m->definition->plan_count)return ESCX_STATUS_INVALID_ARGUMENT;
p=&m->definition->plans[pid];first=ESCX_ROM_READ_U16(&p->first_state);count=ESCX_ROM_READ_U8(&p->state_count);for(i=0U;i<count;++i){state=ESCX_ROM_READ_U8(&m->definition->plan_states[first+i]);type=Type(m,state);if((type==ESCX_STATE_COMPOUND)||(type==ESCX_STATE_PARALLEL)){ESCX_ProcessStatus r;r=CaptureHistoriesForOwner(m,state);if(r!=ESCX_STATUS_PROCESSED)return r;}}return ESCX_STATUS_PROCESSED;}
static ESCX_ProcessStatus RunPlan(ESCX_StateMachine*m,ESCX_UINT16_T pid,void*d,ESCX_BOOL_T entering){const ESCX_StatePlan*p;ESCX_UINT16_T first,i;ESCX_BYTE_T count,s;if(pid==ESCX_NO_PLAN)return ESCX_STATUS_PROCESSED;
 if(pid>=m->definition->plan_count)return ESCX_STATUS_INVALID_ARGUMENT;
p=&m->definition->plans[pid];first=ESCX_ROM_READ_U16(&p->first_state);count=ESCX_ROM_READ_U8(&p->state_count);if((ESCX_UINT16_T)(first+count)>m->definition->plan_state_count)return ESCX_STATUS_INVALID_ARGUMENT;for(i=0U;i<count;++i){s=ESCX_ROM_READ_U8(&m->definition->plan_states[first+i]);if(entering!=ESCX_FALSE){if((Type(m,s)==ESCX_STATE_HISTORY_SHALLOW)||(Type(m,s)==ESCX_STATE_HISTORY_DEEP)){ESCX_ProcessStatus hr;hr=RestoreHistory(m,s,d);if(hr!=ESCX_STATUS_PROCESSED)return hr;continue;}SetActive(m,s,ESCX_TRUE);CallState(m,s,d,0);CallState(m,s,d,2);if((Type(m,s)==ESCX_STATE_ATOMIC)||(Type(m,s)==ESCX_STATE_FINAL)){if(AddAtomic(m,s)==ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;if(Type(m,s)==ESCX_STATE_FINAL){ESCX_ProcessStatus cr;cr=NotifyFinalEntered(m,s);if(cr!=ESCX_STATUS_PROCESSED)return cr;}}}else{CallState(m,s,d,1);SetActive(m,s,ESCX_FALSE);RemoveAtomic(m,s);}}return ESCX_STATUS_PROCESSED;}
static ESCX_BOOL_T PlanHasState(const ESCX_StateMachine*m,ESCX_UINT16_T pid,ESCX_BYTE_T state){const ESCX_StatePlan*p;ESCX_UINT16_T first,i;ESCX_BYTE_T count;if(pid==ESCX_NO_PLAN)return ESCX_FALSE;if(pid>=m->definition->plan_count)return ESCX_FALSE;p=&m->definition->plans[pid];first=ESCX_ROM_READ_U16(&p->first_state);count=ESCX_ROM_READ_U8(&p->state_count);for(i=0U;i<count;++i)if(ESCX_ROM_READ_U8(&m->definition->plan_states[first+i])==state)return ESCX_TRUE;return ESCX_FALSE;}
static ESCX_BOOL_T Conflict(const ESCX_StateMachine*m,ESCX_UINT16_T a,ESCX_UINT16_T b){const ESCX_Transition*ta;const ESCX_Transition*tb;ESCX_UINT16_T pa,pb,i;const ESCX_StatePlan*p;ESCX_BYTE_T count,state;if(a==b)return ESCX_TRUE;ta=&m->definition->transitions[a];tb=&m->definition->transitions[b];pa=ESCX_ROM_READ_U16(&ta->exit_plan);pb=ESCX_ROM_READ_U16(&tb->exit_plan);if((pa==ESCX_NO_PLAN)||(pb==ESCX_NO_PLAN))return ESCX_FALSE;p=&m->definition->plans[pa];count=ESCX_ROM_READ_U8(&p->state_count);i=ESCX_ROM_READ_U16(&p->first_state);while(count--!=0U){state=ESCX_ROM_READ_U8(&m->definition->plan_states[i++]);if(PlanHasState(m,pb,state))return ESCX_TRUE;}return ESCX_FALSE;}
static ESCX_BYTE_T DepthOfTransition(const ESCX_StateMachine*m,ESCX_UINT16_T t){return ESCX_ROM_READ_U8(&m->definition->transitions[t].source_depth);}
static ESCX_BYTE_T OrderOfTransition(const ESCX_StateMachine*m,ESCX_UINT16_T t){return ESCX_ROM_READ_U8(&m->definition->transitions[t].document_order);}
static void WorkSetClear(ESCX_BYTE_T *bits){memset(bits,0,ESCX_ACTIVE_BYTES(ESCX_MAX_STATES));}
static void WorkSetAdd(ESCX_BYTE_T *bits,ESCX_BYTE_T state){bits[state>>3]=(ESCX_BYTE_T)(bits[state>>3]|(ESCX_BYTE_T)(1U<<(state&7U)));}
static ESCX_BOOL_T WorkSetHas(const ESCX_BYTE_T *bits,ESCX_BYTE_T state){return((bits[state>>3]&(ESCX_BYTE_T)(1U<<(state&7U)))!=0U)?ESCX_TRUE:ESCX_FALSE;}
static ESCX_ProcessStatus AddPlanToWorkSet(const ESCX_StateMachine*m,ESCX_UINT16_T pid,ESCX_BYTE_T*bits)
{
 const ESCX_StatePlan*p;ESCX_UINT16_T first,i;ESCX_BYTE_T count,state;
 if(pid==ESCX_NO_PLAN)return ESCX_STATUS_PROCESSED;
 if(pid>=m->definition->plan_count)return ESCX_STATUS_INVALID_ARGUMENT;
 p=&m->definition->plans[pid];first=ESCX_ROM_READ_U16(&p->first_state);count=ESCX_ROM_READ_U8(&p->state_count);
 if((ESCX_UINT16_T)(first+count)>m->definition->plan_state_count)return ESCX_STATUS_INVALID_ARGUMENT;
 for(i=0U;i<count;++i){state=ESCX_ROM_READ_U8(&m->definition->plan_states[first+i]);if(state!=ESCX_NO_STATE)WorkSetAdd(bits,state);}
 return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus ExitWorkSet(ESCX_StateMachine*m,const ESCX_BYTE_T*bits,void*d)
{
 ESCX_BYTE_T depth,state,best,best_order;
 for(depth=ESCX_MAX_HIERARCHY_DEPTH;depth!=0U;--depth){
  for(;;){best=ESCX_NO_STATE;best_order=0U;
   for(state=1U;(ESCX_UINT16_T)state<m->definition->state_count;++state)if((WorkSetHas(bits,state)!=ESCX_FALSE)&&(IsActive(m,state)!=ESCX_FALSE)&&(StateDepth(m,state)==depth)){if((best==ESCX_NO_STATE)||(StateOrder(m,state)>best_order)){best=state;best_order=StateOrder(m,state);}}
   if(best==ESCX_NO_STATE)break;
   CallState(m,best,d,1);SetActive(m,best,ESCX_FALSE);RemoveAtomic(m,best);((ESCX_BYTE_T*)bits)[best>>3]=(ESCX_BYTE_T)(((ESCX_BYTE_T*)bits)[best>>3]&((ESCX_BYTE_T)~(ESCX_BYTE_T)(1U<<(best&7U))));
  }
 }
 return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus EnterWorkSet(ESCX_StateMachine*m,const ESCX_BYTE_T*bits,void*d)
{
 ESCX_BYTE_T depth,state,best,best_order,type;ESCX_ProcessStatus r;
 for(depth=0U;depth<=ESCX_MAX_HIERARCHY_DEPTH;++depth){
  for(;;){best=ESCX_NO_STATE;best_order=255U;
   for(state=1U;(ESCX_UINT16_T)state<m->definition->state_count;++state)if((WorkSetHas(bits,state)!=ESCX_FALSE)&&(StateDepth(m,state)==depth)){if((best==ESCX_NO_STATE)||(StateOrder(m,state)<best_order)){best=state;best_order=StateOrder(m,state);}}
   if(best==ESCX_NO_STATE)break;
   ((ESCX_BYTE_T*)bits)[best>>3]=(ESCX_BYTE_T)(((ESCX_BYTE_T*)bits)[best>>3]&((ESCX_BYTE_T)~(ESCX_BYTE_T)(1U<<(best&7U))));type=Type(m,best);
   if((type==ESCX_STATE_HISTORY_SHALLOW)||(type==ESCX_STATE_HISTORY_DEEP)){r=RestoreHistory(m,best,d);if(r!=ESCX_STATUS_PROCESSED)return r;continue;}
   if(IsActive(m,best)==ESCX_FALSE){SetActive(m,best,ESCX_TRUE);CallState(m,best,d,0);CallState(m,best,d,2);}
   if((type==ESCX_STATE_ATOMIC)||(type==ESCX_STATE_FINAL)){if(AddAtomic(m,best)==ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;if(type==ESCX_STATE_FINAL){r=NotifyFinalEntered(m,best);if(r!=ESCX_STATUS_PROCESSED)return r;}}
  }
  if(depth==ESCX_MAX_HIERARCHY_DEPTH)break;
 }
 return ESCX_STATUS_PROCESSED;
}
static void SortTransitionsByDocumentOrder(const ESCX_StateMachine*m,ESCX_UINT16_T*ids,ESCX_BYTE_T count)
{ESCX_BYTE_T i,j;ESCX_UINT16_T value;for(i=1U;i<count;++i){value=ids[i];j=i;while((j!=0U)&&(OrderOfTransition(m,ids[j-1U])>OrderOfTransition(m,value))){ids[j]=ids[j-1U];--j;}ids[j]=value;}}
static ESCX_ProcessStatus ExecuteSet(ESCX_StateMachine*m,ESCX_UINT16_T*ids,ESCX_BYTE_T count,void*d)
{
 ESCX_BYTE_T exit_bits[ESCX_ACTIVE_BYTES(ESCX_MAX_STATES)];ESCX_BYTE_T entry_bits[ESCX_ACTIVE_BYTES(ESCX_MAX_STATES)];ESCX_BYTE_T i,type;ESCX_UINT16_T xp,ep;ESCX_ActionFunction action;ESCX_ProcessStatus r;const ESCX_Transition*t;
 if(count==0U)return ESCX_STATUS_IGNORED;
 WorkSetClear(exit_bits);WorkSetClear(entry_bits);
 for(i=0U;i<count;++i){if(ids[i]>=m->definition->transition_count)return ESCX_STATUS_INVALID_ARGUMENT;t=&m->definition->transitions[ids[i]];type=ESCX_ROM_READ_U8(&t->type);if(type!=ESCX_TRANSITION_TARGETLESS){xp=ESCX_ROM_READ_U16(&t->exit_plan);ep=ESCX_ROM_READ_U16(&t->entry_plan);r=CaptureHistoriesForExitPlan(m,xp);if(r!=ESCX_STATUS_PROCESSED)return r;r=AddPlanToWorkSet(m,xp,exit_bits);if(r!=ESCX_STATUS_PROCESSED)return r;r=AddPlanToWorkSet(m,ep,entry_bits);if(r!=ESCX_STATUS_PROCESSED)return r;}}
 r=ExitWorkSet(m,exit_bits,d);if(r!=ESCX_STATUS_PROCESSED)return r;SortTransitionsByDocumentOrder(m,ids,count);
 for(i=0U;i<count;++i){t=&m->definition->transitions[ids[i]];action=ReadAction(&t->action);if(action!=(ESCX_ActionFunction)0)action(m,d);}
 return EnterWorkSet(m,entry_bits,d);
}
static ESCX_ProcessStatus ProcessEvent(ESCX_StateMachine*m,ESCX_BYTE_T e,void*d){ESCX_UINT16_T selected[ESCX_MAX_ENABLED_TRANSITIONS];ESCX_BYTE_T selected_count=0U,leaf,s,depth,i,j;ESCX_UINT16_T sid,tid;ESCX_SelectorFunction f;if(e>=m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;m->current_event=e;for(i=0U;i<m->active_atomic_count;++i){leaf=m->active_atomic_states[i];s=leaf;depth=0U;while(s!=ESCX_NO_STATE){sid=FindSelector(m,s,e);if((sid!=ESCX_NO_SELECTOR)&&(sid<m->definition->selector_count)){f=ReadSelector(&m->definition->selectors[sid]);if(f==(ESCX_SelectorFunction)0)return ESCX_STATUS_INVALID_ARGUMENT;tid=f(m,d);if(tid!=ESCX_NO_TRANSITION){ESCX_BOOL_T keep=ESCX_TRUE;for(j=0U;j<selected_count;++j){if(Conflict(m,tid,selected[j])){if((DepthOfTransition(m,tid)>DepthOfTransition(m,selected[j]))||((DepthOfTransition(m,tid)==DepthOfTransition(m,selected[j]))&&(OrderOfTransition(m,tid)<OrderOfTransition(m,selected[j])))){selected[j]=tid;}keep=ESCX_FALSE;break;}}if(keep!=ESCX_FALSE){if(selected_count>=ESCX_MAX_ENABLED_TRANSITIONS)return ESCX_STATUS_INVALID_ARGUMENT;selected[selected_count++]=tid;}break;}}s=Parent(m,s);if(++depth>ESCX_MAX_HIERARCHY_DEPTH)return ESCX_STATUS_HIERARCHY_LIMIT;}}return ExecuteSet(m,selected,selected_count,d);}
static ESCX_ProcessStatus ProcessEventless(ESCX_StateMachine*m){ESCX_BYTE_T i,s,depth;ESCX_UINT16_T sid,tid;ESCX_SelectorFunction f;for(i=0U;i<m->active_atomic_count;++i){s=m->active_atomic_states[i];depth=0U;while(s!=ESCX_NO_STATE){sid=EventlessSelector(m,s);if((sid!=ESCX_NO_SELECTOR)&&(sid<m->definition->selector_count)){f=ReadSelector(&m->definition->selectors[sid]);if(f==(ESCX_SelectorFunction)0)return ESCX_STATUS_INVALID_ARGUMENT;tid=f(m,(void*)0);if(tid!=ESCX_NO_TRANSITION){m->current_event=ESCX_NO_EVENT;{ESCX_UINT16_T one[1];one[0]=tid;return ExecuteSet(m,one,1U,(void*)0);}}}s=Parent(m,s);if(++depth>ESCX_MAX_HIERARCHY_DEPTH)return ESCX_STATUS_HIERARCHY_LIMIT;}}return ESCX_STATUS_IGNORED;}
static ESCX_ProcessStatus Stabilize(ESCX_StateMachine*m){ESCX_UINT16_T steps=0U;ESCX_ProcessStatus r;ESCX_QueuedEvent q;for(;;){r=ProcessEventless(m);if(r==ESCX_STATUS_PROCESSED){if(++steps>ESCX_MAX_MICROSTEPS)return ESCX_STATUS_MICROSTEP_LIMIT;continue;}if(r!=ESCX_STATUS_IGNORED)return r;if(ESCX_EventQueuePop(&m->internal_queue,&q)!=ESCX_FALSE){
#if ESCX_ENABLE_EVENT_DATA
r=ProcessEvent(m,q.event_id,q.event_data);
#else
r=ProcessEvent(m,q.event_id,(void*)0);
#endif
if((r!=ESCX_STATUS_PROCESSED)&&(r!=ESCX_STATUS_IGNORED)){return r;}
if(++steps>ESCX_MAX_MICROSTEPS){return ESCX_STATUS_MICROSTEP_LIMIT;}
continue;}return ESCX_STATUS_PROCESSED;}}
static ESCX_ProcessStatus Macrostep(ESCX_StateMachine*m,ESCX_BYTE_T e,void*d){ESCX_ProcessStatus r;r=ForwardExternalToInvokes(m,e,d);if(r!=ESCX_STATUS_PROCESSED)return r;r=ProcessEvent(m,e,d);if((r!=ESCX_STATUS_PROCESSED)&&(r!=ESCX_STATUS_IGNORED))return r;r=Stabilize(m);return r;}
ESCX_ProcessStatus ESCX_StateMachineStart(ESCX_StateMachine*m){ESCX_ProcessStatus r;if((m==(ESCX_StateMachine*)0)||(m->definition==(const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT;if(m->running!=ESCX_FALSE)return ESCX_STATUS_IGNORED;m->finished=ESCX_FALSE;m->running=ESCX_TRUE;r=EnterInitial(m,m->definition->initial_state,(void*)0);if(r!=ESCX_STATUS_PROCESSED)return r;m->processing=ESCX_TRUE;r=Stabilize(m);m->processing=ESCX_FALSE;return r;}
ESCX_ProcessStatus ESCX_StateMachineRaiseInternal(ESCX_StateMachine*m,ESCX_BYTE_T e,void*d){if((m==(ESCX_StateMachine*)0)||(m->running==ESCX_FALSE))return ESCX_STATUS_NOT_RUNNING;if(e>=m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;return ESCX_EventQueuePush(&m->internal_queue,e,d)?ESCX_STATUS_PROCESSED:ESCX_STATUS_QUEUE_FULL;}
ESCX_ProcessStatus ESCX_StateMachineSend(ESCX_StateMachine*m,ESCX_BYTE_T e,void*d){ESCX_ProcessStatus r;ESCX_QueuedEvent q;if((m==(ESCX_StateMachine*)0)||(m->definition==(const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT;if(m->running==ESCX_FALSE)return ESCX_STATUS_NOT_RUNNING;if(e>=m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;if(m->processing!=ESCX_FALSE)return ESCX_EventQueuePush(&m->external_queue,e,d)?ESCX_STATUS_PROCESSED:ESCX_STATUS_QUEUE_FULL;m->processing=ESCX_TRUE;r=Macrostep(m,e,d);while((r==ESCX_STATUS_PROCESSED)&&(ESCX_EventQueuePop(&m->external_queue,&q)!=ESCX_FALSE)){
#if ESCX_ENABLE_EVENT_DATA
r=Macrostep(m,q.event_id,q.event_data);
#else
r=Macrostep(m,q.event_id,(void*)0);
#endif
}m->processing=ESCX_FALSE;return r;}
void ESCX_StateMachineStop(ESCX_StateMachine*m){ESCX_BYTE_T state;if((m==(ESCX_StateMachine*)0)||(m->running==ESCX_FALSE))return;while(m->active_atomic_count!=0U){state=m->active_atomic_states[m->active_atomic_count-1U];CallState(m,state,(void*)0,1);SetActive(m,state,ESCX_FALSE);--m->active_atomic_count;}memset(m->active_bits,0,m->active_byte_count);m->running=ESCX_FALSE;m->processing=ESCX_FALSE;ESCX_EventQueueInit(&m->internal_queue);ESCX_EventQueueInit(&m->external_queue);
#if ESCX_ENABLE_DELAYED_SENDS
{ESCX_BYTE_T di;for(di=0U;di<ESCX_MAX_DELAYED_SENDS;++di){if((m->delayed_sends[di].active!=ESCX_FALSE)&&(m->cancel_timer!=(ESCX_CancelTimerFunction)0))(void)m->cancel_timer(m,m->delayed_sends[di].timer_id,m->timer_context);m->delayed_sends[di].active=ESCX_FALSE;m->delayed_sends[di].timer_id=ESCX_INVALID_TIMER_ID;}}
#endif
}
static ESCX_ProcessStatus ReportErrorInternal(ESCX_StateMachine *,ESCX_ErrorCode,ESCX_ProcessStatus);
static ESCX_ProcessStatus DispatchSend(ESCX_StateMachine *,ESCX_BYTE_T,void *,ESCX_BYTE_T,ESCX_BYTE_T);
ESCX_ProcessStatus ESCX_StateMachineConfigureTimers(ESCX_StateMachine*m,ESCX_StartTimerFunction st,ESCX_CancelTimerFunction ct,void*ctx){
#if ESCX_ENABLE_TIMERS
if(m==(ESCX_StateMachine*)0)return ESCX_STATUS_INVALID_ARGUMENT;
m->start_timer=st;m->cancel_timer=ct;m->timer_context=ctx;
return ESCX_STATUS_PROCESSED;
#else
(void)m;(void)st;(void)ct;(void)ctx;return ESCX_STATUS_IGNORED;
#endif
}
ESCX_ProcessStatus ESCX_StateMachineTimerExpired(ESCX_StateMachine*m,ESCX_TimerId id){
#if ESCX_ENABLE_TIMERS && ESCX_ENABLE_DELAYED_SENDS
ESCX_BYTE_T i;ESCX_DelayedSend pending;if(m==(ESCX_StateMachine*)0)return ESCX_STATUS_INVALID_ARGUMENT;if(m->running==ESCX_FALSE)return ESCX_STATUS_NOT_RUNNING;if(id==ESCX_INVALID_TIMER_ID)return ESCX_STATUS_IGNORED;
for(i=0U;i<ESCX_MAX_DELAYED_SENDS;++i)if((m->delayed_sends[i].active!=ESCX_FALSE)&&(m->delayed_sends[i].timer_id==id)){pending=m->delayed_sends[i];m->delayed_sends[i].active=ESCX_FALSE;m->delayed_sends[i].timer_id=ESCX_INVALID_TIMER_ID;return DispatchSend(m,pending.event_id,pending.event_data,pending.target,pending.target_id);}
return ESCX_STATUS_IGNORED;
#else
(void)m;(void)id;return ESCX_STATUS_IGNORED;
#endif
}
ESCX_ProcessStatus ESCX_StateMachineConfigureParent(ESCX_StateMachine *m,ESCX_StateMachine *parent,ESCX_BYTE_T parent_invoke_id)
{
    if(m==(ESCX_StateMachine*)0)return ESCX_STATUS_INVALID_ARGUMENT;
    if((parent==(ESCX_StateMachine*)0)&&(parent_invoke_id!=ESCX_NO_INVOKE))return ESCX_STATUS_INVALID_ARGUMENT;
    m->parent_machine=parent;m->parent_invoke_id=parent_invoke_id;return ESCX_STATUS_PROCESSED;
}

static ESCX_ProcessStatus DispatchSend(ESCX_StateMachine *m,ESCX_BYTE_T event_id,void *data,ESCX_BYTE_T target,ESCX_BYTE_T target_id)
{
    ESCX_InvokeDescriptor d;int status;
    if((m==(ESCX_StateMachine*)0)||(event_id>=m->definition->event_count))return ESCX_STATUS_INVALID_EVENT;
    if(target==ESCX_SEND_TARGET_INTERNAL)return ESCX_StateMachineRaiseInternal(m,event_id,data);
    if(target==ESCX_SEND_TARGET_SELF)return ESCX_StateMachineSend(m,event_id,data);
    if(target==ESCX_SEND_TARGET_PARENT){
        if((m->parent_machine==(ESCX_StateMachine*)0)||(m->parent_invoke_id==ESCX_NO_INVOKE))return ReportErrorInternal(m,ESCX_ERROR_COMMUNICATION,ESCX_STATUS_NO_PARENT);
        return ESCX_StateMachineReceiveInvokeEvent(m->parent_machine,m->parent_invoke_id,event_id,data);
    }
    if(target==ESCX_SEND_TARGET_INVOKE){
        if((target_id>=m->invoke_count)||(m->invoke_instances==(ESCX_InvokeInstance*)0)||(m->invoke_instances[target_id].active==ESCX_FALSE))return ReportErrorInternal(m,ESCX_ERROR_COMMUNICATION,ESCX_STATUS_INVALID_TARGET);
        ReadInvokeDescriptor(m,target_id,&d);if(d.forward==(ESCX_InvokeForwardFunction)0)return ReportErrorInternal(m,ESCX_ERROR_COMMUNICATION,ESCX_STATUS_INVALID_TARGET);
        status=d.forward(m,d.child_storage,event_id,data);if(status!=(int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m,ESCX_ERROR_COMMUNICATION,(ESCX_ProcessStatus)status);return ESCX_STATUS_PROCESSED;
    }
    return ESCX_STATUS_INVALID_TARGET;
}

ESCX_ProcessStatus ESCX_StateMachineSendRequest(ESCX_StateMachine *m,const ESCX_SendRequest *request)
{
#if ESCX_ENABLE_DELAYED_SENDS
    ESCX_BYTE_T i;
#endif
    if((m==(ESCX_StateMachine*)0)||(request==(const ESCX_SendRequest*)0))return ESCX_STATUS_INVALID_ARGUMENT;
    if(m->running==ESCX_FALSE)return ESCX_STATUS_NOT_RUNNING;
    if(request->event_id>=m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;
    if(request->delay_ticks==0UL)return DispatchSend(m,request->event_id,request->event_data,request->target,request->target_id);
#if ESCX_ENABLE_DELAYED_SENDS
    if(m->start_timer==(ESCX_StartTimerFunction)0)return ReportErrorInternal(m,ESCX_ERROR_EXECUTION,ESCX_STATUS_TIMER_START_FAILED);
    for(i=0U;i<ESCX_MAX_DELAYED_SENDS;++i)if(m->delayed_sends[i].active==ESCX_FALSE){
        ESCX_TimerId id;m->delayed_sends[i].event_id=request->event_id;m->delayed_sends[i].target=request->target;m->delayed_sends[i].target_id=request->target_id;m->delayed_sends[i].send_id=request->send_id;m->delayed_sends[i].event_data=request->event_data;m->delayed_sends[i].timer_id=ESCX_INVALID_TIMER_ID;
        id=m->start_timer(m,request->delay_ticks,m->timer_context);if(id==ESCX_INVALID_TIMER_ID)return ReportErrorInternal(m,ESCX_ERROR_EXECUTION,ESCX_STATUS_TIMER_START_FAILED);m->delayed_sends[i].timer_id=id;m->delayed_sends[i].active=ESCX_TRUE;return ESCX_STATUS_PROCESSED;
    }
    return ESCX_STATUS_QUEUE_FULL;
#else
    return ESCX_STATUS_INVALID_TARGET;
#endif
}

ESCX_ProcessStatus ESCX_StateMachineCancelSend(ESCX_StateMachine *m,ESCX_UINT16_T send_id)
{
#if ESCX_ENABLE_DELAYED_SENDS
    ESCX_BYTE_T i;if((m==(ESCX_StateMachine*)0)||(send_id==ESCX_NO_SEND_ID))return ESCX_STATUS_INVALID_ARGUMENT;
    for(i=0U;i<ESCX_MAX_DELAYED_SENDS;++i)if((m->delayed_sends[i].active!=ESCX_FALSE)&&(m->delayed_sends[i].send_id==send_id)){ESCX_TimerId id=m->delayed_sends[i].timer_id;ESCX_BOOL_T ok=ESCX_TRUE;m->delayed_sends[i].active=ESCX_FALSE;m->delayed_sends[i].timer_id=ESCX_INVALID_TIMER_ID;if(m->cancel_timer!=(ESCX_CancelTimerFunction)0)ok=m->cancel_timer(m,id,m->timer_context);return(ok!=ESCX_FALSE)?ESCX_STATUS_PROCESSED:ReportErrorInternal(m,ESCX_ERROR_EXECUTION,ESCX_STATUS_TIMER_CANCEL_FAILED);}
    return ESCX_STATUS_SEND_NOT_FOUND;
#else
    (void)m;(void)send_id;return ESCX_STATUS_SEND_NOT_FOUND;
#endif
}

static ESCX_ProcessStatus ReportErrorInternal(ESCX_StateMachine *m,ESCX_ErrorCode code,ESCX_ProcessStatus cause)
{
#if ESCX_ENABLE_ERROR_EVENTS
    ESCX_BYTE_T event_id;
    ESCX_ProcessStatus stabilization_status;
    if(m==(ESCX_StateMachine*)0)return ESCX_STATUS_INVALID_ARGUMENT;
    m->last_error=(ESCX_BYTE_T)code;
    m->last_error_status=cause;
    if(m->reporting_error!=ESCX_FALSE)return cause;
    if(code==ESCX_ERROR_COMMUNICATION)event_id=m->error_communication_event;
    else event_id=m->error_execution_event;
    if((event_id==ESCX_NO_EVENT)||(m->running==ESCX_FALSE))return cause;
    m->reporting_error=ESCX_TRUE;
    if(ESCX_EventQueuePush(&m->internal_queue,event_id,(void*)0)==ESCX_FALSE){
        m->reporting_error=ESCX_FALSE;
        return cause;
    }
    if(m->processing==ESCX_FALSE){
        m->processing=ESCX_TRUE;
        stabilization_status=Stabilize(m);
        m->processing=ESCX_FALSE;
        m->reporting_error=ESCX_FALSE;
        if((stabilization_status!=ESCX_STATUS_PROCESSED)&&(stabilization_status!=ESCX_STATUS_IGNORED))return stabilization_status;
    }else m->reporting_error=ESCX_FALSE;
    return cause;
#else
    if(m!=(ESCX_StateMachine*)0){m->last_error=(ESCX_BYTE_T)code;m->last_error_status=cause;}
    return cause;
#endif
}
ESCX_ProcessStatus ESCX_StateMachineReportError(ESCX_StateMachine *m,ESCX_ErrorCode code,ESCX_ProcessStatus cause)
{
    if((code==ESCX_ERROR_NONE)||(code>ESCX_ERROR_PLATFORM))return ESCX_STATUS_INVALID_ARGUMENT;
    return ReportErrorInternal(m,code,cause);
}
ESCX_ProcessStatus ESCX_StateMachineConfigureErrorEvents(ESCX_StateMachine *m,ESCX_BYTE_T execution_event,ESCX_BYTE_T communication_event)
{
    if((m==(ESCX_StateMachine*)0)||(m->definition==(const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT;
    if((execution_event!=ESCX_NO_EVENT)&&(execution_event>=m->definition->event_count))return ESCX_STATUS_INVALID_EVENT;
    if((communication_event!=ESCX_NO_EVENT)&&(communication_event>=m->definition->event_count))return ESCX_STATUS_INVALID_EVENT;
    m->error_execution_event=execution_event;m->error_communication_event=communication_event;return ESCX_STATUS_PROCESSED;
}

ESCX_BYTE_T ESCX_StateMachineGetLastError(const ESCX_StateMachine *m)
{
    return (m==(const ESCX_StateMachine*)0)?(ESCX_BYTE_T)ESCX_ERROR_PLATFORM:m->last_error;
}

ESCX_ProcessStatus ESCX_StateMachineGetLastErrorStatus(const ESCX_StateMachine *m)
{
    return (m==(const ESCX_StateMachine*)0)?ESCX_STATUS_INVALID_ARGUMENT:m->last_error_status;
}

void ESCX_StateMachineClearError(ESCX_StateMachine *m)
{
    if(m!=(ESCX_StateMachine*)0){m->last_error=ESCX_ERROR_NONE;m->last_error_status=ESCX_STATUS_PROCESSED;m->reporting_error=ESCX_FALSE;}
}
