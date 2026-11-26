#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int log[8]; int count; int cancels;
	};
	int Init0(ESCX_StateMachine* m, void*, ESCX_InvokeToken token)
	{
		static_cast<Data*>(m->instance)->log[static_cast<Data*>(m->instance)->count++] = 1; return ESCX_STATUS_PROCESSED;
	}
	int Start0(ESCX_StateMachine* m, void*, ESCX_InvokeToken)
	{
		static_cast<Data*>(m->instance)->log[static_cast<Data*>(m->instance)->count++] = 2; return ESCX_STATUS_PROCESSED;
	}
	int Init1(ESCX_StateMachine* m, void*, ESCX_InvokeToken token)
	{
		static_cast<Data*>(m->instance)->log[static_cast<Data*>(m->instance)->count++] = 3; return ESCX_STATUS_PROCESSED;
	}
	int Start1(ESCX_StateMachine* m, void*, ESCX_InvokeToken)
	{
		static_cast<Data*>(m->instance)->log[static_cast<Data*>(m->instance)->count++] = 4; return ESCX_STATUS_PROCESSED;
	}
	void Cancel(ESCX_StateMachine* m, void*, ESCX_InvokeToken)
	{
		++static_cast<Data*>(m->instance)->cancels;
	}
	int TakeEventless(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor stable_states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_StateDescriptor transient_states[3] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,0,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,0,0,0,0,0,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 };
	const ESCX_StatePlan stable_plans[1] = { { 0,0 } }; const ESCX_BYTE_T stable_plan_states[1] = { 0 };
	const ESCX_StatePlan transient_plans[2] = { { 0,1 },{ 1,1 } }; const ESCX_BYTE_T transient_plan_states[2] = { 1,2 };
	const ESCX_Transition stable_transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } };
	const ESCX_Transition transient_transitions[1] = { { 0,1,0,ESCX_TRANSITION_EXTERNAL,1,0,0 } };
	const ESCX_SelectorFunction stable_selectors[1] = { 0 }; const ESCX_SelectorFunction transient_selectors[1] = { TakeEventless };
	const ESCX_UINT16_T stable_base[2] = { 0 }; const ESCX_UINT16_T transient_base[3] = { 0 };
	const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition stable_def = { stable_states,children,stable_transitions,stable_plans,stable_plan_states,stable_selectors,stable_base,owners,slots,2,1,1,1,1,1,1,1,2 };
	const ESCX_StateMachineDefinition transient_def = { transient_states,children,transient_transitions,transient_plans,transient_plan_states,transient_selectors,transient_base,owners,slots,3,1,1,2,2,1,1,1,2 };
}
TEST(InvokeOwnership, GeneratedRangeStartsMultipleInvokesInDocumentOrderAfterStabilization)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; ESCX_InvokeInstance instances[2] = {}; 
	Data data = { { 0 },0,0 };
	ESCX_InvokeDescriptor invokes[2] = { { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start0,Cancel,0,0,0,0,Init0 },{ 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start1,Cancel,0,0,0,0,Init1 } };
	const ESCX_UINT16_T first[2] = { 0,0 }; const ESCX_BYTE_T count[2] = { 0,2 }; const ESCX_InvokeMap map = { first,count,2,2 };
	ESCX_StateMachineInit(&m, &stable_def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, invokes, instances, 2));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokeMap(&m, &map));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	ASSERT_EQ(4, data.count); EXPECT_EQ(1, data.log[0]); EXPECT_EQ(2, data.log[1]);
	EXPECT_EQ(3, data.log[2]); EXPECT_EQ(4, data.log[3]);
	EXPECT_TRUE(instances[0].active); EXPECT_TRUE(instances[1].active);
	ESCX_StateMachineStop(&m); EXPECT_EQ(2, data.cancels);
}
TEST(InvokeOwnership, EventlessExitPreventsPendingInvokeStartup)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; ESCX_InvokeInstance instance[1] = {}; 
	Data data = { { 0 },0,0 };
	ESCX_InvokeDescriptor invoke = { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start0,Cancel,0,0,0,0,Init0 };
	const ESCX_UINT16_T first[3] = { 0,0,1 }; const ESCX_BYTE_T count[3] = { 0,1,0 };
	const ESCX_InvokeMap map = { first,count,3,1 };
	ESCX_StateMachineInit(&m, &transient_def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &invoke, instance, 1));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokeMap(&m, &map));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	EXPECT_EQ(0, data.count); EXPECT_FALSE(instance[0].active);
	EXPECT_TRUE(ESCX_StateMachineIsInState(&m, 2));
}
TEST(InvokeOwnership, RejectsRangeWhoseDescriptorHasDifferentOwner)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; ESCX_InvokeInstance instance[1] = {}; 
	Data data = { { 0 },0,0 };
	ESCX_InvokeDescriptor invoke = { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start0,Cancel };
	const ESCX_UINT16_T first[2] = { 0,0 }; const ESCX_BYTE_T count[2] = { 1,0 }; 
	const ESCX_InvokeMap map = { first,count,2,1 };
	ESCX_StateMachineInit(&m, &stable_def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &invoke, instance, 1));
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureInvokeMap(&m, &map));
}
