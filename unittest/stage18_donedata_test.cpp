#include <gtest/gtest.h>

extern "C"
{
#include "StateMachine.h"
}

namespace
{
    struct DoneData
    {
        int value;
        void* seen;
        ESCX_BYTE_T invoke;
    };

    int FinalPayload(
        ESCX_StateMachine* machine,
        void** output)
    {
        DoneData* data =
            static_cast<DoneData*>(machine->instance);

        *output = &data->value;

        return ESCX_STATUS_PROCESSED;
    }

    ESCX_UINT16_T SelectDone(
        ESCX_StateMachine* machine,
        void*)
    {
        const ESCX_Event* event =
            ESCX_StateMachineGetCurrentEvent(machine);

        DoneData* data =
            static_cast<DoneData*>(machine->instance);

        data->seen = event->data;
        data->invoke = event->invoke_id;

        return ESCX_NO_TRANSITION;
    }

    /*
     * State hierarchy:
     *
     * state 3: top-level compound
     *   state 1: nested compound
     *     state 2: final
     *
     * Completion of state 1 produces done.state event 3.
     * State 3 is the document-level state, so completion of state 1 does
     * not immediately terminate the state-machine session.
     */
    const ESCX_StateDescriptor states[4] = {
        /* Reserved state zero. */
        {
            0,
            0,
            0,
            ESCX_NO_STATE,
            0,
            0,
            ESCX_NO_STATE,
            0,
            0,
            ESCX_STATE_ATOMIC,
            ESCX_NO_SELECTOR,
            ESCX_NO_EVENT,
            ESCX_NO_HISTORY_SLOT,
            ESCX_NO_PLAN,
            0,
            0,
            0,
            0,
            0
        },

        /*
         * Nested compound state.
         *
         * Its final child is state 2.
         * Completion generates event 3.
         */
        {
            0,
            0,
            0,
            3,
            0,
            0,
            2,
            1,
            1,
            ESCX_STATE_COMPOUND,
            ESCX_NO_SELECTOR,
            3,
            ESCX_NO_HISTORY_SLOT,
            ESCX_NO_PLAN,
            0,
            0,
            0,
            0,
            0
        },

        /*
         * Final state whose donedata callback returns &DoneData::value.
         */
        {
            0,
            0,
            0,
            1,
            0,
            0,
            ESCX_NO_STATE,
            2,
            2,
            ESCX_STATE_FINAL,
            ESCX_NO_SELECTOR,
            ESCX_NO_EVENT,
            ESCX_NO_HISTORY_SLOT,
            ESCX_NO_PLAN,
            0,
            0,
            0,
            0,
            FinalPayload
        },

        /*
         * Top-level compound state.
         *
         * Its initial child is nested compound state 1.
         */
        {
            0,
            0,
            0,
            ESCX_NO_STATE,
            0,
            0,
            1,
            0,
            0,
            ESCX_STATE_COMPOUND,
            ESCX_NO_SELECTOR,
            ESCX_NO_EVENT,
            ESCX_NO_HISTORY_SLOT,
            ESCX_NO_PLAN,
            0,
            0,
            0,
            0,
            0
        }
    };

    const ESCX_BYTE_T children[1] = {
        0
    };

    const ESCX_StatePlan plans[1] = {
        { 0, 0 }
    };

    const ESCX_BYTE_T plan_states[1] = {
        0
    };

    /*
     * Targetless transition used only to observe the current completion
     * event descriptor. The selector records _event.data and invoke ID.
     */
    const ESCX_Transition transitions[1] = {
        {
            ESCX_NO_PLAN,
            ESCX_NO_PLAN,
            0,
            ESCX_TRANSITION_TARGETLESS,
            2,
            2,
            0,
            0
        }
    };

    const ESCX_SelectorFunction selectors[1] = {
        SelectDone
    };

    const ESCX_UINT16_T bases[4] = {
        0,
        0,
        0,
        0
    };

    /*
     * Event 3 is handled by active final state 2.
     *
     * slot index = base[state 2] + event 3 = 3
     */
    const ESCX_BYTE_T owners[4] = {
        0,
        0,
        0,
        2
    };

    const ESCX_UINT16_T slots[4] = {
        ESCX_NO_SELECTOR,
        ESCX_NO_SELECTOR,
        ESCX_NO_SELECTOR,
        0
    };

    const ESCX_StateMachineDefinition definition = {
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
        1,
        1,
        1,
        1,
        1,
        4,
        3, /* Top-level initial state. */
        4
    };

    int InvokeStart(
        ESCX_StateMachine*,
        void*,
        ESCX_InvokeToken)
    {
        return ESCX_STATUS_PROCESSED;
    }

    int InvokePayload(
        ESCX_StateMachine* machine,
        void** output)
    {
        DoneData* data =
            static_cast<DoneData*>(machine->instance);

        *output = &data->value;

        return ESCX_STATUS_PROCESSED;
    }
}

TEST(DoneData, DoneStateCarriesFinalPayload)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active_bits[1] = {};

    DoneData data = {
        42,
        nullptr,
        ESCX_NO_INVOKE
    };

    ESCX_StateMachineInit(
        &machine,
        &definition,
        &data,
        active_bits,
        static_cast<ESCX_BYTE_T>(sizeof(active_bits)),
        nullptr,
        nullptr,
        nullptr
    );

    ASSERT_EQ(
        ESCX_STATUS_PROCESSED,
        ESCX_StateMachineStart(&machine)
    );

    EXPECT_EQ(&data.value, data.seen);
    EXPECT_EQ(ESCX_NO_INVOKE, data.invoke);
}

TEST(DoneData, DoneInvokeCarriesPayloadAndInvokeId)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active_bits[1] = {};
    ESCX_InvokeInstance invoke_instances[1] = {};

    DoneData data = {
        77,
        nullptr,
        ESCX_NO_INVOKE
    };

    /*
     * The invocation is owned by top-level state 3.
     * Entering state 3 automatically starts it.
     */
    const ESCX_InvokeDescriptor invoke = {
        3,
        3,
        ESCX_FALSE,
        nullptr,
        InvokeStart,
        nullptr,
        nullptr,
        nullptr,
        InvokePayload
    };

    ESCX_StateMachineInit(
        &machine,
        &definition,
        &data,
        active_bits,
        static_cast<ESCX_BYTE_T>(sizeof(active_bits)),
        nullptr,
        nullptr,
        nullptr
    );

    ASSERT_EQ(
        ESCX_STATUS_PROCESSED,
        ESCX_StateMachineConfigureInvokes(
            &machine,
            &invoke,
            invoke_instances,
            1
        )
    );

    ASSERT_EQ(
        ESCX_STATUS_PROCESSED,
        ESCX_StateMachineStart(&machine)
    );

    ASSERT_EQ(ESCX_TRUE, invoke_instances[0].active);

    /*
     * Starting the machine already generated done.state for the nested
     * compound. Clear the observation fields so this section verifies
     * only the later done.invoke event.
     */
    data.seen = nullptr;
    data.invoke = ESCX_NO_INVOKE;

    ASSERT_EQ(
        ESCX_STATUS_PROCESSED,
        ESCX_StateMachineCompleteInvoke(
            &machine,
            ESCX_InvokeToken{ 0U,invoke_instances[0].generation },
            nullptr
        )
    );

    EXPECT_EQ(&data.value, data.seen);
    EXPECT_EQ(0, data.invoke);
    EXPECT_EQ(ESCX_FALSE, invoke_instances[0].active);
}