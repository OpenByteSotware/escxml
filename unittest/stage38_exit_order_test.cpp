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
	void Exit1(ESCX_StateMachine* m, void*)
	{
		static_cast<Data*>(m->instance)->log[static_cast<Data*>(m->instance)->count++] = 1;
	}
	void Exit2(ESCX_StateMachine* m, void*)
	{
		static_cast<Data*>(m->instance)->log[static_cast<Data*>(m->instance)->count++] = 2;
	}
	void Exit3(ESCX_StateMachine* m, void*)
	{
		static_cast<Data*>(m->instance)->log[static_cast<Data*>(m->instance)->count++] = 3;
	}
	void Exit4(ESCX_StateMachine* m, void*)
	{
		static_cast<Data*>(m->instance)->log[static_cast<Data*>(m->instance)->count++] = 4;
	}
	const ESCX_StateDescriptor states[5] = { { 0 },{ 0,Exit1,0,0,0,2,ESCX_NO_STATE,0,0,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,Exit2,0,1,0,0,3,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,Exit3,0,2,0,0,0,2,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,Exit4,0,1,0,0,0,1,3,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[2] = { 2,4 }; const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 }; const ESCX_SelectorFunction selectors[1] = { 0 }; const ESCX_UINT16_T bases[5] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR }; const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,5,2,1,1,1,1,1,1,2 };
	const ESCX_BYTE_T ordered_states[4] = { 3,4,2,1 }; const ESCX_ExitOrder order = { ordered_states,4 };
}
TEST(ExitOrder, GeneratedOrderStopsDeepestFirstAndReverseDocumentOrder)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = { { 0 },0 };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureExitOrder(&m, &order)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(2, m.active_atomic_count); EXPECT_EQ(3, m.active_atomic_states[0]); 
	EXPECT_EQ(4, m.active_atomic_states[1]); ESCX_StateMachineStop(&m); 
	ASSERT_EQ(4, data.count); EXPECT_EQ(3, data.log[0]); 
	EXPECT_EQ(4, data.log[1]); EXPECT_EQ(2, data.log[2]); EXPECT_EQ(1, data.log[3]);
}
TEST(ExitOrder, RejectsDuplicateAndIncorrectlyOrderedTables)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = { { 0 },0 }; 
	const ESCX_BYTE_T duplicate_states[4] = { 3,3,2,1 };
	const ESCX_ExitOrder duplicate = { duplicate_states,4 };
	const ESCX_BYTE_T wrong_states[4] = { 4,3,2,1 }; const ESCX_ExitOrder wrong = { wrong_states,4 };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureExitOrder(&m, &duplicate)); 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureExitOrder(&m, &wrong));
}
