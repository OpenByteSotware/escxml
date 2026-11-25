#include <gtest/gtest.h>

extern "C"
{
#include "StateMachine.h"
}

namespace
{
    struct FastData
    {
        int actions;
    };

    void Act(ESCX_StateMachine* m, void*)
    {
        static_cast<FastData*>(m->instance)->actions++;
    }

    ESCX_UINT16_T Select(ESCX_StateMachine*, void*)
    {
        return 0;
    }

    const ESCX_StateDescriptor states[3] =
    {
        { 0 },
        { 0,0,0,0,0,0,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
        { 0,0,0,1,0,0,0,1,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN }
    };

    const ESCX_BYTE_T children[1] = { 0 };
    const ESCX_StatePlan plans[1] = { { 0,0 } };
    const ESCX_BYTE_T plan_states[1] = { 0 };

    const ESCX_Transition transitions[1] =
    {
        { ESCX_NO_PLAN,ESCX_NO_PLAN,Act,ESCX_TRANSITION_TARGETLESS,1,0,0 }
    };

    const ESCX_SelectorFunction selectors[1] = { Select };

    /*
     * Baseline hierarchical dispatch declares event 2 on compound state 1.
     * Normal dispatch reaches selector 0 by walking from atomic state 2 to
     * its ancestor state 1.
     */
    const ESCX_UINT16_T bases[3] = { 0,0,0 };
    const ESCX_BYTE_T owners[3] = { 0,0,1 };
    const ESCX_UINT16_T slots[3] =
    {
        ESCX_NO_SELECTOR,
        ESCX_NO_SELECTOR,
        0
    };

    const ESCX_StateMachineDefinition def =
    {
        states,
        children,
        transitions,
        plans,
        plan_states,
        selectors,
        bases,
        owners,
        slots,
        3,
        1,
        1,
        1,
        1,
        1,
        3,
        1,
        3
    };

    /*
     * Effective fast dispatch flattens the inherited mapping. Atomic state 2
     * therefore resolves event 2 directly to selector 0 without an ancestor
     * walk during event processing.
     */
    const ESCX_UINT16_T fast_bases[3] = { 0,0,0 };
    const ESCX_BYTE_T fast_owners[3] = { 0,0,2 };
    const ESCX_UINT16_T fast_slots[3] =
    {
        ESCX_NO_SELECTOR,
        ESCX_NO_SELECTOR,
        0
    };

    const ESCX_FastDispatch fast =
    {
        fast_bases,
        fast_owners,
        fast_slots,
        3,
        nullptr,
        0
    };
}

TEST(FastDispatch, UsesOneEffectiveAtomicStateLookup)
{
    ESCX_StateMachine m = {};
    ESCX_BYTE_T active[1] = {};
    ESCX_BYTE_T exit_states[2] = {};
    ESCX_BYTE_T entry_states[2] = {};
    ESCX_BYTE_T exit_seen[1] = {};
    ESCX_BYTE_T entry_seen[1] = {};
    ESCX_MicrostepWorkspace w =
    {
        exit_states,
        2,
        0,
        entry_states,
        2,
        0,
        exit_seen,
        entry_seen,
        1
    };
    FastData data = { 0 };

    ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);

    ASSERT_EQ(
        ESCX_STATUS_PROCESSED,
        ESCX_StateMachineConfigureFastDispatch(&m, &fast));
    ASSERT_EQ(
        ESCX_STATUS_PROCESSED,
        ESCX_StateMachineConfigureWorkspace(&m, &w));
    ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));

    EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 2, 0));
    EXPECT_EQ(1, data.actions);
}
