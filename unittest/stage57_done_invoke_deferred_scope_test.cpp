#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}

static ESCX_InvokeToken InvokeTokenFor(const ESCX_StateMachine* m, ESCX_BYTE_T i) { ESCX_InvokeToken t = { i,m->invoke_instances[i].generation }; return t; }
namespace
{
	struct Data
	{
		ESCX_Event finalized; ESCX_Event transitioned; int finalizes; int transitions;
	};
	int Start(ESCX_StateMachine*, void*, ESCX_InvokeToken)
	{
		return ESCX_STATUS_PROCESSED;
	}
	int Finalize(ESCX_StateMachine* m, void*, const ESCX_Event* e)
	{
		Data* d = static_cast<Data*>(m->instance); d->finalized = *e; ++d->finalizes;
		return ESCX_StateMachineGetCurrentEvent(m)->invoke_id == e->invoke_id ? ESCX_STATUS_PROCESSED : ESCX_STATUS_CALLBACK_FAILED;
	}
	void Observe(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); d->transitioned = *ESCX_StateMachineGetCurrentEvent(m); ++d->transitions;
	}
	int Select(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[2] = 
	{ { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; 
	const ESCX_StatePlan plans[1] = { { 0,0 } }; 
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = 
	{ { ESCX_NO_PLAN,ESCX_NO_PLAN,Observe,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { Select }; 
	const ESCX_UINT16_T bases[2] = { 0,0 };
	const ESCX_BYTE_T owners[2] = { 0,1 }; 
	const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = 
	{ states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,2,1,2 };
	void Init(ESCX_StateMachine* m, ESCX_BYTE_T* active, ESCX_InvokeInstance* instance, Data* data)
	{
		static ESCX_InvokeDescriptor invoke;
		invoke = ESCX_InvokeDescriptor{ 1,1,ESCX_FALSE,0,Start,0,0,0,0,Finalize };
		ESCX_StateMachineInit(m, &def, data, active, 1, 0, 0, 0);
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(m, &invoke, instance, 1));
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(m));
	}
}
TEST(DoneInvokeDeferredScope, CompletionDuringMacrostepQueuesDescriptorAndRestoresPriorEvent)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {};
	Data data = {}; int payload = 17;
	ESCX_Event done = { 1,ESCX_EVENT_TYPE_INTERNAL,ESCX_EVENT_ORIGIN_SCXML,9,66,&payload };
	Init(&m, active, instance, &data); m.processing = ESCX_TRUE;
	int prior_payload = 23;
	m.current_event_descriptor = ESCX_Event{ 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,ESCX_NO_INVOKE,44,&prior_payload };
	m.current_event = 1;
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineCompleteInvokeEvent(&m, InvokeTokenFor(&m, 0), &done));
	EXPECT_FALSE(instance[0].active); ASSERT_EQ(1, data.finalizes); 
	EXPECT_EQ(0, data.transitions); ASSERT_EQ(1, m.internal_queue.count);
	EXPECT_EQ(ESCX_EVENT_ORIGIN_INVOKE, data.finalized.origin_type);
	EXPECT_EQ(0, data.finalized.invoke_id); 
	EXPECT_EQ(66, data.finalized.send_id); 
	EXPECT_EQ(&payload, data.finalized.data);
	const ESCX_Event* current = ESCX_StateMachineGetCurrentEvent(&m); 
	ASSERT_NE(nullptr, current);
	EXPECT_EQ(1, current->name_id);
	EXPECT_EQ(ESCX_EVENT_ORIGIN_PLATFORM, current->origin_type);
	EXPECT_EQ(ESCX_NO_INVOKE, current->invoke_id);
	EXPECT_EQ(44, current->send_id);
	EXPECT_EQ(&prior_payload, current->data);
	m.processing = ESCX_FALSE;
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0));
	ASSERT_EQ(2, data.transitions);
	EXPECT_EQ(ESCX_EVENT_ORIGIN_INVOKE, data.transitioned.origin_type); 
	EXPECT_EQ(0, data.transitioned.invoke_id); 
	EXPECT_EQ(66, data.transitioned.send_id); 
	EXPECT_EQ(&payload, data.transitioned.data);
}
