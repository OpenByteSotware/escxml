#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct CompletionData
	{
		int count;
	};
	ESCX_UINT16_T ObserveDone(ESCX_StateMachine* m, void*)
	{
		static_cast<CompletionData*>(m->instance)->count++; return ESCX_NO_TRANSITION;
	}
	const ESCX_StateDescriptor states[4] = {
	 { 0,0,0,0,0,0,0,0,0,0,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0,0 },
	 { 0,0,0,3,0,0,2,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,3,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0,0 },
	 { 0,0,0,1,0,0,0,2,2,ESCX_STATE_FINAL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0,0 },
	 { 0,0,0,0,0,0,1,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0,0 }
	};
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,2,2,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { ObserveDone }; const ESCX_UINT16_T bases[4] = { 0,0,0,0 };
	const ESCX_BYTE_T owners[4] = { 0,0,0,2 }; const ESCX_UINT16_T slots[4] = { ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,4,1,1,1,1,1,4,3,4 };
}
TEST(CompletionLatch, EmitsDoneStateOncePerActivation)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_BYTE_T completion[1] = {};
	CompletionData data = { 0 };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureCompletionStorage(&m, completion, 1));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	EXPECT_EQ(1, data.count);
	/* Re-stabilizing an already complete configuration must not emit another done.state. */
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0));
	EXPECT_EQ(1, data.count);
}
