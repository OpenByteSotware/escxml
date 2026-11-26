#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int log[8]; int count;
	};
	int Assign(ESCX_StateMachine* m, void*)
	{
		static_cast<Data*>(m->instance)->log[static_cast<Data*>(m->instance)->count++] = 1; return ESCX_STATUS_PROCESSED;
	}
	int AfterRaise(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); d->log[d->count++] = 2; return ESCX_STATUS_PROCESSED;
	}
	int Fail(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); d->log[d->count++] = 3; return ESCX_STATUS_CALLBACK_FAILED;
	}
	int MustNotRun(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); d->log[d->count++] = 4; return ESCX_STATUS_PROCESSED;
	}
	void RaisedAction(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); d->log[d->count++] = 5;
	}
	int SelectRaised(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; 
	const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { {
			ESCX_NO_PLAN,ESCX_NO_PLAN,RaisedAction,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { SelectRaised }; 
	const ESCX_UINT16_T bases[2] = { 0,0 }; 
	const ESCX_BYTE_T owners[3] = { 0,0,1 }; 
	const ESCX_UINT16_T slots[3] = { ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,3,1,3 };
}
TEST(ExecutableContent, ExecutesInOrderAndDefersRaisedEvent)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = { { 0 },0 }; 
	ESCX_Event raised = { 2,ESCX_EVENT_TYPE_INTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,ESCX_NO_SEND_ID,0 }; ESCX_ExecutableStep steps[3] = { { ESCX_EXECUTABLE_GENERATED,Assign,0,0,0,0 },{ ESCX_EXECUTABLE_RAISE,0,0,&raised,0,0 },
		{ ESCX_EXECUTABLE_GENERATED,AfterRaise,0,0,0,0 } };
	ESCX_ExecutableBlock block = { steps,3 }; 
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineExecuteContent(&m, &block));
	ASSERT_EQ(2, data.count); EXPECT_EQ(1, data.log[0]); EXPECT_EQ(2, data.log[1]); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0)); 
	ASSERT_EQ(3, data.count); EXPECT_EQ(5, data.log[2]);
}
TEST(ExecutableContent, StopsAtFirstFailureAndReportsExecutionError)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = { { 0 },0 };
	ESCX_ExecutableStep steps[3] = { { ESCX_EXECUTABLE_GENERATED,Assign,0,0,0,0 },{ 
		ESCX_EXECUTABLE_GENERATED,Fail,0,0,0,0 },{ ESCX_EXECUTABLE_GENERATED,MustNotRun,0,0,0,0 } };
	ESCX_ExecutableBlock block = { steps,3 }; 
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineExecuteContent(&m, &block));
	ASSERT_EQ(2, data.count); EXPECT_EQ(1, data.log[0]); EXPECT_EQ(3, data.log[1]);
	EXPECT_EQ(ESCX_ERROR_EXECUTION, ESCX_StateMachineGetLastError(&m));
}
