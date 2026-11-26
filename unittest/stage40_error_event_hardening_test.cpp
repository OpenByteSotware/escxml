#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int actions; int nested_reports; ESCX_ErrorEventData delivered;
	};
	void ObserveError(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); ++d->actions;
		d->delivered = *static_cast<ESCX_ErrorEventData*>(ESCX_StateMachineGetCurrentEvent(m)->data);
		if (d->nested_reports == 0) {
			++d->nested_reports; (void)ESCX_StateMachineReportError(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_CALLBACK_FAILED);
		}
	}
	int SelectError(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,ObserveError,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { SelectError }; const ESCX_UINT16_T bases[2] = { 0 };
	const ESCX_BYTE_T owners[4] = { 0,0,1,1 }; const ESCX_UINT16_T slots[4] = { ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,0,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,4,1,4 };
}
TEST(ErrorEventHardening, PendingPayloadIsStableAndSecondReportDoesNotEnqueue)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = {}; int payload = 9;
	ESCX_Event driving = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,ESCX_NO_INVOKE,77,&payload };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureErrorEvents(&m, 2, 3)); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	m.processing = ESCX_TRUE; EXPECT_EQ(ESCX_STATUS_INVALID_TARGET, ESCX_StateMachineReportError(&m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET)); EXPECT_TRUE(m.reporting_error); EXPECT_EQ(1, m.internal_queue.count);
	EXPECT_EQ(ESCX_STATUS_CALLBACK_FAILED, ESCX_StateMachineReportError(&m, ESCX_ERROR_EXECUTION, ESCX_STATUS_CALLBACK_FAILED)); EXPECT_EQ(1, m.internal_queue.count); m.processing = ESCX_FALSE;
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendEvent(&m, &driving)); 
	ASSERT_EQ(1, data.actions); EXPECT_EQ(ESCX_ERROR_COMMUNICATION, data.delivered.code); EXPECT_EQ(ESCX_STATUS_INVALID_TARGET, data.delivered.cause); EXPECT_FALSE(m.reporting_error);
}
TEST(ErrorEventHardening, ErrorRaisedWhileHandlingPlatformErrorDoesNotRecurse)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = {};
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureErrorEvents(&m, 2, 3)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_STATUS_INVALID_TARGET, ESCX_StateMachineReportError(&m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET)); EXPECT_EQ(1, data.actions); EXPECT_EQ(1, data.nested_reports); EXPECT_EQ(0, m.internal_queue.count); EXPECT_FALSE(m.reporting_error);
}
TEST(ErrorEventHardening, QueueFullDoesNotLatchAndLaterReportCanRetry)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = {};
	ESCX_Event filler = { 1,ESCX_EVENT_TYPE_INTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,0,0 }; 
	ESCX_BYTE_T i;
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureErrorEvents(&m, 2, 3)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	for (i = 0; i < ESCX_QUEUE_SIZE; ++i)ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineRaiseEvent(&m, &filler)); m.processing = ESCX_TRUE; EXPECT_EQ(ESCX_STATUS_INVALID_TARGET, ESCX_StateMachineReportError(&m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET)); EXPECT_FALSE(m.reporting_error); m.processing = ESCX_FALSE; ESCX_EventQueueInit(&m.internal_queue); EXPECT_EQ(ESCX_STATUS_INVALID_TARGET, ESCX_StateMachineReportError(&m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET)); EXPECT_EQ(1, data.actions); EXPECT_FALSE(m.reporting_error);
}
