#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Log
	{
		int values[8]; int count;
	}; void Add(ESCX_StateMachine* m, int v)
	{
		Log* d = static_cast<Log*>(m->instance); d->values[d->count++] = v;
	}
	int ParentEntry(ESCX_StateMachine* m, void*)
	{
		Add(m, 1); return ESCX_STATUS_PROCESSED;
	} int InitialAction(ESCX_StateMachine* m, void*)
	{
		Add(m, 2); return ESCX_STATUS_PROCESSED;
	} int ChildEntry(ESCX_StateMachine* m, void*)
	{
		Add(m, 3); return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[3] = {
	 { 0 },
	 { 0,0,0,0,0,0,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,ParentEntry,0,0,0,0,InitialAction,0 },
	 { 0,0,0,1,0,0,0,1,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,ChildEntry,0,0,0,0,0,0 }
	};
	const ESCX_BYTE_T children[1] = { 0 }; 
	const ESCX_StatePlan plans[1] = { { 0,0 } }; 
	const ESCX_BYTE_T plan_states[1] = { 0 }; 
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0,0 } }; const ESCX_SelectorFunction selectors[1] = { 0 }; const ESCX_UINT16_T bases[3] = { 0,0,0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR }; const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,3,1,1,1,1,1,1,1,2 };
	int FailInitial(ESCX_StateMachine*, void*)
	{
		return ESCX_STATUS_CALLBACK_FAILED;
	}
}
TEST(InitialTransitionContent, RunsBetweenParentAndChildEntry)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {};
	Log log = { { 0 },0 };
	ESCX_StateMachineInit(&m, &def, &log, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(3, log.count); EXPECT_EQ(1, log.values[0]); 
	EXPECT_EQ(2, log.values[1]); EXPECT_EQ(3, log.values[2]);
}
TEST(InitialTransitionContent, FailurePropagatesAsExecutionError)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Log log = { { 0 },0 }; 
	ESCX_StateDescriptor failing[3]; ESCX_StateMachineDefinition local = def; 
	failing[0] = states[0]; failing[1] = states[1]; failing[2] = states[2];
	failing[1].initial_action = FailInitial; local.states = failing;
	ESCX_StateMachineInit(&m, &local, &log, bits, 1, 0, 0, 0); 
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_ERROR_EXECUTION, ESCX_StateMachineGetLastError(&m));
}
