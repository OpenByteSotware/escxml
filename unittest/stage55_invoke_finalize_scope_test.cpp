#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		ESCX_Event finalized; int count;
	};
	int Start(ESCX_StateMachine*, void*)
	{
		return ESCX_STATUS_PROCESSED;
	}
	int Finalize(ESCX_StateMachine* m, void*, const ESCX_Event* e)
	{
		Data* d = static_cast<Data*>(m->instance); d->finalized = *e; ++d->count;
		return ESCX_StateMachineGetCurrentEvent(m)->name_id == e->name_id ? ESCX_STATUS_PROCESSED : ESCX_STATUS_CALLBACK_FAILED;
	}
	const ESCX_StateDescriptor states[2] = 
	{ { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; 
	const ESCX_StatePlan plans[1] = { { 0,0 } }; 
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = 
	{ { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { 0 };
	const ESCX_UINT16_T bases[2] = { 0 };
	const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def =
	{ states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,1,1,2 };
	void Init(ESCX_StateMachine* m, ESCX_BYTE_T* active, 
		ESCX_InvokeInstance* instance, Data* data, ESCX_BYTE_T done_event)
	{
		static ESCX_InvokeDescriptor invoke;
		invoke = ESCX_InvokeDescriptor{ 1,done_event,ESCX_FALSE,0,Start,0,0,0,0,Finalize };
		ESCX_StateMachineInit(m, &def, data, active, 1, 0, 0, 0);
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(m, &invoke, instance, 1));
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(m));
	}
}
TEST(InvokeFinalizeScope, CompletionWithoutDoneEventLeavesNeutralCurrentEvent)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {};
	Data data = {}; int payload = 4;
	Init(&m, active, instance, &data, ESCX_NO_EVENT);
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineCompleteInvoke(&m, 0, &payload));
	ASSERT_EQ(1, data.count); EXPECT_EQ(&payload, data.finalized.data);
	const ESCX_Event* current = ESCX_StateMachineGetCurrentEvent(&m); ASSERT_NE(nullptr, current);
	EXPECT_EQ(ESCX_NO_EVENT, current->name_id); EXPECT_EQ(ESCX_EVENT_ORIGIN_NONE, current->origin_type); EXPECT_EQ(nullptr, current->data);
}
TEST(InvokeFinalizeScope, QueueFullFailureDoesNotLeakFinalizeEvent)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {}; 
	Data data = {};
	ESCX_Event filler = { 1,ESCX_EVENT_TYPE_INTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,0,0 };
	Init(&m, active, instance, &data, 1);
	m.processing = ESCX_TRUE;
	for (ESCX_BYTE_T i = 0; i < ESCX_QUEUE_SIZE; ++i) 
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineRaiseEvent(&m, &filler));
	EXPECT_EQ(ESCX_STATUS_QUEUE_FULL, ESCX_StateMachineCompleteInvoke(&m, 0, 0));
	m.processing = ESCX_FALSE;
	const ESCX_Event* current = ESCX_StateMachineGetCurrentEvent(&m); 
	ASSERT_NE(nullptr, current);
	EXPECT_EQ(ESCX_NO_EVENT, current->name_id); EXPECT_EQ(ESCX_EVENT_ORIGIN_NONE, current->origin_type);
}
TEST(InvokeFinalizeScope, EventQueuedDuringExistingMacrostepRestoresNeutralScope)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {}; Data data = {}; int payload = 7;
	Init(&m, active, instance, &data, 1); m.processing = ESCX_TRUE;
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineReceiveInvokeEvent(&m, 0, 1, &payload));
	EXPECT_EQ(1, m.internal_queue.count); EXPECT_EQ(1, data.count);
	const ESCX_Event* current = ESCX_StateMachineGetCurrentEvent(&m); ASSERT_NE(nullptr, current);
	EXPECT_EQ(ESCX_NO_EVENT, current->name_id); EXPECT_EQ(ESCX_EVENT_ORIGIN_NONE, current->origin_type);
	m.processing = ESCX_FALSE;
}
