#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
    struct Data
    {
        ESCX_Event transported;
        ESCX_Event action_event;
        int transports;
        int actions;
    };
    struct Timer
    {
        ESCX_TimerId next;
    };
    ESCX_TimerId StartTimer(ESCX_StateMachine*, unsigned long, void* context)
    {
        Timer* timer = static_cast<Timer*>(context);
        return timer->next++;
    }
    int Transport(
        ESCX_StateMachine* machine,
        const ESCX_SendTargetDescriptor*,
        const ESCX_Event* event,
        void*)
    {
        Data* data = static_cast<Data*>(machine->instance);
        data->transported = *event;
        ++data->transports;
        return ESCX_STATUS_PROCESSED;
    }
    void Observe(ESCX_StateMachine* machine, void*)
    {
        Data* data = static_cast<Data*>(machine->instance);
        data->action_event = *ESCX_StateMachineGetCurrentEvent(machine);
        ++data->actions;
    }
    ESCX_UINT16_T Select(ESCX_StateMachine*, void*)
    {
        return 0;
    }
    const ESCX_StateDescriptor states[2] =
    {
        { 0 },
        { 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN }
    };
    const ESCX_BYTE_T children[1] = { 0 };
    const ESCX_StatePlan plans[1] = { { 0,0 } };
    const ESCX_BYTE_T plan_states[1] = { 0 };
    const ESCX_Transition transitions[1] =
    {
        { ESCX_NO_PLAN,ESCX_NO_PLAN,Observe,ESCX_TRANSITION_TARGETLESS,1,0,0 }
    };
    const ESCX_SelectorFunction selectors[1] = { Select };
    const ESCX_UINT16_T bases[2] = { 0,0 };
    const ESCX_BYTE_T owners[2] = { 0,1 };
    const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
    const ESCX_StateMachineDefinition def =
    {
        states,children,transitions,plans,plan_states,selectors,
        bases,owners,slots,2,1,1,1,1,1,2,1,2
    };
    void ExpectNeutral(const ESCX_StateMachine& machine)
    {
        const ESCX_Event* current = ESCX_StateMachineGetCurrentEvent(&machine);
        ASSERT_NE(nullptr,current);
        EXPECT_EQ(ESCX_NO_EVENT,current->name_id);
        EXPECT_EQ(ESCX_EVENT_ORIGIN_NONE,current->origin_type);
        EXPECT_EQ(ESCX_NO_INVOKE,current->invoke_id);
        EXPECT_EQ(ESCX_NO_SEND_ID,current->send_id);
        EXPECT_EQ(nullptr,current->data);
    }
}
TEST(DelayedSendSenderScope, CustomTransportDoesNotBindSenderCurrentEvent)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active[1] = {};
    Data data = {};
    Timer timer = { 40 };
    int payload = 12;
    const ESCX_TargetedSendRequest request =
    {
        { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_SCXML,3,81,&payload },
        ESCX_SEND_TARGET_CUSTOM(9,7),
        5
    };
    ESCX_StateMachineInit(&machine,&def,&data,active,1,0,0,0);
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineConfigureIOProcessor(&machine,Transport,0));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineConfigureTimers(&machine,StartTimer,0,&timer));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&machine));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineSendTargetedRequest(&machine,&request));
    ExpectNeutral(machine);
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineTimerExpired(&machine,40));
    ASSERT_EQ(1,data.transports);
    EXPECT_EQ(1,data.transported.name_id);
    EXPECT_EQ(ESCX_EVENT_ORIGIN_SCXML,data.transported.origin_type);
    EXPECT_EQ(3,data.transported.invoke_id);
    EXPECT_EQ(81,data.transported.send_id);
    EXPECT_EQ(&payload,data.transported.data);
    ExpectNeutral(machine);
}
TEST(DelayedSendSenderScope, CurrentSessionDeliveryBindsOnlyDuringMacrostepThenReturnsNeutral)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active[1] = {};
    Data data = {};
    Timer timer = { 60 };
    int payload = 21;
    const ESCX_TargetedSendRequest request =
    {
        { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,92,&payload },
        ESCX_SEND_TARGET_SCXML_CURRENT,
        5
    };
    ESCX_StateMachineInit(&machine,&def,&data,active,1,0,0,0);
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineConfigureTimers(&machine,StartTimer,0,&timer));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&machine));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineSendTargetedRequest(&machine,&request));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineTimerExpired(&machine,60));
    ASSERT_EQ(1,data.actions);
    EXPECT_EQ(1,data.action_event.name_id);
    EXPECT_EQ(ESCX_EVENT_ORIGIN_SCXML,data.action_event.origin_type);
    EXPECT_EQ(92,data.action_event.send_id);
    EXPECT_EQ(&payload,data.action_event.data);
    ExpectNeutral(machine);
}
TEST(DelayedSendSenderScope, ExpiryDuringMacrostepPreservesInterruptedEvent)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active[1] = {};
    Data data = {};
    Timer timer = { 80 };
    int outgoing_payload = 5;
    int driving_payload = 9;
    const ESCX_TargetedSendRequest request =
    {
        { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,37,&outgoing_payload },
        ESCX_SEND_TARGET_CUSTOM(7,3),
        4
    };
    const ESCX_Event driving =
    {
        1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,ESCX_NO_INVOKE,44,&driving_payload
    };
    ESCX_StateMachineInit(&machine,&def,&data,active,1,0,0,0);
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineConfigureIOProcessor(&machine,Transport,0));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineConfigureTimers(&machine,StartTimer,0,&timer));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&machine));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineSendTargetedRequest(&machine,&request));
    machine.processing = ESCX_TRUE;
    machine.current_event = driving.name_id;
    machine.current_event_descriptor = driving;
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineTimerExpired(&machine,80));
    const ESCX_Event* current = ESCX_StateMachineGetCurrentEvent(&machine);
    ASSERT_NE(nullptr,current);
    EXPECT_EQ(1,current->name_id);
    EXPECT_EQ(ESCX_EVENT_ORIGIN_PLATFORM,current->origin_type);
    EXPECT_EQ(44,current->send_id);
    EXPECT_EQ(&driving_payload,current->data);
    machine.processing = ESCX_FALSE;
}
