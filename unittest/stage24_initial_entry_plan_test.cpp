#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Log
	{
		int v[8]; int n;
	}; void Add(ESCX_StateMachine* m, int x)
	{
		Log* d = static_cast<Log*>(m->instance); d->v[d->n++] = x;
	}
	int InitialAction(ESCX_StateMachine* m, void*)
	{
		Add(m, 1); return ESCX_STATUS_PROCESSED;
	} int EnterParallel(ESCX_StateMachine* m, void*)
	{
		Add(m, 2); return ESCX_STATUS_PROCESSED;
	} int EnterA(ESCX_StateMachine* m, void*)
	{
		Add(m, 3); return ESCX_STATUS_PROCESSED;
	} int EnterB(ESCX_StateMachine* m, void*)
	{
		Add(m, 4); return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[5] = {
	 { 0 },
	 { 0,0,0,0,0,1,ESCX_NO_STATE,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0,0,InitialAction,0,0 },
	 { 0,0,0,1,0,2,ESCX_NO_STATE,1,1,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,EnterParallel,0,0,0,0,0,0,ESCX_NO_PLAN },
	 { 0,0,0,2,0,0,0,2,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,EnterA,0,0,0,0,0,0,ESCX_NO_PLAN },
	 { 0,0,0,2,0,0,0,2,3,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,EnterB,0,0,0,0,0,0,ESCX_NO_PLAN }
	};
	const ESCX_BYTE_T children[3] = { 2,3,4 }; 
	const ESCX_StatePlan plans[1] = { { 0,3 } };
	const ESCX_BYTE_T plan_states[3] = { 2,3,4 }; 
	const ESCX_Transition transitions[1] = { { 
			ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { 0 }; const ESCX_UINT16_T bases[5] = { 0 }; 
	const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR }; 
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,5,3,1,1,3,1,1,1,2 };
}
TEST(InitialEntryPlan, EntersGeneratedParallelConfigurationAfterInitialAction)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Log log = { { 0 },0 }; 
	ESCX_StateMachineInit(&m, &def, &log, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(4, log.n); EXPECT_EQ(1, log.v[0]); EXPECT_EQ(2, log.v[1]); EXPECT_EQ(3, log.v[2]);
	EXPECT_EQ(4, log.v[3]); EXPECT_TRUE(ESCX_StateMachineIsInState(&m, 2));
	EXPECT_TRUE(ESCX_StateMachineIsInState(&m, 3));
	EXPECT_TRUE(ESCX_StateMachineIsInState(&m, 4));
}
