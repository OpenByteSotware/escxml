#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int first; int second; int entries;
	};
	int InitFirst(ESCX_StateMachine* m, void*)
	{
		static_cast<Data*>(m->instance)->first++; return ESCX_STATUS_PROCESSED;
	}
	int InitSecond(ESCX_StateMachine* m, void*)
	{
		static_cast<Data*>(m->instance)->second++; return ESCX_STATUS_PROCESSED;
	}
	int EnterFirst(ESCX_StateMachine* m, void*)
	{
		static_cast<Data*>(m->instance)->entries++; return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[3] = { { 0,0,0,0,0,0,0,0,0,0,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,EnterFirst,0,0,InitFirst },{ 0,0,0,0,0,0,0,0,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,InitSecond } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0,0 } }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 }; const ESCX_SelectorFunction selectors[1] = { 0 }; const ESCX_UINT16_T bases[3] = { 0,0,0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR }; const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,3,1,1,1,1,1,1,1,2 };
}
TEST(DataBinding, EarlyInitializesAllBeforeEntry)
{
	ESCX_StateMachine m; ESCX_BYTE_T active[1], initialized[1];
	Data data = { 0,0,0 }; ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureDataBinding(&m, ESCX_DATA_BINDING_EARLY, initialized, 1));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	EXPECT_EQ(1, data.first);
	EXPECT_EQ(1, data.second); EXPECT_EQ(1, data.entries);
	EXPECT_TRUE(ESCX_StateMachineIsDataInitialized(&m, 2));
}
TEST(DataBinding, LateInitializesOnlyEnteredState)
{
	ESCX_StateMachine m; ESCX_BYTE_T active[1], initialized[1]; Data data = { 0,0,0 };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureDataBinding(&m, ESCX_DATA_BINDING_LATE, initialized, 1));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); EXPECT_EQ(1, data.first);
	EXPECT_EQ(0, data.second); EXPECT_TRUE(ESCX_StateMachineIsDataInitialized(&m, 1));
	EXPECT_FALSE(ESCX_StateMachineIsDataInitialized(&m, 2));
}
