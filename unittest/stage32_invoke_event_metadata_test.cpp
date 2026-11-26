#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}

static ESCX_InvokeToken InvokeTokenFor(const ESCX_StateMachine* m, ESCX_BYTE_T i) { ESCX_InvokeToken t = { i,m->invoke_instances[i].generation }; return t; }
namespace
{
	struct D
	{
		ESCX_Event f; ESCX_Event r; int nf; int nr;
	}; int Start(ESCX_StateMachine*, void*, ESCX_InvokeToken)
	{
		return ESCX_STATUS_PROCESSED;
	}int Forward(ESCX_StateMachine* m, void*, const ESCX_Event* e)
	{
		D* d = static_cast<D*>(m->instance); d->f = *e; ++d->nf; return ESCX_STATUS_PROCESSED;
	}void Observe(ESCX_StateMachine* m, void*)
	{
		D* d = static_cast<D*>(m->instance); d->r = *ESCX_StateMachineGetCurrentEvent(m); ++d->nr;
	}int Select(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}const ESCX_StateDescriptor st[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } }; const ESCX_BYTE_T ch[1] = { 0 }; const ESCX_StatePlan pl[1] = { { 0,0 } }; const ESCX_BYTE_T ps[1] = { 0 }; const ESCX_Transition tr[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,Observe,ESCX_TRANSITION_TARGETLESS,1,0,0 } }; const ESCX_SelectorFunction se[1] = { Select }; const ESCX_UINT16_T ba[2] = { 0,0 }; const ESCX_BYTE_T ow[2] = { 0,1 }; const ESCX_UINT16_T sl[2] = { ESCX_NO_SELECTOR,0 }; const ESCX_StateMachineDefinition df = { st,ch,tr,pl,ps,se,ba,ow,sl,2,1,1,1,1,1,2,1,2 };
}
TEST(InvokeEventMetadata, AutoforwardPreservesDescriptor)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; 
	ESCX_InvokeInstance ii[1] = {}; D d = {}; int p = 4; 
	ESCX_InvokeDescriptor in = { 1,ESCX_NO_EVENT,ESCX_TRUE,0,Start,0,0,0,0,0,0,Forward }; 
	ESCX_Event e = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,ESCX_NO_INVOKE,73,&p };
	ESCX_StateMachineInit(&m, &df, &d, b, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &in, ii, 1)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendEvent(&m, &e)); 
	EXPECT_EQ(1, d.nf); 
	EXPECT_EQ(73, d.f.send_id); 
	EXPECT_EQ(&p, d.f.data);
}
TEST(InvokeEventMetadata, ChildDeliveryPreservesDescriptor)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {};
	ESCX_InvokeInstance ii[1] = {};
	D d = {}; 
	int p = 8; 
	ESCX_InvokeDescriptor in = { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start }; 
	ESCX_Event e = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_SCXML,9,88,&p }; 
	ESCX_StateMachineInit(&m, &df, &d, b, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &in, ii, 1)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineReceiveInvokeEventDescriptor(&m, InvokeTokenFor(&m, 0), &e)); 
	EXPECT_EQ(1, d.nr); 
	EXPECT_EQ(ESCX_EVENT_ORIGIN_INVOKE, d.r.origin_type); 
	EXPECT_EQ(0, d.r.invoke_id); 
	EXPECT_EQ(88, d.r.send_id); 
	EXPECT_EQ(&p, d.r.data);
}
