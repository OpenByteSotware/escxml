#include <gtest/gtest.h>

extern "C" {
#include "StateMachine.h"
}

namespace
{
    const ESCX_StateDescriptor states[6] =
    {
        { 0 },
        { 0,0,0,0,0,0,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
        { 0,0,0,1,0,0,3,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
        { 0,0,0,2,0,0,4,2,2,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
        { 0,0,0,3,0,0,0,3,3,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
        { 0,0,0,1,0,0,0,1,4,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN }
    };

    const ESCX_BYTE_T children[1] = { 0 };
    const ESCX_SelectorFunction selectors[1] = { 0 };
    const ESCX_UINT16_T bases[6] = { 0 };
    const ESCX_BYTE_T owners[1] = { 0 };
    const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };

    const ESCX_BYTE_T targets[1] = { 4 };

    const ESCX_TransitionDomainDescriptor domains[1] =
    {
        { 1,ESCX_TRANSITION_EXTERNAL,0,1 }
    };

    const ESCX_TransitionDomainMap map =
    {
        domains,
        targets,
        1,
        1
    };

    void Init(
        ESCX_StateMachine* machine,
        ESCX_BYTE_T* active,
        const ESCX_StatePlan* plans,
        const ESCX_BYTE_T* plan_states,
        ESCX_UINT16_T plan_state_count)
    {
        static ESCX_Transition transition;
        static ESCX_StateMachineDefinition definition;

        /*
         * Source state 5 and target state 4 are in different branches below
         * compound state 1. State 1 is therefore the external transition
         * domain. The complete entry path is 2, 3, 4.
         */
        transition = ESCX_Transition
        {
            0,
            1,
            0,
            ESCX_TRANSITION_EXTERNAL,
            5,
            1,
            0
        };

        definition = ESCX_StateMachineDefinition
        {
            states,
            children,
            &transition,
            plans,
            plan_states,
            selectors,
            bases,
            owners,
            slots,
            6,
            1,
            1,
            2,
            plan_state_count,
            1,
            1,
            1,
            2
        };

        ESCX_StateMachineInit(
            machine,
            &definition,
            0,
            active,
            1,
            0,
            0,
            0);
    }
}

TEST(
    TransitionEntryPlanValidation,
    AcceptsCompleteDomainToTargetPath)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active[1] = {};

    const ESCX_StatePlan plans[2] =
    {
        { 0,1 },
        { 1,3 }
    };

    const ESCX_BYTE_T plan_states[4] =
    {
        5,
        2,3,4
    };

    Init(&machine, active, plans, plan_states, 4);

    EXPECT_EQ(
        ESCX_STATUS_PROCESSED,
        ESCX_StateMachineConfigureTransitionDomainMap(
            &machine,
            &map));

    EXPECT_EQ(&map, machine.transition_domain_map);
}

TEST(
    TransitionEntryPlanValidation,
    RejectsMissingExplicitTarget)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active[1] = {};

    const ESCX_StatePlan plans[2] =
    {
        { 0,1 },
        { 1,2 }
    };

    const ESCX_BYTE_T plan_states[3] =
    {
        5,
        2,3
    };

    Init(&machine, active, plans, plan_states, 3);

    EXPECT_EQ(
        ESCX_STATUS_INVALID_ARGUMENT,
        ESCX_StateMachineConfigureTransitionDomainMap(
            &machine,
            &map));

    EXPECT_EQ(nullptr, machine.transition_domain_map);
}

TEST(
    TransitionEntryPlanValidation,
    RejectsMissingTargetAncestor)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active[1] = {};

    const ESCX_StatePlan plans[2] =
    {
        { 0,1 },
        { 1,2 }
    };

    const ESCX_BYTE_T plan_states[3] =
    {
        5,
        2,4
    };

    Init(&machine, active, plans, plan_states, 3);

    EXPECT_EQ(
        ESCX_STATUS_INVALID_ARGUMENT,
        ESCX_StateMachineConfigureTransitionDomainMap(
            &machine,
            &map));

    EXPECT_EQ(nullptr, machine.transition_domain_map);
}

TEST(
    TransitionEntryPlanValidation,
    RejectsDomainOrOutsideStateInEntryPlan)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active[1] = {};

    const ESCX_StatePlan plans[2] =
    {
        { 0,1 },
        { 1,4 }
    };

    const ESCX_BYTE_T plan_states[5] =
    {
        5,
        1,2,3,4
    };

    Init(&machine, active, plans, plan_states, 5);

    EXPECT_EQ(
        ESCX_STATUS_INVALID_ARGUMENT,
        ESCX_StateMachineConfigureTransitionDomainMap(
            &machine,
            &map));

    EXPECT_EQ(nullptr, machine.transition_domain_map);
}
