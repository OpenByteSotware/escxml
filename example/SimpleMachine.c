#include "SimpleMachine.h"
static void IdleEntry(ESCX_StateMachine *m,void *e){SimpleData*d;(void)e;d=(SimpleData*)m->instance;++d->entry_count;}
static ESCX_BOOL_T CanStart(ESCX_StateMachine*m,void*e){(void)m;(void)e;return ESCX_TRUE;}
static void Action(ESCX_StateMachine*m,void*e){SimpleData*d;(void)e;d=(SimpleData*)m->instance;++d->transition_count;}
ESCX_STATES_BEGIN(SimpleMachine)
 ESCX_STATE((ESCX_StateFunction)0,(ESCX_StateFunction)0,(ESCX_StateFunction)0)
 ESCX_STATE(IdleEntry,(ESCX_StateFunction)0,(ESCX_StateFunction)0)
 ESCX_STATE((ESCX_StateFunction)0,(ESCX_StateFunction)0,(ESCX_StateFunction)0)
ESCX_STATES_END()
ESCX_TRANSITIONS_BEGIN(SimpleMachine)
 ESCX_TRANSITION(SimpleMachine_Running,CanStart,Action)
 ESCX_TRANSITION(SimpleMachine_Idle,(ESCX_GuardFunction)0,Action)
ESCX_TRANSITIONS_END()
ESCX_RANGES_BEGIN(SimpleMachine)
 ESCX_RANGE(0U,1U)
 ESCX_RANGE(1U,1U)
ESCX_RANGES_END()
ESCX_STATE_BASE_BEGIN(SimpleMachine)
 ESCX_INDEX_U16(0U) ESCX_INDEX_U16(0U) ESCX_INDEX_U16(1U)
ESCX_STATE_BASE_END()
ESCX_SLOT_OWNER_BEGIN(SimpleMachine)
 ESCX_INDEX_U8(ESCX_NO_OWNER) ESCX_INDEX_U8(SimpleMachine_Idle) ESCX_INDEX_U8(ESCX_NO_OWNER) ESCX_INDEX_U8(SimpleMachine_Running)
ESCX_SLOT_OWNER_END()
ESCX_SLOT_RANGE_BEGIN(SimpleMachine)
 ESCX_INDEX_U16(ESCX_NO_RANGE) ESCX_INDEX_U16(0U) ESCX_INDEX_U16(ESCX_NO_RANGE) ESCX_INDEX_U16(1U)
ESCX_SLOT_RANGE_END()
ESCX_DEFINE_MACHINE(SimpleMachine,SimpleMachine_Idle);
ESCX_IMPLEMENT_MACHINE(SimpleMachine)
