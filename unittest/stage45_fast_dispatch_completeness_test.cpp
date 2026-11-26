#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int event_actions; int eventless_checks;
	};
	void Act(ESCX_StateMachine* m, void*)
	{
		++static_cast<Data*>(m->instance)->event_actions;
	}
	int SelectAncestor(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}
	int NoEventless(ESCX_StateMachine* m, void*, ESCX_UINT16_T* transition_id) {
		++static_cast<Data*>(m->instance)->eventless_checks; *transition_id = (ESCX_UINT16_T)(ESCX_NO_TRANSITION); return ESCX_STATUS_PROCESSED;
	}
	/* event 1 is prefix parent; event 2 is child. State 1 owns selector for event 1. */
	const ESCX_StateDescriptor states[3] = {
	 { 0 },
	 { 0,0,0,0,0,0,2,0,0,ESCX_STATE_COMPOUND,1,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,1,0,0,0,1,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN }
	};
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,Act,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[2] = { SelectAncestor,NoEventless };
	const ESCX_UINT16_T bases[3] = { 0,0,0 }; const ESCX_BYTE_T owners[3] = { 0,1,0 }; const ESCX_UINT16_T slots[3] = { ESCX_NO_SELECTOR,0,ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,3,1,1,1,1,2,3,1,3 };
	const ESCX_BYTE_T parents[3] = { ESCX_NO_EVENT,ESCX_NO_EVENT,1 };
	/* Effective generated table: leaf 2, events 1 and 2 both select ancestor selector 0. */
	const ESCX_UINT16_T fast_bases[3] = { 0,0,0 }; const ESCX_BYTE_T fast_owners[3] = { 0,2,2 }; const ESCX_UINT16_T fast_slots[3] = { ESCX_NO_SELECTOR,0,0 };
	const ESCX_UINT16_T fast_eventless[3] = { ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,1 };
	const ESCX_FastDispatch complete = { fast_bases,fast_owners,fast_slots,3,fast_eventless,3 };
}
TEST(FastDispatchCompleteness, AcceptsFlattenedAncestorPrefixAndEventlessSemantics)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0,0 };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureEventMatching(&m, parents, 3, ESCX_NO_EVENT));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureFastDispatch(&m, &complete));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 2, 0));
	EXPECT_EQ(1, data.event_actions);
	EXPECT_GT(data.eventless_checks, 0);
}
TEST(FastDispatchCompleteness, RejectsMissingEffectiveAncestorEventMapping)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0,0 };
	const ESCX_BYTE_T bad_owners[3] = { 0,2,0 }; const ESCX_FastDispatch bad = { fast_bases,bad_owners,fast_slots,3,fast_eventless,3 };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureEventMatching(&m, parents, 3, ESCX_NO_EVENT));
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureFastDispatch(&m, &bad));
	EXPECT_EQ(nullptr, m.fast_dispatch);
}
TEST(FastDispatchCompleteness, RejectsIncorrectEffectiveEventlessSelector)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Data data = { 0,0 };
	const ESCX_UINT16_T bad_eventless[3] = { ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,ESCX_NO_SELECTOR };
	const ESCX_FastDispatch bad = { fast_bases,fast_owners,fast_slots,3,bad_eventless,3 };
	ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureEventMatching(&m, parents, 3, ESCX_NO_EVENT));
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureFastDispatch(&m, &bad));
}

TEST(FastDispatchCompleteness, RejectsSpuriousSelectorWhereHierarchyHasNoMapping)
{
    ESCX_StateMachine m = {};
    ESCX_BYTE_T active[1] = {};
    Data data = { 0,0 };

    /* Event 0 has no exact, prefix, wildcard, or ancestor selector in def. */
    const ESCX_UINT16_T spurious_bases[3] = { 0,0,0 };
    const ESCX_BYTE_T spurious_owners[3] = { 0,0,2 };
    const ESCX_UINT16_T spurious_slots[3] =
    {
        ESCX_NO_SELECTOR,
        ESCX_NO_SELECTOR,
        0
    };
    const ESCX_FastDispatch spurious =
    {
        spurious_bases,
        spurious_owners,
        spurious_slots,
        3,
        fast_eventless,
        3
    };

    ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);
    ASSERT_EQ(
        ESCX_STATUS_PROCESSED,
        ESCX_StateMachineConfigureEventMatching(
            &m,
            parents,
            3,
            ESCX_NO_EVENT));

    EXPECT_EQ(
        ESCX_STATUS_INVALID_ARGUMENT,
        ESCX_StateMachineConfigureFastDispatch(&m, &spurious));
    EXPECT_EQ(nullptr, m.fast_dispatch);
}
