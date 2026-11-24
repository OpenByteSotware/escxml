#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int enabled; int actions;
	};
	void Act(ESCX_StateMachine* m, void*)
	{
		++static_cast<Data*>(m->instance)->actions;
	}
	ESCX_UINT16_T Select(ESCX_StateMachine* m, void*)
	{
		return static_cast<Data*>(m->instance)->enabled ? 0 : ESCX_NO_TRANSITION;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; 
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,Act,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { Select }; const ESCX_UINT16_T bases[2] = { 0 }; 
	const ESCX_BYTE_T owners[2] = { 0,1 }; 
	const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,2,1,2 };
}
TEST(EventAcceptance, UnmatchedValidEventIsAcceptedWithoutTransition)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = { 0,0 }; 
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0)); 
	const ESCX_MacrostepResult* r = ESCX_StateMachineGetLastMacrostepResult(&m); 
	ASSERT_NE(nullptr, r); EXPECT_TRUE(r->event_accepted); EXPECT_FALSE(r->transition_taken); 
	EXPECT_EQ(0, r->microstep_count); EXPECT_EQ(0, data.actions);
}
TEST(EventAcceptance, EnabledTransitionIsReportedSeparately)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = { 1,0 }; 
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0)); 
	const ESCX_MacrostepResult* r = ESCX_StateMachineGetLastMacrostepResult(&m); 
	ASSERT_NE(nullptr, r); EXPECT_TRUE(r->event_accepted); EXPECT_TRUE(r->transition_taken); 
	EXPECT_EQ(1, r->microstep_count); EXPECT_EQ(1, data.actions);
}
TEST(EventAcceptance, InvalidEventIsRejectedAndNotAccepted)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = { 0,0 }; 
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_STATUS_INVALID_EVENT, ESCX_StateMachineSend(&m, 2, 0)); 
	const ESCX_MacrostepResult* r = ESCX_StateMachineGetLastMacrostepResult(&m); 
	ASSERT_NE(nullptr, r); EXPECT_FALSE(r->event_accepted);
}
