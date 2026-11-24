#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int a; int b;
	}; void A(ESCX_StateMachine* m, void*)
	{
		static_cast<Data*>(m->instance)->a++;
	} void B(ESCX_StateMachine* m, void*)
	{
		static_cast<Data*>(m->instance)->b++;
	}
	ESCX_UINT16_T SA(ESCX_StateMachine*, void*)
	{
		return 0;
	} ESCX_UINT16_T SB(ESCX_StateMachine*, void*)
	{
		return 1;
	}
	const ESCX_StateDescriptor states[4] = { { 0 },{ 0,0,0,0,0,2,0,0,0,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,0,1,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,0,1,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[2] = { 2,3 }; 
	const ESCX_StatePlan plans[1] = { { 0,0 } };
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[2] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,A,ESCX_TRANSITION_TARGETLESS,2,1,0 },{ ESCX_NO_PLAN,ESCX_NO_PLAN,B,ESCX_TRANSITION_TARGETLESS,3,1,1 } };
	const ESCX_SelectorFunction selectors[2] = { SA,SB }; const ESCX_UINT16_T bases[4] = { 0,0,0,1 };
	const ESCX_BYTE_T owners[3] = { 0,2,3 }; const ESCX_UINT16_T slots[3] = { ESCX_NO_SELECTOR,0,1 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,4,2,2,1,1,2,3,1,2 };
	/* Two rows, one byte each: transition 0 conflicts with 1, and 1 conflicts with 0. */
	const ESCX_BYTE_T conflict_bits[2] = { 0x03U,0x03U }; 
	const ESCX_ConflictMap map = { conflict_bits,2,1 };
}
TEST(ConflictBitmap, UsesGeneratedPairConflictLookup)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0,0 };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureConflictMap(&m, &map)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0));
	EXPECT_EQ(1, data.a); EXPECT_EQ(0, data.b);
}
TEST(ConflictBitmap, RejectsWrongTransitionCount)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0,0 }; 
	const ESCX_ConflictMap invalid = { conflict_bits,1,1 }; 
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0); 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureConflictMap(&m, &invalid));
}
