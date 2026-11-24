#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct StopData
	{
		int log[8]; int count;
	};
	void Log(ESCX_StateMachine* m, int value)
	{
		StopData* d = static_cast<StopData*>(m->instance); d->log[d->count++] = value;
	}
	void ExitRoot(ESCX_StateMachine* m, void*)
	{
		Log(m, 1);
	} void ExitParent(ESCX_StateMachine* m, void*)
	{
		Log(m, 2);
	} void ExitLeaf(ESCX_StateMachine* m, void*)
	{
		Log(m, 3);
	}
	const ESCX_StateDescriptor states[4] = { { 0,0,0,0,0,0,0,0,0,0,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,ExitRoot,0,0,0,0,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,ExitParent,0,1,0,0,3,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,ExitLeaf,0,2,0,0,0,2,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 }; const ESCX_SelectorFunction selectors[1] = { 0 }; const ESCX_UINT16_T bases[4] = { 0,0,0,0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,4,1,1,1,1,1,1,1,2 };
}
TEST(Termination, ExitsLeafThenAncestorsAndCanRestart)
{
	ESCX_StateMachine m; ESCX_BYTE_T bits[1]; StopData data = { { 0 },0 };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ESCX_StateMachineStop(&m); ASSERT_EQ(3, data.count); 
	EXPECT_EQ(3, data.log[0]); EXPECT_EQ(2, data.log[1]); 
	EXPECT_EQ(1, data.log[2]); EXPECT_FALSE(ESCX_StateMachineIsInState(&m, 1));
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_TRUE(ESCX_StateMachineIsInState(&m, 3));
}
TEST(Termination, EventMatchingSurvivesEventlessProcessingAndRestart)
{
	ESCX_StateMachine m; ESCX_BYTE_T bits[1]; StopData data = { { 0 },0 }; 
	const ESCX_BYTE_T parents[2] = { ESCX_NO_EVENT,ESCX_NO_EVENT }; 
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureEventMatching(&m, parents, 2, ESCX_NO_EVENT)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ESCX_StateMachineStop(&m); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(parents, m.event_parent); 
	EXPECT_EQ(2, m.event_parent_count);
}
