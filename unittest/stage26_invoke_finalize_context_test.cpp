#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}

static ESCX_InvokeToken InvokeTokenFor(const ESCX_StateMachine* m, ESCX_BYTE_T i) { ESCX_InvokeToken t = { i,m->invoke_instances[i].generation }; return t; }
namespace
{
	struct Data
	{
		ESCX_Event seen; int finalize_count; int transition_count;
	};
	int Start(ESCX_StateMachine*, void*, ESCX_InvokeToken)
	{
		return ESCX_STATUS_PROCESSED;
	}
	int Finalize(ESCX_StateMachine* m, void*, const ESCX_Event* event)
	{
		Data* d = static_cast<Data*>(m->instance); d->seen = *event; ++d->finalize_count; const ESCX_Event* current = ESCX_StateMachineGetCurrentEvent(m); return current->invoke_id == event->invoke_id ? ESCX_STATUS_PROCESSED : ESCX_STATUS_CALLBACK_FAILED;
	}
	void Observe(ESCX_StateMachine* m, void*)
	{
		++static_cast<Data*>(m->instance)->transition_count;
	}
	int Select(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; 
	const ESCX_StatePlan plans[1] = { { 0,0 } };
	const ESCX_BYTE_T plan_states[1] = { 0 }; 
	const ESCX_Transition transitions[1] = { {
			ESCX_NO_PLAN,ESCX_NO_PLAN,Observe,ESCX_TRANSITION_TARGETLESS,1,0,0 } }; 
	const ESCX_SelectorFunction selectors[1] = { Select }; 
	const ESCX_UINT16_T bases[2] = { 0,0 }; const ESCX_BYTE_T owners[2] = { 0,1 }; 
	const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 }; 
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,2,1,2 };
}
TEST(InvokeFinalize, ChildEventIsCurrentDuringFinalizeAndTransition)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; ESCX_InvokeInstance instances[1] = {}; 
	int payload = 42; Data data = { { ESCX_NO_EVENT,0,0,ESCX_NO_INVOKE,0,0 },0,0 };
	ESCX_InvokeDescriptor invoke = { 1,1,ESCX_FALSE,0,Start,0,0,0,0,Finalize };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &invoke, instances, 1));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); ASSERT_TRUE(instances[0].active);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineReceiveInvokeEvent(&m, InvokeTokenFor(&m, 0), 1, &payload)); 
	EXPECT_EQ(1, data.finalize_count);
	EXPECT_EQ(1, data.transition_count);
	EXPECT_EQ(ESCX_EVENT_ORIGIN_INVOKE, data.seen.origin_type); 
	EXPECT_EQ(0, data.seen.invoke_id);
	EXPECT_EQ(&payload, data.seen.data);
}
TEST(InvokeFinalize, LateEventAfterCancellationIsRejected)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; ESCX_InvokeInstance instances[1] = {};
	Data data = { {},0,0 }; ESCX_InvokeDescriptor invoke = { 1,1,ESCX_FALSE,0,Start,0,0,0,0,Finalize };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &invoke, instances, 1)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	ESCX_StateMachineStop(&m);
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineReceiveInvokeEvent(&m, InvokeTokenFor(&m, 0), 1, 0));
	EXPECT_EQ(0, data.finalize_count);
}
