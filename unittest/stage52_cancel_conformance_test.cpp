#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int after;
	};
	int After(ESCX_StateMachine* m, void*)
	{
		++static_cast<Data*>(m->instance)->after;
		return ESCX_STATUS_PROCESSED;
	}
	ESCX_TimerId StartTimer(ESCX_StateMachine*, unsigned long, void*)
	{
		return 17;
	}
	ESCX_BOOL_T CancelTimerFails(ESCX_StateMachine*, ESCX_TimerId, void*)
	{
		return ESCX_FALSE;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; 
	const ESCX_StatePlan plans[1] = { { 0,0 } };
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { 0 }; const ESCX_UINT16_T bases[2] = { 0 }; 
	const ESCX_BYTE_T owners[1] = { 0 }; 
	const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,1,1,2 };
	void Init(ESCX_StateMachine* m, ESCX_BYTE_T* active, Data* data)
	{
		ESCX_StateMachineInit(m, &def, data, active, 1, 0, 0, 0);
	}
}
TEST(CancelConformance, UnknownSendIdIsNoOpInExecutableContent)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0 };
	const ESCX_ExecutableStep steps[2] = {
		{ ESCX_EXECUTABLE_CANCEL,0,0,0,0,55,0 },
		{ ESCX_EXECUTABLE_GENERATED,After,0,0,0,0,0 }
	};
	const ESCX_ExecutableBlock block = { steps,2 };
	Init(&m, active, &data); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineExecuteContent(&m, &block));
	EXPECT_EQ(1, data.after); EXPECT_EQ(ESCX_ERROR_NONE, ESCX_StateMachineGetLastError(&m));
}
TEST(CancelConformance, PublicApiRetainsNotFoundDiagnostic)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0 }; Init(&m, active, &data);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	EXPECT_EQ(ESCX_STATUS_SEND_NOT_FOUND, ESCX_StateMachineCancelSend(&m, 55));
}
TEST(CancelConformance, TimerCancellationFailureStillRaisesExecutionErrorAndStopsBlock)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0 };
	const ESCX_SendRequest request = { 1,ESCX_SEND_TARGET_INTERNAL,0,55,10,0 };
	const ESCX_ExecutableStep steps[2] = {
		{ ESCX_EXECUTABLE_CANCEL,0,0,0,0,55,0 },
		{ ESCX_EXECUTABLE_GENERATED,After,0,0,0,0,0 }
	};
	const ESCX_ExecutableBlock block = { steps,2 };
	Init(&m, active, &data); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureTimers(&m, StartTimer, CancelTimerFails, 0)); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendRequest(&m, &request));
	EXPECT_EQ(ESCX_STATUS_TIMER_CANCEL_FAILED, ESCX_StateMachineExecuteContent(&m, &block));
	EXPECT_EQ(0, data.after); EXPECT_EQ(ESCX_ERROR_EXECUTION, ESCX_StateMachineGetLastError(&m));
}
