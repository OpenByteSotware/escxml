#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
const ESCX_BYTE_T children[1] = { 0 };
const ESCX_StatePlan plans[1] = { { 0,0 } };
const ESCX_BYTE_T plan_states[1] = { 0 };
const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } };
const ESCX_SelectorFunction selectors[1] = { 0 };
const ESCX_UINT16_T bases[2] = { 0,0 };
const ESCX_BYTE_T owners[1] = { 0 };
const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,1,1,4 };
void Init(ESCX_StateMachine* machine, ESCX_BYTE_T* active)
{
    ESCX_StateMachineInit(machine,&def,0,active,1,0,0,0);
}
}
TEST(EventHierarchyValidation, AcceptsAcyclicGeneratedPrefixForest)
{
    ESCX_StateMachine machine = {}; ESCX_BYTE_T active[1] = {};
    const ESCX_BYTE_T parents[4] = { ESCX_NO_EVENT,ESCX_NO_EVENT,1,2 };
    Init(&machine,active);
    EXPECT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineConfigureEventMatching(&machine,parents,4,ESCX_NO_EVENT));
    EXPECT_EQ(parents,machine.event_parent);
}
TEST(EventHierarchyValidation, RejectsSelfAndMultiEventCycles)
{
    ESCX_StateMachine machine = {}; ESCX_BYTE_T active[1] = {};
    const ESCX_BYTE_T self_cycle[4] = { ESCX_NO_EVENT,1,1,2 };
    const ESCX_BYTE_T multi_cycle[4] = { ESCX_NO_EVENT,2,3,1 };
    Init(&machine,active);
    EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT,ESCX_StateMachineConfigureEventMatching(&machine,self_cycle,4,ESCX_NO_EVENT));
    EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT,ESCX_StateMachineConfigureEventMatching(&machine,multi_cycle,4,ESCX_NO_EVENT));
    EXPECT_EQ(nullptr,machine.event_parent);
}
TEST(EventHierarchyValidation, RejectsMutationWhileRunning)
{
    ESCX_StateMachine machine = {}; ESCX_BYTE_T active[1] = {};
    const ESCX_BYTE_T parents[4] = { ESCX_NO_EVENT,ESCX_NO_EVENT,1,2 };
    Init(&machine,active);
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&machine));
    EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT,ESCX_StateMachineConfigureEventMatching(&machine,parents,4,ESCX_NO_EVENT));
}
