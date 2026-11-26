#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int initializes; int starts; int fail_initialize; int fail_start;
	};
	int Initialize(ESCX_StateMachine* machine, void*, ESCX_InvokeToken)
	{
		Data* data = static_cast<Data*>(machine->instance); ++data->initializes;
		return data->fail_initialize ? ESCX_STATUS_CALLBACK_FAILED : ESCX_STATUS_PROCESSED;
	}
	int Start(ESCX_StateMachine* machine, void*, ESCX_InvokeToken)
	{
		Data* data = static_cast<Data*>(machine->instance); ++data->starts;
		return data->fail_start ? ESCX_STATUS_COMMUNICATION_ERROR : ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } }; const ESCX_SelectorFunction selectors[1] = { 0 };
	const ESCX_UINT16_T bases[2] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,1,1,2 };
	void Init(ESCX_StateMachine* machine, ESCX_BYTE_T* active, ESCX_InvokeInstance* instance, Data* data, ESCX_InvokeStartFunction start)
	{
		static ESCX_InvokeDescriptor invoke;
		invoke = ESCX_InvokeDescriptor{ 1,ESCX_NO_EVENT,ESCX_FALSE,0,start,0,0,0,0,0,Initialize };
		ESCX_StateMachineInit(machine, &def, data, active, 1, 0, 0, 0);
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(machine, &invoke, instance, 1));
	}
}
TEST(InvokeStartErrorClassification, InputInitializationFailureIsExecutionError)
{
	ESCX_StateMachine machine = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {}; Data data = { 0,0,1,0 };
	Init(&machine, active, instance, &data, Start);
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineStart(&machine));
	EXPECT_EQ(ESCX_ERROR_EXECUTION, ESCX_StateMachineGetLastError(&machine));
	EXPECT_EQ(1, data.initializes); EXPECT_EQ(0, data.starts); EXPECT_FALSE(instance[0].active);
}
TEST(InvokeStartErrorClassification, ServiceStartFailureIsCommunicationError)
{
	ESCX_StateMachine machine = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {}; Data data = { 0,0,0,1 };
	Init(&machine, active, instance, &data, Start);
	EXPECT_EQ(ESCX_STATUS_COMMUNICATION_ERROR, ESCX_StateMachineStart(&machine));
	EXPECT_EQ(ESCX_ERROR_COMMUNICATION, ESCX_StateMachineGetLastError(&machine));
	EXPECT_EQ(1, data.initializes); EXPECT_EQ(1, data.starts); EXPECT_FALSE(instance[0].active);
}
TEST(InvokeStartErrorClassification, MissingServiceStartIsCommunicationError)
{
	ESCX_StateMachine machine = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {}; Data data = {};
	Init(&machine, active, instance, &data, 0);
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineStart(&machine));
	EXPECT_EQ(ESCX_ERROR_COMMUNICATION, ESCX_StateMachineGetLastError(&machine));
	EXPECT_EQ(1, data.initializes); EXPECT_FALSE(instance[0].active);
}
