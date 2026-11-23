#include "InvokeMachine.h"
static int ChildStart(ESCX_StateMachine*m,void*storage){InvokeData*d;(void)storage;d=(InvokeData*)m->instance;++d->starts;return (int)ESCX_STATUS_PROCESSED;}
static void ChildCancel(ESCX_StateMachine*m,void*storage){InvokeData*d;(void)storage;d=(InvokeData*)m->instance;++d->cancels;}
static int ChildForward(ESCX_StateMachine*m,void*storage,ESCX_BYTE_T event_id,void*event_data){InvokeData*d;(void)storage;(void)event_data;d=(InvokeData*)m->instance;++d->forwards;d->last_event=event_id;return (int)ESCX_STATUS_PROCESSED;}
static void ChildFinalize(ESCX_StateMachine*m,void*storage,ESCX_BYTE_T event_id,void*event_data){InvokeData*d;(void)storage;(void)event_data;d=(InvokeData*)m->instance;++d->finalizes;d->last_event=event_id;}
ESCX_SELECTOR_BEGIN(SDone)(void)machine;(void)event_data;ESCX_SELECTOR_RETURN(0U)ESCX_SELECTOR_END()
ESCX_STATES_BEGIN(InvokeMachine)
 ESCX_STATE((ESCX_StateFunction)0,(ESCX_StateFunction)0,(ESCX_StateFunction)0)
 ESCX_STATE_EX((ESCX_StateFunction)0,(ESCX_StateFunction)0,(ESCX_StateFunction)0,0,0,2,IM_Invoking,0,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR)
 ESCX_STATE_EX((ESCX_StateFunction)0,(ESCX_StateFunction)0,(ESCX_StateFunction)0,IM_Root,0,0,0,1,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR)
 ESCX_STATE_EX((ESCX_StateFunction)0,(ESCX_StateFunction)0,(ESCX_StateFunction)0,IM_Root,0,0,0,1,3,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR)
ESCX_STATES_END()
ESCX_CHILDREN_BEGIN(InvokeMachine) ESCX_CHILD(IM_Invoking) ESCX_CHILD(IM_Done) ESCX_CHILDREN_END()
ESCX_PLAN_STATES_BEGIN(InvokeMachine) ESCX_PLAN_STATE(IM_Invoking) ESCX_PLAN_STATE(IM_Done) ESCX_PLAN_STATES_END()
ESCX_PLANS_BEGIN(InvokeMachine) ESCX_PLAN(0,1) ESCX_PLAN(1,1) ESCX_PLANS_END()
ESCX_TRANSITIONS_BEGIN(InvokeMachine) ESCX_TRANSITION_META(0,1,(ESCX_ActionFunction)0,ESCX_TRANSITION_EXTERNAL,IM_Invoking,1,1) ESCX_TRANSITIONS_END()
ESCX_SELECTORS_BEGIN(InvokeMachine) ESCX_SELECTOR(SDone) ESCX_SELECTORS_END()
ESCX_STATE_BASE_BEGIN(InvokeMachine) ESCX_INDEX_U16(0) ESCX_INDEX_U16(4) ESCX_INDEX_U16(8) ESCX_INDEX_U16(12) ESCX_STATE_BASE_END()
#define N ESCX_NO_OWNER
ESCX_SLOT_OWNER_BEGIN(InvokeMachine)
 ESCX_INDEX_U8(N) ESCX_INDEX_U8(N) ESCX_INDEX_U8(N) ESCX_INDEX_U8(N)
 ESCX_INDEX_U8(N) ESCX_INDEX_U8(N) ESCX_INDEX_U8(N) ESCX_INDEX_U8(N)
 ESCX_INDEX_U8(N) ESCX_INDEX_U8(IM_Invoking) ESCX_INDEX_U8(N) ESCX_INDEX_U8(N)
 ESCX_INDEX_U8(N) ESCX_INDEX_U8(N) ESCX_INDEX_U8(N) ESCX_INDEX_U8(N)
ESCX_SLOT_OWNER_END()
#undef N
#define X ESCX_NO_SELECTOR
ESCX_SLOT_SELECTOR_BEGIN(InvokeMachine)
 ESCX_INDEX_U16(X) ESCX_INDEX_U16(X) ESCX_INDEX_U16(X) ESCX_INDEX_U16(X)
 ESCX_INDEX_U16(X) ESCX_INDEX_U16(X) ESCX_INDEX_U16(X) ESCX_INDEX_U16(X)
 ESCX_INDEX_U16(X) ESCX_INDEX_U16(0) ESCX_INDEX_U16(X) ESCX_INDEX_U16(X)
 ESCX_INDEX_U16(X) ESCX_INDEX_U16(X) ESCX_INDEX_U16(X) ESCX_INDEX_U16(X)
ESCX_SLOT_SELECTOR_END()
#undef X
ESCX_INVOKES_BEGIN(InvokeMachine)
 ESCX_INVOKE(IM_Invoking,IM_DoneInvoke,ESCX_TRUE,(void*)0,ChildStart,ChildCancel,ChildForward,ChildFinalize)
ESCX_INVOKES_END()
ESCX_DEFINE_MACHINE(InvokeMachine,IM_Root);
ESCX_IMPLEMENT_MACHINE(InvokeMachine)
ESCX_ProcessStatus InvokeMachine_Configure(ESCX_StateMachine*m,ESCX_InvokeInstance*i){return ESCX_StateMachineConfigureInvokes(m,InvokeMachine_invokes,i,1U);}
