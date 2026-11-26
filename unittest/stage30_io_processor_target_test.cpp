#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct D
	{
		int actions; int transports; ESCX_UINT16_T target;
	}; void A(ESCX_StateMachine* m, void*)
	{
		++static_cast<D*>(m->instance)->actions;
	}int S(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}int X(ESCX_StateMachine* m, const ESCX_SendTargetDescriptor* t, const ESCX_Event*, void*)
	{
		D* d = static_cast<D*>(m->instance); ++d->transports; d->target = t->target_id;
		return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor st[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,
		ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T ch[1] = { 0 }; 
	const ESCX_StatePlan pl[1] = { { 0,0 } };
	const ESCX_BYTE_T ps[1] = { 0 }; 
	const ESCX_Transition tr[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,A,ESCX_TRANSITION_TARGETLESS,1,0,0 } }; 
	const ESCX_SelectorFunction se[1] = { S }; const ESCX_UINT16_T ba[2] = { 0,0 };
	const ESCX_BYTE_T ow[2] = { 0,1 }; const ESCX_UINT16_T sl[2] = { ESCX_NO_SELECTOR,0 }; 
	const ESCX_StateMachineDefinition df = { st,ch,tr,pl,ps,se,ba,ow,sl,2,1,1,1,1,1,2,1,2 };
}
TEST(IOProcessorTargets, BuiltInInternalAndCurrent)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; D d = { 0,0,0 }; 
	ESCX_SystemContext c = { 42,0,0,0 };
	ESCX_Event e = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,7,0 }; 
	ESCX_SendTargetDescriptor i = ESCX_SEND_TARGET_INTERNAL_QUEUE, q = ESCX_SEND_TARGET_SCXML_CURRENT;
	ESCX_StateMachineInit(&m, &df, &d, b, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureSystemContext(&m, &c)); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendToTarget(&m, &i, &e)); EXPECT_EQ(0, d.actions); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendToTarget(&m, &q, &e)); EXPECT_EQ(2, d.actions);
}
TEST(IOProcessorTargets, RemoteSessionUsesTransport)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; D d = { 0,0,0 }; 
	ESCX_SystemContext c = { 42,0,0,0 }; ESCX_Event e = { 1,2,2,ESCX_NO_INVOKE,0,0 };
	ESCX_SendTargetDescriptor t = ESCX_SEND_TARGET_SCXML_SESSION(77); ESCX_StateMachineInit(&m, &df, &d, b, 1, 0, 0, 0); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureSystemContext(&m, &c)); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureIOProcessor(&m, X, 0)); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendToTarget(&m, &t, &e)); EXPECT_EQ(1, d.transports); EXPECT_EQ(77, d.target);
}
TEST(IOProcessorTargets, InvalidCombinationReportsCommunication)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; D d = { 0,0,0 };
	ESCX_Event e = { 1,2,2,ESCX_NO_INVOKE,0,0 };
	ESCX_SendTargetDescriptor t = { ESCX_IO_PROCESSOR_INTERNAL,ESCX_IO_TARGET_PARENT_SESSION,0 };
	ESCX_StateMachineInit(&m, &df, &d, b, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	EXPECT_EQ(ESCX_STATUS_INVALID_TARGET, ESCX_StateMachineSendToTarget(&m, &t, &e));
	EXPECT_EQ(ESCX_ERROR_COMMUNICATION, ESCX_StateMachineGetLastError(&m));
}
