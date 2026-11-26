#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		ESCX_InvokeToken started; int finalizes;
	};
	int Start(ESCX_StateMachine* machine, void*, ESCX_InvokeToken token)
	{
		static_cast<Data*>(machine->instance)->started = token;
		return ESCX_STATUS_PROCESSED;
	}
	int Finalize(ESCX_StateMachine* machine, void*, const ESCX_Event*)
	{
		++static_cast<Data*>(machine->instance)->finalizes;
		return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[2] = 
	{ { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 };
	const ESCX_StatePlan plans[1] = { { 0,0 } };
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = 
	{ { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { 0 };
	const ESCX_UINT16_T bases[2] = { 0,0 };
	const ESCX_BYTE_T owners[1] = { 0 };
	const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = 
	{ states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,1,1,2 };
}
TEST(InvokeInstanceToken, RejectsStaleEventAfterDescriptorReuse)
{
	ESCX_StateMachine machine = {}; ESCX_BYTE_T active[1] = {}; 
	ESCX_InvokeInstance instance[1] = {}; Data data = {};
	ESCX_InvokeDescriptor invoke = { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start,0,0,0,0,Finalize };
	ESCX_StateMachineInit(&machine, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&machine, &invoke, instance, 1));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&machine));
	const ESCX_InvokeToken stale = data.started;
	instance[0].active = ESCX_FALSE;
	++instance[0].generation;
	if (instance[0].generation == 0U) ++instance[0].generation;
	instance[0].active = ESCX_TRUE;
	const ESCX_Event event = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_INVOKE,0,0,0 };
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineReceiveInvokeEventDescriptor(&machine, stale, &event));
	EXPECT_EQ(0, data.finalizes);
	EXPECT_TRUE(instance[0].active);
}
