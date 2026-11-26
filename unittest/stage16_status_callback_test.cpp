#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	int FailEntry(ESCX_StateMachine*, void*)
	{
		return ESCX_STATUS_CALLBACK_FAILED;
	}
	int FailAction(ESCX_StateMachine*, void*)
	{
		return ESCX_STATUS_CALLBACK_FAILED;
	}
	int Select(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor entry_states[2] = { { 0,0,0,0,0,0,0,0,0,0,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,FailEntry,0,0 } };
	const ESCX_StateDescriptor action_states[2] = { { 0,0,0,0,0,0,0,0,0,0,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0 } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 }; const ESCX_Transition entry_transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0,0 } }; const ESCX_Transition action_transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,1,0,0,FailAction } }; const ESCX_SelectorFunction selectors[1] = { Select }; const ESCX_UINT16_T bases[2] = { 0,0 }; const ESCX_BYTE_T owners[2] = { 0,1 }; const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition entry_def = { entry_states,children,entry_transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,2,1,2 };
	const ESCX_StateMachineDefinition action_def = { action_states,children,action_transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,2,1,2 };
}
TEST(StatusCallbacks, EntryFailurePropagatesAndRecordsExecutionError)
{
	ESCX_StateMachine m;
	ESCX_BYTE_T bits[1]; 
	ESCX_StateMachineInit(&m, &entry_def, 0, bits, 1, 0, 0, 0); 
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_ERROR_EXECUTION, ESCX_StateMachineGetLastError(&m)); 
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineGetLastErrorStatus(&m));
}
TEST(StatusCallbacks, TransitionFailurePropagatesAndRecordsExecutionError)
{
	ESCX_StateMachine m; ESCX_BYTE_T bits[1]; 
	ESCX_StateMachineInit(&m, &action_def, 0, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineSend(&m, 1, 0)); 
	EXPECT_EQ(ESCX_ERROR_EXECUTION, ESCX_StateMachineGetLastError(&m));
}
