#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}

static ESCX_InvokeToken InvokeTokenFor(const ESCX_StateMachine* m, ESCX_BYTE_T i) { ESCX_InvokeToken t = { i,m->invoke_instances[i].generation }; return t; }
namespace
{
	struct D
	{
		ESCX_Event finalized; ESCX_Event transitioned; ESCX_Event forwarded; int finalizes; int transitions; int forwards;
	}; int Start(ESCX_StateMachine*, void*, ESCX_InvokeToken)
	{
		return ESCX_STATUS_PROCESSED;
	}int Finalize(ESCX_StateMachine* m, void*, const ESCX_Event* e)
	{
		D* d = static_cast<D*>(m->instance); d->finalized = *e; ++d->finalizes; return ESCX_STATUS_PROCESSED;
	}int Forward(ESCX_StateMachine* m, void*, const ESCX_Event* e)
	{
		D* d = static_cast<D*>(m->instance); d->forwarded = *e; ++d->forwards; return ESCX_STATUS_PROCESSED;
	}void Observe(ESCX_StateMachine* m, void*)
	{
		D* d = static_cast<D*>(m->instance); d->transitioned = *ESCX_StateMachineGetCurrentEvent(m); ++d->transitions;
	}int Select(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}const ESCX_StateDescriptor st[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } }; const ESCX_BYTE_T ch[1] = { 0 }; const ESCX_StatePlan pl[1] = { { 0,0 } }; const ESCX_BYTE_T ps[1] = { 0 }; const ESCX_Transition tr[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,Observe,ESCX_TRANSITION_TARGETLESS,1,0,0 } }; const ESCX_SelectorFunction se[1] = { Select }; const ESCX_UINT16_T ba[2] = { 0,0 }; const ESCX_BYTE_T ow[2] = { 0,1 }; const ESCX_UINT16_T sl[2] = { ESCX_NO_SELECTOR,0 }; const ESCX_StateMachineDefinition df = { st,ch,tr,pl,ps,se,ba,ow,sl,2,1,1,1,1,1,2,1,2 };
}
TEST(InvokeCompletionMetadata, FinalizeAndDoneTransitionSeeCompleteDescriptor)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {};
	ESCX_InvokeInstance ii[1] = {}; D d = {}; int payload = 5; 
	ESCX_InvokeDescriptor in = { 1,1,ESCX_FALSE,0,Start,0,0,0,0,Finalize };
	ESCX_Event done = { 1,ESCX_EVENT_TYPE_INTERNAL,ESCX_EVENT_ORIGIN_SCXML,9,66,&payload }; 
	ESCX_StateMachineInit(&m, &df, &d, b, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &in, ii, 1)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineCompleteInvokeEvent(&m, InvokeTokenFor(&m, 0), &done)); 
	ASSERT_EQ(1, d.finalizes); 
	ASSERT_EQ(1, d.transitions); 
	EXPECT_EQ(66, d.finalized.send_id); 
	EXPECT_EQ(&payload, d.finalized.data); 
	EXPECT_EQ(ESCX_EVENT_ORIGIN_INVOKE, d.finalized.origin_type); 
	EXPECT_EQ(0, d.finalized.invoke_id); 
	EXPECT_EQ(66, d.transitioned.send_id);
}
TEST(InvokeCompletionMetadata, ExplicitInvokeTargetUsesDescriptorForwarding)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; ESCX_InvokeInstance ii[1] = {}; D d = {}; 
	int payload = 7; 
	ESCX_InvokeDescriptor in = { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start,0,0,0,0,0,0,Forward }; 
	ESCX_Event event = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,91,&payload };
	ESCX_SendTargetDescriptor target = ESCX_SEND_TARGET_SCXML_INVOKE(0); 
	ESCX_StateMachineInit(&m, &df, &d, b, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &in, ii, 1)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendToTarget(&m, &target, &event)); 
	ASSERT_EQ(1, d.forwards); 
	EXPECT_EQ(91, d.forwarded.send_id); 
	EXPECT_EQ(&payload, d.forwarded.data);
}
