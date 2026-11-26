#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int cancels; int exits; ESCX_InvokeToken canceled;
	};
	int Start(ESCX_StateMachine*, void*, ESCX_InvokeToken)
	{
		return ESCX_STATUS_PROCESSED;
	}
	int FailCancel(ESCX_StateMachine* m, void*, ESCX_InvokeToken token)
	{
		Data* d = static_cast<Data*>(m->instance); ++d->cancels; d->canceled = token; return ESCX_STATUS_COMMUNICATION_ERROR;
	}
	int Exit(ESCX_StateMachine* m, void*)
	{
		++static_cast<Data*>(m->instance)->exits; return ESCX_STATUS_PROCESSED;
	}
	int Select(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id)
	{
		*transition_id = 0U; return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[3] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,0,Exit },{ 0,0,0,0,0,0,0,0,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[2] = { { 0,1 },{ 1,1 } }; const ESCX_BYTE_T plan_states[2] = { 1,2 };
	const ESCX_Transition transitions[1] = { { 0,1,0,ESCX_TRANSITION_EXTERNAL,1,0,0 } }; const ESCX_SelectorFunction selectors[1] = { Select };
	const ESCX_UINT16_T bases[3] = { 0,0,0 }; const ESCX_BYTE_T owners[2] = { 0,1 }; const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,3,1,1,2,2,1,2,1,2 };
}
TEST(InvokeCancelStatus, FailureInvalidatesTokenAndReportsCommunication)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instance[1] = {}; Data data = {};
	ESCX_InvokeDescriptor invoke = { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start,FailCancel };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(&m, &invoke, instance, 1));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	const ESCX_InvokeToken token = { 0U,instance[0].generation };
	EXPECT_EQ(ESCX_STATUS_COMMUNICATION_ERROR, ESCX_StateMachineSend(&m, 1, 0));
	EXPECT_EQ(1, data.cancels); EXPECT_EQ(0, data.exits); EXPECT_FALSE(instance[0].active);
	EXPECT_EQ(ESCX_ERROR_COMMUNICATION, ESCX_StateMachineGetLastError(&m));
	EXPECT_EQ(ESCX_STATUS_COMMUNICATION_ERROR, ESCX_StateMachineGetLastErrorStatus(&m));
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineReceiveInvokeEvent(&m, token, 1, 0));
}
