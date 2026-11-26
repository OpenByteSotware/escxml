#include <gtest/gtest.h>

extern "C"
{
#include "StateMachine.h"
}

namespace
{
    struct Data
    {
        int a;
        int b;
    };

    void A(ESCX_StateMachine* m, void*)
    {
        static_cast<Data*>(m->instance)->a++;
    }

    void B(ESCX_StateMachine* m, void*)
    {
        static_cast<Data*>(m->instance)->b++;
    }

    int SA(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
        *transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
    }

    int SB(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
        *transition_id = (ESCX_UINT16_T)(1); return ESCX_STATUS_PROCESSED;
    }

    const ESCX_StateDescriptor states[4] =
    {
        { 0 },
        { 0,0,0,0,0,2,0,0,0,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
        { 0,0,0,1,0,0,0,1,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
        { 0,0,0,1,0,0,0,1,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN }
    };

    const ESCX_BYTE_T children[2] = { 2,3 };

    /*
     * Both transitions are external transitions whose generated domains exit
     * the parallel state and both active atomic descendants. Their exit sets
     * therefore intersect, so the transitions genuinely conflict.
     *
     * Exit order is deepest-first and reverse document order: 3, 2, 1.
     * Entry order is ancestor-first and document order: 1, 2, 3.
     */
    const ESCX_StatePlan plans[2] =
    {
        { 0,3 },
        { 3,3 }
    };

    const ESCX_BYTE_T plan_states[6] =
    {
        3,2,1,
        1,2,3
    };

    const ESCX_Transition transitions[2] =
    {
        { 0,1,A,ESCX_TRANSITION_EXTERNAL,2,1,0 },
        { 0,1,B,ESCX_TRANSITION_EXTERNAL,3,1,1 }
    };

    const ESCX_SelectorFunction selectors[2] = { SA,SB };
    const ESCX_UINT16_T bases[4] = { 0,0,0,1 };
    const ESCX_BYTE_T owners[3] = { 0,2,3 };
    const ESCX_UINT16_T slots[3] = { ESCX_NO_SELECTOR,0,1 };

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
        4,
        2,
        2,
        2,
        6,
        2,
        3,
        1,
        2
    };

    /*
     * Two rows, one byte each. Each transition conflicts with itself and with
     * the other transition because both generated exit sets are { 3, 2, 1 }.
     */
    const ESCX_BYTE_T conflict_bits[2] = { 0x03U,0x03U };
    const ESCX_ConflictMap map = { conflict_bits,2,1 };
}

TEST(ConflictBitmap, UsesGeneratedPairConflictLookup)
{
    ESCX_StateMachine m = {};
    ESCX_BYTE_T active[1] = {};
    Data data = { 0,0 };

    ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);

    ASSERT_EQ(
        ESCX_STATUS_PROCESSED,
        ESCX_StateMachineConfigureConflictMap(&m, &map));
    ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
    EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0));

    /* Equal-depth conflicting transitions are resolved in document order. */
    EXPECT_EQ(1, data.a);
    EXPECT_EQ(0, data.b);
}

TEST(ConflictBitmap, RejectsWrongTransitionCount)
{
    ESCX_StateMachine m = {};
    ESCX_BYTE_T active[1] = {};
    Data data = { 0,0 };
    const ESCX_ConflictMap invalid = { conflict_bits,1,1 };

    ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0);

    EXPECT_EQ(
        ESCX_STATUS_INVALID_ARGUMENT,
        ESCX_StateMachineConfigureConflictMap(&m, &invalid));
}
