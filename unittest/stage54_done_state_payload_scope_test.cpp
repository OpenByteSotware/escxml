#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int first; int second; void* parallel_payload; int observed;
	};
	int FirstDone(ESCX_StateMachine* m, void** out)
	{
		*out = &static_cast<Data*>(m->instance)->first; return ESCX_STATUS_PROCESSED;
	}
	int SecondDone(ESCX_StateMachine* m, void** out)
	{
		*out = &static_cast<Data*>(m->instance)->second; return ESCX_STATUS_PROCESSED;
	}
	void ObserveParallelDone(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); d->parallel_payload = ESCX_StateMachineGetCurrentEvent(m)->data; ++d->observed;
	}
	ESCX_UINT16_T SelectParallelDone(ESCX_StateMachine*, void*)
	{
		return 0;
	}
	const ESCX_StateDescriptor states[7] = {
	 { 0 },
	 { 0,0,0,0,0,1,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,1,1,2,ESCX_NO_STATE,1,1,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,1,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,2,0,0,4,2,2,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,3,0,0,0,3,3,ESCX_STATE_FINAL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0,FirstDone },
	 { 0,0,0,2,0,0,6,2,4,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,5,0,0,0,3,5,ESCX_STATE_FINAL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0,SecondDone }
	};
	const ESCX_BYTE_T children[3] = { 2,3,5 };
	const ESCX_StatePlan plans[1] = { { 0,0 } }; 
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,ObserveParallelDone,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { SelectParallelDone };
	const ESCX_UINT16_T bases[7] = { 0,0,0,0,0,0,0 };
	const ESCX_BYTE_T owners[2] = { 0,1 }; 
	const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,7,3,1,1,1,1,2,1,2 };
}
TEST(DoneStatePayloadScope, ParallelCompletionDoesNotReuseLastRegionFinalPayload)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; 
	ESCX_BYTE_T completion[1] = {}; 
	Data data = { 1,2,reinterpret_cast<void*>(1),0 };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureCompletionStorage(&m, completion, 1));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	EXPECT_EQ(1, data.observed); EXPECT_EQ(nullptr, data.parallel_payload);
}
