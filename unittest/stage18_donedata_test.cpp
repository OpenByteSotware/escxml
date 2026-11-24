#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct DoneData
	{
		int value; void* seen; ESCX_BYTE_T invoke;
	};
	int FinalPayload(ESCX_StateMachine* m, void** out)
	{
		*out = &static_cast<DoneData*>(m->instance)->value; return ESCX_STATUS_PROCESSED;
	}
	ESCX_UINT16_T SelectDone(ESCX_StateMachine* m, void*)
	{
		const ESCX_Event* e = ESCX_StateMachineGetCurrentEvent(m); DoneData* d = static_cast<DoneData*>(m->instance); d->seen = e->data; d->invoke = e->invoke_id; return ESCX_NO_TRANSITION;
	}
	const ESCX_StateDescriptor states[4] = { { 0,0,0,0,0,0,0,0,0,0,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0,0 },{ 0,0,0,0,0,0,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,3,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0,0 },{ 0,0,0,1,0,0,0,1,1,ESCX_STATE_FINAL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0,FinalPayload },{ 0,0,0,0,0,0,0,0,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,0,0,0,0 } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 }; const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,3,0,0,0 } }; const ESCX_SelectorFunction selectors[1] = { SelectDone }; const ESCX_UINT16_T bases[4] = { 0,0,0,0 }; const ESCX_BYTE_T owners[4] = { 0,0,0,2 }; const ESCX_UINT16_T slots[4] = { ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,0 }; const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,4,1,1,1,1,1,4,1,4 };
	int InvokeStart(ESCX_StateMachine*, void*)
	{
		return ESCX_STATUS_PROCESSED;
	} int InvokePayload(ESCX_StateMachine* m, void** out)
	{
		*out = &static_cast<DoneData*>(m->instance)->value; return ESCX_STATUS_PROCESSED;
	}
}
TEST(DoneData, DoneStateCarriesFinalPayload)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; 
	DoneData data = { 42,0,ESCX_NO_INVOKE }; ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); EXPECT_EQ(&data.value, data.seen);
}
TEST(DoneData, DoneInvokeCarriesPayloadAndInvokeId)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; DoneData data = { 77,0,ESCX_NO_INVOKE };
	ESCX_InvokeInstance instance[1];
	ESCX_InvokeDescriptor invoke = { 3,3,ESCX_FALSE,0,InvokeStart,0,0,0,InvokePayload };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &invoke, instance, 1)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); instance[0].active = ESCX_TRUE;
	data.seen = 0; EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineCompleteInvoke(&m, 0, 0));
	EXPECT_EQ(&data.value, data.seen); EXPECT_EQ(0, data.invoke);
}
