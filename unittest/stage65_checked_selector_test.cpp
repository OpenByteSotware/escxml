#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int failures; int error_actions;
	};
	int FailingGuard(ESCX_StateMachine* machine, void*, ESCX_UINT16_T* transition_id)
	{
		Data* data = static_cast<Data*>(machine->instance);
		++data->failures;
		*transition_id = 0U; /* Engine must discard output from a failed evaluation. */
		return ESCX_STATUS_CALLBACK_FAILED;
	}
	int SelectError(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id)
	{
		*transition_id = 0U;
		return ESCX_STATUS_PROCESSED;
	}
	void ObserveError(ESCX_StateMachine* machine, void*)
	{
		++static_cast<Data*>(machine->instance)->error_actions;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 };
	const ESCX_StatePlan plans[1] = { { 0,0 } };
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,ObserveError,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[2] = { SelectError,FailingGuard };
	const ESCX_UINT16_T bases[2] = { 0,0 };
	const ESCX_BYTE_T owners[3] = { 0,1,1 };
	const ESCX_UINT16_T slots[3] = { ESCX_NO_SELECTOR,1,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,2,3,1,3 };
}
TEST(CheckedSelector, FailureRaisesExecutionErrorAndIsTreatedAsFalse)
{
	ESCX_StateMachine machine = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0,0 };
	ESCX_StateMachineInit(&machine, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureErrorEvents(&machine, 2, ESCX_NO_EVENT));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&machine));
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&machine, 1, 0));
	EXPECT_EQ(1, data.failures);
	EXPECT_EQ(1, data.error_actions);
	EXPECT_EQ(ESCX_ERROR_EXECUTION, ESCX_StateMachineGetLastError(&machine));
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineGetLastErrorStatus(&machine));
}
