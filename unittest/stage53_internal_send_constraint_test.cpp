#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int actions; int timer_starts;
	};
	void Act(ESCX_StateMachine* m, void*)
	{
		++static_cast<Data*>(m->instance)->actions;
	}
	ESCX_UINT16_T Select(ESCX_StateMachine*, void*)
	{
		return 0;
	}
	ESCX_TimerId StartTimer(ESCX_StateMachine* m, unsigned long, void*)
	{
		++static_cast<Data*>(m->instance)->timer_starts;
		return 17;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,Act,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { Select }; const ESCX_UINT16_T bases[2] = { 0,0 }; const ESCX_BYTE_T owners[2] = { 0,1 }; const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,2,1,2 };
	void Init(ESCX_StateMachine* m, ESCX_BYTE_T* active, Data* data)
	{
		ESCX_StateMachineInit(m, &def, data, active, 1, 0, 0, 0);
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureTimers(m, StartTimer, 0, 0));
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(m));
	}
}
TEST(InternalSendConstraint, LegacyPositiveDelayRaisesExecutionErrorWithoutTimer)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0,0 };
	const ESCX_SendRequest request = { 1,ESCX_SEND_TARGET_INTERNAL,0,7,5,0 };
	Init(&m, active, &data);
	EXPECT_EQ(ESCX_STATUS_INVALID_TARGET, ESCX_StateMachineSendRequest(&m, &request));
	EXPECT_EQ(ESCX_ERROR_EXECUTION, ESCX_StateMachineGetLastError(&m));
	EXPECT_EQ(0, data.timer_starts); EXPECT_EQ(0, data.actions);
}
TEST(InternalSendConstraint, TargetedPositiveDelayRaisesExecutionErrorWithoutTimer)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0,0 };
	const ESCX_TargetedSendRequest request = { { 1,ESCX_EVENT_TYPE_INTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,8,0 },ESCX_SEND_TARGET_INTERNAL_QUEUE,5 };
	Init(&m, active, &data);
	EXPECT_EQ(ESCX_STATUS_INVALID_TARGET, ESCX_StateMachineSendTargetedRequest(&m, &request));
	EXPECT_EQ(ESCX_ERROR_EXECUTION, ESCX_StateMachineGetLastError(&m));
	EXPECT_EQ(0, data.timer_starts); EXPECT_EQ(0, data.actions);
}
TEST(InternalSendConstraint, ZeroDelayInternalSendRemainsQueuedForCurrentMacrostep)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0,0 };
	const ESCX_SendRequest request = { 1,ESCX_SEND_TARGET_INTERNAL,0,9,0,0 };
	Init(&m, active, &data);
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendRequest(&m, &request));
	EXPECT_EQ(0, data.timer_starts);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0));
	EXPECT_EQ(2, data.actions);
}
