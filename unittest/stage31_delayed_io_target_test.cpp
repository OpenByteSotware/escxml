#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct D
	{
		int calls; ESCX_Event event; ESCX_SendTargetDescriptor target;
	}; struct T
	{
		ESCX_TimerId next;
	}; ESCX_TimerId Start(ESCX_StateMachine*, unsigned long, void* c)
	{
		return static_cast<T*>(c)->next++;
	}ESCX_BOOL_T Cancel(ESCX_StateMachine*, ESCX_TimerId, void*)
	{
		return ESCX_TRUE;
	}int Transport(ESCX_StateMachine* m, const ESCX_SendTargetDescriptor* t, const ESCX_Event* e, void*)
	{
		D* d = static_cast<D*>(m->instance); 
		++d->calls; d->event = *e; d->target = *t;
		return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor st[2] = 
	{ { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } }; 
	const ESCX_BYTE_T ch[1] = { 0 }; 
	const ESCX_StatePlan pl[1] = { { 0,0 } };
	const ESCX_BYTE_T ps[1] = { 0 }; 
	const ESCX_Transition tr[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,1,0,0 } }; 
	const ESCX_SelectorFunction se[1] = { 0 }; 
	const ESCX_UINT16_T ba[2] = { 0 }; 
	const ESCX_BYTE_T ow[1] = { 0 }; 
	const ESCX_UINT16_T sl[1] = { ESCX_NO_SELECTOR }; 
	const ESCX_StateMachineDefinition df = { st,ch,tr,pl,ps,se,ba,ow,sl,2,1,1,1,1,1,1,1,2 };
}
TEST(DelayedIOTargets, PreservesTargetAndEventMetadataUntilExpiry)
{
	ESCX_StateMachine m = {};
	ESCX_BYTE_T b[1] = {};
	D d = {};
	T timer = { 50 }; int payload = 7; 
	ESCX_TargetedSendRequest q = { { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_SCXML,3,91,&payload },
		ESCX_SEND_TARGET_SCXML_SESSION(77),10 };
	ESCX_StateMachineInit(&m, &df, &d, b, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureIOProcessor(&m, Transport, 0)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureTimers(&m, Start, Cancel, &timer)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendTargetedRequest(&m, &q)); 
	EXPECT_EQ(0, d.calls); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineTimerExpired(&m, 50));
	ASSERT_EQ(1, d.calls); EXPECT_EQ(77, d.target.target_id); EXPECT_EQ(1, d.event.name_id); 
	EXPECT_EQ(ESCX_EVENT_ORIGIN_TIMER, d.event.origin_type); EXPECT_EQ(3, d.event.invoke_id); 
	EXPECT_EQ(91, d.event.send_id); EXPECT_EQ(&payload, d.event.data);
}
TEST(DelayedIOTargets, CancelBySendIdPreventsDelivery)
{
	ESCX_StateMachine m = {};
	ESCX_BYTE_T b[1] = {}; D d = {}; 
	T timer = { 80 }; 
	ESCX_TargetedSendRequest q = {
		{ 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,12,0 }
	,ESCX_SEND_TARGET_CUSTOM(9,4),5 };
	ESCX_StateMachineInit(&m, &df, &d, b, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureIOProcessor(&m, Transport, 0)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureTimers(&m, Start, Cancel, &timer)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendTargetedRequest(&m, &q)); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineCancelSend(&m, 12)); 
	EXPECT_EQ(ESCX_STATUS_IGNORED, ESCX_StateMachineTimerExpired(&m, 80)); 
	EXPECT_EQ(0, d.calls);
}
