#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		ESCX_Event seen; int actions;
	};
	void Observe(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance);
		d->seen = *ESCX_StateMachineGetCurrentEvent(m);
		++d->actions;
	}
	ESCX_UINT16_T Select(ESCX_StateMachine*, void*)
	{
		return 0;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 };
	const ESCX_StatePlan plans[1] = { { 0,0 } };
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,Observe,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { Select };
	const ESCX_UINT16_T bases[2] = { 0,0 };
	const ESCX_BYTE_T owners[2] = { 0,1 };
	const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,2,1,2 };
}
TEST(InternalIOEventNormalization, TargetedSendOverridesCallerTypeOriginAndInvokeMetadata)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = {}; int payload = 42;
	ESCX_Event supplied = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_INVOKE,7,91,&payload };
	const ESCX_SendTargetDescriptor target = ESCX_SEND_TARGET_INTERNAL_QUEUE;
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendToTarget(&m, &target, &supplied));
	EXPECT_EQ(0, data.actions);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0));
	ASSERT_EQ(2, data.actions);
	EXPECT_EQ(ESCX_EVENT_TYPE_INTERNAL, data.seen.type);
	EXPECT_EQ(ESCX_EVENT_ORIGIN_SCXML, data.seen.origin_type);
	EXPECT_EQ(ESCX_NO_INVOKE, data.seen.invoke_id);
	EXPECT_EQ(91, data.seen.send_id);
	EXPECT_EQ(&payload, data.seen.data);
	EXPECT_EQ(ESCX_EVENT_TYPE_EXTERNAL, supplied.type);
	EXPECT_EQ(ESCX_EVENT_ORIGIN_INVOKE, supplied.origin_type);
	EXPECT_EQ(7, supplied.invoke_id);
}
TEST(InternalIOEventNormalization, TargetedRequestUsesSameNormalizedInternalDescriptor)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = {}; int payload = 9;
	const ESCX_TargetedSendRequest request = { { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,4,73,&payload },ESCX_SEND_TARGET_INTERNAL_QUEUE,0 };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendTargetedRequest(&m, &request));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0));
	ASSERT_EQ(2, data.actions);
	EXPECT_EQ(ESCX_EVENT_TYPE_INTERNAL, data.seen.type);
	EXPECT_EQ(ESCX_EVENT_ORIGIN_SCXML, data.seen.origin_type);
	EXPECT_EQ(ESCX_NO_INVOKE, data.seen.invoke_id);
	EXPECT_EQ(73, data.seen.send_id);
	EXPECT_EQ(&payload, data.seen.data);
}
