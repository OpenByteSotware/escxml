#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int errors; ESCX_ErrorEventData delivered;
	};
	void ObserveError(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); ++d->errors;
		d->delivered = *static_cast<ESCX_ErrorEventData*>(ESCX_StateMachineGetCurrentEvent(m)->data);
	}
	int SelectError(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id)
	{
		*transition_id = 0U; return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,ObserveError,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { SelectError }; const ESCX_UINT16_T bases[2] = { 0,0 };
	const ESCX_BYTE_T owners[4] = { 0,0,0,1 }; const ESCX_UINT16_T slots[4] = { ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,4,1,4 };
}
TEST(PendingErrorDelivery, FullQueueRetainsFirstErrorUntilCapacityIsAvailable)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = {};
	const ESCX_Event filler = { 1,ESCX_EVENT_TYPE_INTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,0,0 };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureErrorEvents(&m, 2, 3));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	for (ESCX_BYTE_T i = 0U; i < ESCX_QUEUE_SIZE; ++i) ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineRaiseEvent(&m, &filler));
	m.processing = ESCX_TRUE;
	EXPECT_EQ(ESCX_STATUS_INVALID_TARGET, ESCX_StateMachineReportError(&m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET));
	EXPECT_TRUE(m.pending_error_delivery); EXPECT_FALSE(m.reporting_error);
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineReportError(&m, ESCX_ERROR_EXECUTION, ESCX_STATUS_CALLBACK_FAILED));
	EXPECT_EQ(ESCX_ERROR_COMMUNICATION, m.pending_error_event_data.code);
	m.processing = ESCX_FALSE;
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0));
	EXPECT_EQ(1, data.errors); EXPECT_EQ(ESCX_ERROR_COMMUNICATION, data.delivered.code);
	EXPECT_EQ(ESCX_STATUS_INVALID_TARGET, data.delivered.cause);
	EXPECT_FALSE(m.pending_error_delivery); EXPECT_FALSE(m.reporting_error);
}
TEST(PendingErrorDelivery, ClearDiscardsUndeliveredPendingError)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = {};
	const ESCX_Event filler = { 1,ESCX_EVENT_TYPE_INTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,0,0 };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureErrorEvents(&m, 2, 3)); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	for (ESCX_BYTE_T i = 0U; i < ESCX_QUEUE_SIZE; ++i) ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineRaiseEvent(&m, &filler));
	m.processing = ESCX_TRUE; (void)ESCX_StateMachineReportError(&m, ESCX_ERROR_EXECUTION, ESCX_STATUS_CALLBACK_FAILED); m.processing = ESCX_FALSE;
	ASSERT_TRUE(m.pending_error_delivery); ESCX_StateMachineClearError(&m); EXPECT_FALSE(m.pending_error_delivery);
}
