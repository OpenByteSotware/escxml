#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int finalizes; ESCX_Event finalized; int actions; ESCX_Event seen;
	};
	int Start(ESCX_StateMachine*, void*, ESCX_InvokeToken)
	{
		return ESCX_STATUS_PROCESSED;
	}
	int Finalize(ESCX_StateMachine* m, void*, const ESCX_Event* event)
	{
		Data* data = static_cast<Data*>(m->instance); ++data->finalizes; data->finalized = *event; return ESCX_STATUS_PROCESSED;
	}
	void Observe(ESCX_StateMachine* m, void*)
	{
		Data* data = static_cast<Data*>(m->instance); ++data->actions; data->seen = *ESCX_StateMachineGetCurrentEvent(m);
	}
	int Select(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id)
	{
		*transition_id = 0U; return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,Observe,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { Select }; const ESCX_UINT16_T bases[2] = { 0,0 }; const ESCX_BYTE_T owners[2] = { 0,1 }; const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,2,1,2 };
}
TEST(TransactionalInvokeEvent, FullQueueHasNoFinalizeSideEffectAndExactRetrySucceeds)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {}; Data data = {}; int payload = 19;
	ESCX_InvokeDescriptor invoke = { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start,0,0,0,0,Finalize };
	const ESCX_Event child = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_SCXML,9,83,&payload };
	const ESCX_Event filler = { 1,ESCX_EVENT_TYPE_INTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,0,0 };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &invoke, instance, 1));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	const ESCX_InvokeToken token = { 0U,instance[0].generation };
	m.processing = ESCX_TRUE;
	for (ESCX_BYTE_T i = 0U; i < ESCX_QUEUE_SIZE; ++i) 
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineRaiseEvent(&m, &filler));
	EXPECT_EQ(ESCX_STATUS_QUEUE_FULL, ESCX_StateMachineReceiveInvokeEventDescriptor(&m, token, &child));
	EXPECT_EQ(0, data.finalizes); EXPECT_TRUE(instance[0].active);
	ESCX_EventQueueInit(&m.internal_queue); m.processing = ESCX_FALSE;
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineReceiveInvokeEventDescriptor(&m, token, &child));
	EXPECT_EQ(1, data.finalizes); EXPECT_EQ(1, data.actions); 
	EXPECT_TRUE(instance[0].active);
	EXPECT_EQ(ESCX_EVENT_ORIGIN_INVOKE, data.finalized.origin_type); 
	EXPECT_EQ(0, data.finalized.invoke_id); EXPECT_EQ(83, data.finalized.send_id); 
	EXPECT_EQ(&payload, data.finalized.data);
	EXPECT_EQ(ESCX_EVENT_ORIGIN_INVOKE, data.seen.origin_type); 
	EXPECT_EQ(0, data.seen.invoke_id); EXPECT_EQ(83, data.seen.send_id); 
	EXPECT_EQ(&payload, data.seen.data);
}
