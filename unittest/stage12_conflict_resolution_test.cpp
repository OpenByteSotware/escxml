#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct ConflictData
	{
		int ancestor_actions; int left_actions; int right_actions;
	};
	void AncestorAction(ESCX_StateMachine* m, void*)
	{
		static_cast<ConflictData*>(m->instance)->ancestor_actions++;
	}
	void LeftAction(ESCX_StateMachine* m, void*)
	{
		static_cast<ConflictData*>(m->instance)->left_actions++;
	}
	void RightAction(ESCX_StateMachine* m, void*)
	{
		static_cast<ConflictData*>(m->instance)->right_actions++;
	}
	int SelectAncestor(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}
	int SelectLeft(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(1); return ESCX_STATUS_PROCESSED;
	}
	int SelectRight(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(2); return ESCX_STATUS_PROCESSED;
	}
	/* root parallel(1), regions(2,4), leaves(3,5). Both descendant transitions
	   exit their leaves. The ancestor transition exits root and therefore
	   conflicts with both. Descendants must both preempt the ancestor. */
	const ESCX_StateDescriptor states[6] = {
	 { 0,0,0,0,0,0,0,0,0,0,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,0,0,2,ESCX_NO_STATE,0,0,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,1,0,0,3,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,2,0,0,0,2,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,1,0,0,5,1,3,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,4,0,0,0,2,4,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[2] = { 2,4 };
	const ESCX_StatePlan plans[3] = { { 0,5 },{ 5,1 },{ 6,1 } };
	const ESCX_BYTE_T plan_states[7] = { 3,5,2,4,1,3,5 };
	const ESCX_Transition transitions[3] = {
	 { 0,ESCX_NO_PLAN,AncestorAction,ESCX_TRANSITION_EXTERNAL,1,0,0 },
	 { 1,ESCX_NO_PLAN,LeftAction,ESCX_TRANSITION_EXTERNAL,3,2,1 },
	 { 2,ESCX_NO_PLAN,RightAction,ESCX_TRANSITION_EXTERNAL,5,2,2 } };
	/* event 1: leaf3 selects descendant; leaf5 selects descendant. The ancestor
	   candidate is exposed by leaf3 for event 2 to validate ancestor rejection. */
	int SelectLeftOrAncestor(ESCX_StateMachine*, void* data, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(data ? 0 : 1); return ESCX_STATUS_PROCESSED;
	}
	const ESCX_SelectorFunction selectors[3] = { SelectLeftOrAncestor,SelectRight,SelectAncestor };
	const ESCX_UINT16_T bases[6] = { 0,0,0,0,0,1 };
	const ESCX_BYTE_T owners[3] = { 0,3,5 };
	const ESCX_UINT16_T slots[3] = { ESCX_NO_SELECTOR,0,1 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,6,2,3,3,7,3,3,1,3 };
}
TEST(ConflictResolution, OrthogonalDescendantsRemainSelected)
{
	ESCX_StateMachine m; ESCX_BYTE_T bits[1]; ConflictData data = { 0,0,0 };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0));
	EXPECT_EQ(0, data.ancestor_actions); EXPECT_EQ(1, data.left_actions); 
	EXPECT_EQ(1, data.right_actions);
}
