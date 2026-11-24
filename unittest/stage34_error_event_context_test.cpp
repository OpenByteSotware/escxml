#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct D
	{
		ESCX_Event seen; ESCX_ErrorEventData data; int actions;
	}; void Observe(ESCX_StateMachine* m, void*)
	{
		D* d = static_cast<D*>(m->instance); d->seen = *ESCX_StateMachineGetCurrentEvent(m); d->data = *static_cast<ESCX_ErrorEventData*>(d->seen.data); ++d->actions;
	}ESCX_UINT16_T Select(ESCX_StateMachine*, void*)
	{
		return 0;
	}const ESCX_StateDescriptor st[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } }; const ESCX_BYTE_T ch[1] = { 0 }; const ESCX_StatePlan pl[1] = { { 0,0 } }; const ESCX_BYTE_T ps[1] = { 0 }; const ESCX_Transition tr[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,Observe,ESCX_TRANSITION_TARGETLESS,1,0,0 } }; const ESCX_SelectorFunction se[1] = { Select }; const ESCX_UINT16_T ba[2] = { 0,0 }; const ESCX_BYTE_T ow[4] = { 0,0,0,1 }; const ESCX_UINT16_T sl[4] = { ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,0 }; const ESCX_StateMachineDefinition df = { st,ch,tr,pl,ps,se,ba,ow,sl,2,1,1,1,1,1,4,1,4 };
}
TEST(ErrorEventContext, DeliversPlatformEventWithStructuredData)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; D d = {}; 
	int payload = 7;
	ESCX_Event trigger = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,
		ESCX_NO_INVOKE,44,&payload }; ESCX_StateMachineInit(&m, &df, &d, b, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureErrorEvents(&m, 2, 3)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendEvent(&m, &trigger)); 
	EXPECT_EQ(ESCX_STATUS_INVALID_TARGET, ESCX_StateMachineReportError(&m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET)); 
	ASSERT_EQ(1, d.actions); 
	EXPECT_EQ(ESCX_EVENT_TYPE_PLATFORM, d.seen.type); 
	EXPECT_EQ(ESCX_EVENT_ORIGIN_PLATFORM, d.seen.origin_type); 
	EXPECT_EQ(ESCX_ERROR_COMMUNICATION, d.data.code); 
	EXPECT_EQ(44, d.data.triggering_event.send_id);
}
TEST(ErrorEventContext, ClearResetsStructuredData)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; D d = {}; 
	ESCX_StateMachineInit(&m, &df, &d, b, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineReportError(&m, ESCX_ERROR_EXECUTION, ESCX_STATUS_CALLBACK_FAILED)); ESCX_StateMachineClearError(&m); const ESCX_ErrorEventData* e = ESCX_StateMachineGetLastErrorData(&m); ASSERT_NE(nullptr, e); EXPECT_EQ(ESCX_ERROR_NONE, e->code); EXPECT_EQ(ESCX_NO_EVENT, e->triggering_event.name_id);
}
