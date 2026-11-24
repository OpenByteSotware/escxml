#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		ESCX_Event selector_event; ESCX_Event action_event; ESCX_Event error_trigger; int selected; int acted; int phase;
	};
	ESCX_UINT16_T SelectEventless(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); d->selector_event = *ESCX_StateMachineGetCurrentEvent(m); ++d->selected; return d->phase == 0 ? 0 : ESCX_NO_TRANSITION;
	}
	void EventlessAction(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); d->action_event = *ESCX_StateMachineGetCurrentEvent(m); ++d->acted; d->phase = 1;
		(void)ESCX_StateMachineReportError(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_CALLBACK_FAILED);
		d->error_trigger = ESCX_StateMachineGetLastErrorData(m)->triggering_event;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,0,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,EventlessAction,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { SelectEventless }; const ESCX_UINT16_T bases[2] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,1,1,2 };
}
TEST(CurrentEventScoping, EventlessSelectorAndActionSeeNeutralEvent)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = {};
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	ASSERT_EQ(2, data.selected); ASSERT_EQ(1, data.acted);
	EXPECT_EQ(ESCX_NO_EVENT, data.selector_event.name_id); EXPECT_EQ(ESCX_EVENT_ORIGIN_NONE, data.selector_event.origin_type);
	EXPECT_EQ(ESCX_NO_EVENT, data.action_event.name_id); EXPECT_EQ(ESCX_EVENT_ORIGIN_NONE, data.action_event.origin_type);
	EXPECT_EQ(ESCX_NO_EVENT, data.error_trigger.name_id);
}
namespace
{
	struct DrivenData
	{
		int phase; ESCX_Event eventless_seen; ESCX_Event trigger_seen;
	};
	ESCX_UINT16_T SelectDriven(ESCX_StateMachine* m, void*)
	{
		DrivenData* d = static_cast<DrivenData*>(m->instance); if (d->phase != 1)return ESCX_NO_TRANSITION; d->eventless_seen = *ESCX_StateMachineGetCurrentEvent(m); return 1;
	}
	ESCX_UINT16_T SelectExternal(ESCX_StateMachine* m, void*)
	{
		static_cast<DrivenData*>(m->instance)->phase = 1; return 0;
	}
	void CaptureError(ESCX_StateMachine* m, void*)
	{
		(void)ESCX_StateMachineReportError(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_CALLBACK_FAILED);
		static_cast<DrivenData*>(m->instance)->trigger_seen = ESCX_StateMachineGetLastErrorData(m)->triggering_event; static_cast<DrivenData*>(m->instance)->phase = 2;
	}
	const ESCX_StateDescriptor driven_states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,1,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_Transition driven_transitions[2] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,1,0,0 },{ ESCX_NO_PLAN,ESCX_NO_PLAN,CaptureError,ESCX_TRANSITION_TARGETLESS,1,0,1 } };
	const ESCX_SelectorFunction driven_selectors[2] = { SelectExternal,SelectDriven }; const ESCX_UINT16_T driven_bases[2] = { 0 }; const ESCX_BYTE_T driven_owners[2] = { 0,1 }; const ESCX_UINT16_T driven_slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition driven_def = { driven_states,children,driven_transitions,plans,plan_states,driven_selectors,driven_bases,driven_owners,driven_slots,2,1,2,1,1,2,2,1,2 };
}
TEST(CurrentEventScoping, ErrorDuringEventlessUsesDrivingEventAsStructuredTrigger)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; DrivenData data = {}; int payload = 9;
	ESCX_Event driving = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,ESCX_NO_INVOKE,72,&payload };
	ESCX_StateMachineInit(&m, &driven_def, &data, bits, 1, 0, 0, 0); ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendEvent(&m, &driving));
	EXPECT_EQ(ESCX_NO_EVENT, data.eventless_seen.name_id); EXPECT_EQ(ESCX_EVENT_ORIGIN_NONE, data.eventless_seen.origin_type);
	EXPECT_EQ(1, data.trigger_seen.name_id); EXPECT_EQ(72, data.trigger_seen.send_id); EXPECT_EQ(&payload, data.trigger_seen.data);
}
