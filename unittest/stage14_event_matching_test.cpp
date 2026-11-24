#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct MatchData
	{
		int exact; int parent; int wildcard;
	};
	void Exact(ESCX_StateMachine* m, void*)
	{
		static_cast<MatchData*>(m->instance)->exact++;
	}
	void Parent(ESCX_StateMachine* m, void*)
	{
		static_cast<MatchData*>(m->instance)->parent++;
	}
	void Wildcard(ESCX_StateMachine* m, void*)
	{
		static_cast<MatchData*>(m->instance)->wildcard++;
	}
	ESCX_UINT16_T SelectExact(ESCX_StateMachine*, void*)
	{
		return 0;
	} ESCX_UINT16_T SelectParent(ESCX_StateMachine*, void*)
	{
		return 1;
	} ESCX_UINT16_T SelectWildcard(ESCX_StateMachine*, void*)
	{
		return 2;
	}
	const ESCX_StateDescriptor states[2] = { { 0,0,0,0,0,0,0,0,0,0,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[3] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,Exact,ESCX_TRANSITION_TARGETLESS,1,0,0 },{ ESCX_NO_PLAN,ESCX_NO_PLAN,Parent,ESCX_TRANSITION_TARGETLESS,1,0,1 },{ ESCX_NO_PLAN,ESCX_NO_PLAN,Wildcard,ESCX_TRANSITION_TARGETLESS,1,0,2 } };
	const ESCX_SelectorFunction selectors[3] = { SelectExact,SelectParent,SelectWildcard };
	/* event IDs: 1=error, 2=error.execution, 3=other, 4=*.
	   state 1 base 0 gives direct slots by event ID. */
	const ESCX_UINT16_T bases[2] = { 0,0 }; const ESCX_BYTE_T owners[5] = { 0,1,1,0,1 }; const ESCX_UINT16_T slots[5] = { ESCX_NO_SELECTOR,1,0,ESCX_NO_SELECTOR,2 };
	const ESCX_BYTE_T parents[5] = { ESCX_NO_EVENT,ESCX_NO_EVENT,1,ESCX_NO_EVENT,ESCX_NO_EVENT };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,3,1,1,3,5,1,5 };
}
TEST(EventMatching, ExactThenParentThenWildcard)
{
	ESCX_StateMachine m; ESCX_BYTE_T bits[1]; 
	MatchData data = { 0,0,0 }; ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureEventMatching(&m, parents, 5, 4));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 2, 0));
	EXPECT_EQ(1, data.exact); EXPECT_EQ(0, data.parent); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0)); 
	EXPECT_EQ(1, data.parent); EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 3, 0)); 
	EXPECT_EQ(1, data.wildcard);
}
