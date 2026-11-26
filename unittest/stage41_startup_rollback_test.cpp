#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int entries; int exits; int invoke_starts; int invoke_cancels; int fail_entry; int fail_invoke;
	};
	int EnterParent(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); ++d->entries; return ESCX_STATUS_PROCESSED;
	}
	int ExitParent(ESCX_StateMachine* m, void*)
	{
		++static_cast<Data*>(m->instance)->exits; return ESCX_STATUS_PROCESSED;
	}
	int EnterLeaf(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); ++d->entries; return d->fail_entry ? ESCX_STATUS_CALLBACK_FAILED : ESCX_STATUS_PROCESSED;
	}
	int ExitLeaf(ESCX_StateMachine* m, void*)
	{
		++static_cast<Data*>(m->instance)->exits; return ESCX_STATUS_PROCESSED;
	}
	int StartInvoke(ESCX_StateMachine* m, void*, ESCX_InvokeToken)
	{
		Data* d = static_cast<Data*>(m->instance); ++d->invoke_starts; return d->fail_invoke ? ESCX_STATUS_CALLBACK_FAILED : ESCX_STATUS_PROCESSED;
	}
	void CancelInvoke(ESCX_StateMachine* m, void*, ESCX_InvokeToken)
	{
		++static_cast<Data*>(m->instance)->invoke_cancels;
	}
	const ESCX_StateDescriptor states[3] = { { 0 },{ 0,0,0,0,0,0,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,EnterParent,ExitParent },{ 0,0,0,1,0,0,0,1,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,EnterLeaf,ExitLeaf } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 }; const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } }; const ESCX_SelectorFunction selectors[1] = { 0 }; const ESCX_UINT16_T bases[3] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR }; const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,3,1,1,1,1,1,1,1,2 };
}
TEST(StartupRollback, EntryFailureExitsEnteredStatesAndAllowsRestart)
{
	ESCX_StateMachine m = {}; 
	ESCX_BYTE_T bits[1] = {}; Data data = { 0,0,0,0,1,0 }; 
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineStart(&m)); 
	EXPECT_FALSE(ESCX_StateMachineIsRunning(&m)); EXPECT_FALSE(ESCX_StateMachineIsInState(&m, 1)); 
	EXPECT_EQ(2, data.entries); EXPECT_EQ(2, data.exits); EXPECT_EQ(0, m.internal_queue.count); 
	EXPECT_EQ(0, m.external_queue.count);
	data.fail_entry = 0; EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_TRUE(ESCX_StateMachineIsInState(&m, 2));
}
TEST(StartupRollback, DeferredInvokeFailureRollsBackConfigurationAndCanRetry)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; ESCX_InvokeInstance instance[1] = {};
	Data data = { 0,0,0,0,0,1 }; 
	ESCX_InvokeDescriptor invoke = { 2,ESCX_NO_EVENT,ESCX_FALSE,0,StartInvoke,CancelInvoke }; 
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &invoke, instance, 1));
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineStart(&m)); 
	EXPECT_FALSE(ESCX_StateMachineIsRunning(&m)); EXPECT_FALSE(instance[0].active); 
	EXPECT_EQ(1, data.invoke_starts); EXPECT_EQ(0, data.invoke_cancels); EXPECT_EQ(2, data.exits);
	data.fail_invoke = 0; EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_TRUE(instance[0].active); EXPECT_EQ(2, data.invoke_starts);
}
namespace
{
	int FailParallelChild(ESCX_StateMachine*, void*)
	{
		return ESCX_STATUS_CALLBACK_FAILED;
	}
	const ESCX_StateDescriptor parallel_states[4] = { { 0 },{ 0,0,0,0,0,2,0,0,0,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,0,1,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,0,1,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,FailParallelChild } };
	const ESCX_BYTE_T parallel_children[2] = { 2,3 }; const ESCX_UINT16_T parallel_bases[4] = { 0 }; const ESCX_StateMachineDefinition parallel_def = { parallel_states,parallel_children,transitions,plans,plan_states,selectors,parallel_bases,owners,slots,4,2,1,1,1,1,1,1,2 };
}
TEST(StartupRollback, ParallelChildPreservesOriginalFailureStatus)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; ESCX_StateMachineInit(&m, &parallel_def, 0, bits, 1, 0, 0, 0); EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineStart(&m)); EXPECT_FALSE(ESCX_StateMachineIsRunning(&m)); EXPECT_EQ(0, m.active_atomic_count);
}
