#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
    struct Data
    {
        int phase;
        ESCX_Event event_action;
        ESCX_Event eventless_selector;
        ESCX_Event eventless_action;
    };
    int SelectExternal(ESCX_StateMachine* machine, void*, ESCX_UINT16_T* transition_id) {
        static_cast<Data*>(machine->instance)->phase = 1;
        *transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
    }
    int SelectEventless(ESCX_StateMachine* machine, void*, ESCX_UINT16_T* transition_id) {
        Data* data = static_cast<Data*>(machine->instance);
        if (data->phase != 1) { *transition_id = ESCX_NO_TRANSITION; return ESCX_STATUS_PROCESSED; }
        data->eventless_selector = *ESCX_StateMachineGetCurrentEvent(machine);
        *transition_id = (ESCX_UINT16_T)(1); return ESCX_STATUS_PROCESSED;
    }
    void ExternalAction(ESCX_StateMachine* machine, void*)
    {
        static_cast<Data*>(machine->instance)->event_action = *ESCX_StateMachineGetCurrentEvent(machine);
    }
    void EventlessAction(ESCX_StateMachine* machine, void*)
    {
        Data* data = static_cast<Data*>(machine->instance);
        data->eventless_action = *ESCX_StateMachineGetCurrentEvent(machine);
        data->phase = 2;
    }
    const ESCX_StateDescriptor states[2] =
    {
        { 0 },
        { 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,1,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN }
    };
    const ESCX_BYTE_T children[1] = { 0 };
    const ESCX_StatePlan plans[1] = { { 0,0 } };
    const ESCX_BYTE_T plan_states[1] = { 0 };
    const ESCX_Transition transitions[2] =
    {
        { ESCX_NO_PLAN,ESCX_NO_PLAN,ExternalAction,ESCX_TRANSITION_TARGETLESS,1,0,0 },
        { ESCX_NO_PLAN,ESCX_NO_PLAN,EventlessAction,ESCX_TRANSITION_TARGETLESS,1,0,1 }
    };
    const ESCX_SelectorFunction selectors[2] = { SelectExternal,SelectEventless };
    const ESCX_UINT16_T bases[2] = { 0,0 };
    const ESCX_BYTE_T owners[2] = { 0,1 };
    const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
    const ESCX_StateMachineDefinition def =
    {
        states,children,transitions,plans,plan_states,selectors,
        bases,owners,slots,2,1,2,1,1,2,2,1,2
    };
}
TEST(EventLifetime, DrivingEventRemainsBoundThroughEventlessStabilizationAndAfterMacrostep)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active[1] = {};
    Data data = {};
    int payload = 31;
    const ESCX_Event event =
    {
        1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,
        ESCX_NO_INVOKE,67,&payload
    };
    ESCX_StateMachineInit(&machine,&def,&data,active,1,0,0,0);
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&machine));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineSendEvent(&machine,&event));
    EXPECT_EQ(1,data.event_action.name_id);
    EXPECT_EQ(1,data.eventless_selector.name_id);
    EXPECT_EQ(1,data.eventless_action.name_id);
    EXPECT_EQ(67,data.eventless_selector.send_id);
    EXPECT_EQ(&payload,data.eventless_action.data);
    const ESCX_Event* current = ESCX_StateMachineGetCurrentEvent(&machine);
    ASSERT_NE(nullptr,current);
    EXPECT_EQ(1,current->name_id);
    EXPECT_EQ(ESCX_EVENT_ORIGIN_PLATFORM,current->origin_type);
    EXPECT_EQ(67,current->send_id);
    EXPECT_EQ(&payload,current->data);
}
TEST(EventLifetime, StopAndRestartResetCurrentEventToNeutral)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active[1] = {};
    Data data = {};
    ESCX_StateMachineInit(&machine,&def,&data,active,1,0,0,0);
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&machine));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineSend(&machine,1,0));
    ASSERT_EQ(1,ESCX_StateMachineGetCurrentEvent(&machine)->name_id);
    ESCX_StateMachineStop(&machine);
    EXPECT_EQ(ESCX_NO_EVENT,ESCX_StateMachineGetCurrentEvent(&machine)->name_id);
    data.phase = 2;
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&machine));
    EXPECT_EQ(ESCX_NO_EVENT,ESCX_StateMachineGetCurrentEvent(&machine)->name_id);
    EXPECT_EQ(ESCX_EVENT_ORIGIN_NONE,ESCX_StateMachineGetCurrentEvent(&machine)->origin_type);
}
