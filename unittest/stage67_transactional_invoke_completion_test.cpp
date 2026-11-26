#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int done_calls; int finalize_calls; int fail_done; int fail_finalize;
	};
	int Start(ESCX_StateMachine*, void*, ESCX_InvokeToken)
	{
		return ESCX_STATUS_PROCESSED;
	}
	int DoneData(ESCX_StateMachine* m, void** payload)
	{
		Data* d = static_cast<Data*>(m->instance); ++d->done_calls; *payload = d;
		return d->fail_done ? ESCX_STATUS_CALLBACK_FAILED : ESCX_STATUS_PROCESSED;
	}
	int Finalize(ESCX_StateMachine* m, void*, const ESCX_Event*)
	{
		Data* d = static_cast<Data*>(m->instance); ++d->finalize_calls;
		return d->fail_finalize ? ESCX_STATUS_CALLBACK_FAILED : ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { 0 }; const ESCX_UINT16_T bases[2] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,1,1,3 };
	void Init(ESCX_StateMachine* m, ESCX_BYTE_T* active, ESCX_InvokeInstance* instance, Data* data)
	{
		static ESCX_InvokeDescriptor invoke;
		invoke = ESCX_InvokeDescriptor{ 1,1,ESCX_FALSE,0,Start,0,0,0,DoneData,Finalize };
		ESCX_StateMachineInit(m, &def, data, active, 1, 0, 0, 0);
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(m, &invoke, instance, 1));
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(m));
	}
	ESCX_InvokeToken Token(const ESCX_InvokeInstance* instance)
	{
		return ESCX_InvokeToken{ 0U,instance[0].generation };
	}
}
TEST(TransactionalInvokeCompletion, DoneDataFailureKeepsInvocationActiveForRetry)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {}; Data data = { 0,0,1,0 };
	Init(&m, active, instance, &data); const ESCX_InvokeToken token = Token(instance);
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineCompleteInvoke(&m, token, 0));
	EXPECT_TRUE(instance[0].active); data.fail_done = 0;
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineCompleteInvoke(&m, token, 0));
	EXPECT_FALSE(instance[0].active); EXPECT_EQ(2, data.done_calls); EXPECT_EQ(1, data.finalize_calls);
}
TEST(TransactionalInvokeCompletion, FinalizeFailureKeepsInvocationActiveForRetry)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {}; Data data = { 0,0,0,1 };
	Init(&m, active, instance, &data); const ESCX_InvokeToken token = Token(instance);
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineCompleteInvoke(&m, token, 0));
	EXPECT_TRUE(instance[0].active); data.fail_finalize = 0;
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineCompleteInvoke(&m, token, 0));
	EXPECT_FALSE(instance[0].active); EXPECT_EQ(2, data.finalize_calls);
}
TEST(TransactionalInvokeCompletion, FullQueueDoesNotFinalizeOrConsumeInvocation)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {}; Data data = {};
	Init(&m, active, instance, &data); const ESCX_InvokeToken token = Token(instance);
	const ESCX_Event filler = { 2,ESCX_EVENT_TYPE_INTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,0,0 };
	m.processing = ESCX_TRUE; for (ESCX_BYTE_T i = 0U; i < ESCX_QUEUE_SIZE; ++i) ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineRaiseEvent(&m, &filler));
	EXPECT_EQ(ESCX_STATUS_QUEUE_FULL, ESCX_StateMachineCompleteInvoke(&m, token, 0));
	EXPECT_TRUE(instance[0].active); EXPECT_EQ(0, data.done_calls); EXPECT_EQ(0, data.finalize_calls);
	ESCX_EventQueueInit(&m.internal_queue); m.processing = ESCX_FALSE;
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineCompleteInvoke(&m, token, 0)); EXPECT_FALSE(instance[0].active);
}
