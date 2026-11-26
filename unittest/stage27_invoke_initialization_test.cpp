#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int log[4]; int count; ESCX_BYTE_T invoke_id;
	};
	int Initialize(ESCX_StateMachine* m, void*, ESCX_InvokeToken token)
	{
		Data* d = static_cast<Data*>(m->instance); d->log[d->count++] = 1; d->invoke_id = token.descriptor_index;
		return ESCX_STATUS_PROCESSED;
	}
	int Start(ESCX_StateMachine* m, void*, ESCX_InvokeToken)
	{
		static_cast<Data*>(m->instance)->log[static_cast<Data*>(m->instance)->count++] = 2; 
		return ESCX_STATUS_PROCESSED;
	}
	int FailInitialize(ESCX_StateMachine*, void*, ESCX_InvokeToken token)
	{
		return ESCX_STATUS_CALLBACK_FAILED;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 };
	const ESCX_StatePlan plans[1] = { { 0,0 } }; 
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } }; const ESCX_SelectorFunction selectors[1] = { 0 }; const ESCX_UINT16_T bases[2] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR }; const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,1,1,2 };
}
TEST(InvokeInitialization, InitializesParametersBeforeStart)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; ESCX_InvokeInstance instances[1] = {};
	Data data = { { 0 },0,ESCX_NO_INVOKE }; 
	ESCX_InvokeDescriptor invoke = { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start,0,0,0,0,0,Initialize };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &invoke, instances, 1));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	ASSERT_EQ(2, data.count);
	EXPECT_EQ(1, data.log[0]); EXPECT_EQ(2, data.log[1]); 
	EXPECT_EQ(0, data.invoke_id); 
	EXPECT_TRUE(instances[0].active);
}
TEST(InvokeInitialization, FailurePreventsStartAndRecordsExecutionError)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; ESCX_InvokeInstance instances[1] = {}; 
	Data data = { { 0 },0,ESCX_NO_INVOKE }; 
	ESCX_InvokeDescriptor invoke = { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start,0,0,0,0,0,FailInitialize };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &invoke, instances, 1)); 
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineStart(&m)); 
	EXPECT_FALSE(instances[0].active); 
	EXPECT_EQ(ESCX_ERROR_EXECUTION, ESCX_StateMachineGetLastError(&m));
}
