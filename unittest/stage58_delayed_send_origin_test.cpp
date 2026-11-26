#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
    struct Data
    {
        ESCX_Event seen;
        int calls;
    };
    struct Timer
    {
        ESCX_TimerId next;
    };
    ESCX_TimerId StartTimer(ESCX_StateMachine*, unsigned long, ESCX_DelayedSendToken, void* context)
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
        data->seen = *event;
        ++data->calls;
        return ESCX_STATUS_PROCESSED;
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
        { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,1,0,0 }
    };
    const ESCX_SelectorFunction selectors[1] = { 0 };
    const ESCX_UINT16_T bases[2] = { 0,0 };
    const ESCX_BYTE_T owners[1] = { 0 };
    const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
    const ESCX_StateMachineDefinition def =
    {
        states,children,transitions,plans,plan_states,selectors,
        bases,owners,slots,2,1,1,1,1,1,1,1,2
    };
}
TEST(DelayedSendOrigin, TargetedDeliveryUsesSCXMLOriginNotTimerOrigin)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active[1] = {};
    Data data = {};
    Timer timer = { 30 };
    int payload = 8;
    const ESCX_TargetedSendRequest request =
    {
        { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,4,91,&payload },
        ESCX_SEND_TARGET_CUSTOM(9,7),
        5
    };
    ESCX_StateMachineInit(&machine,&def,&data,active,1,0,0,0);
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineConfigureIOProcessor(&machine,Transport,0));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineConfigureTimers(&machine,StartTimer,0,&timer));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&machine));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineSendTargetedRequest(&machine,&request));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineTimerExpired(&machine, ESCX_DelayedSendToken{0U,machine.delayed_sends[0].generation}, 30));
    ASSERT_EQ(1,data.calls);
    EXPECT_EQ(ESCX_EVENT_TYPE_EXTERNAL,data.seen.type);
    EXPECT_EQ(ESCX_EVENT_ORIGIN_SCXML,data.seen.origin_type);
    EXPECT_EQ(4,data.seen.invoke_id);
    EXPECT_EQ(91,data.seen.send_id);
    EXPECT_EQ(&payload,data.seen.data);
}
TEST(DelayedSendOrigin, StoredDescriptorIsNormalizedBeforeTimerExpiry)
{
    ESCX_StateMachine machine = {};
    ESCX_BYTE_T active[1] = {};
    Data data = {};
    Timer timer = { 50 };
    const ESCX_SendRequest request =
    {
        1,ESCX_SEND_TARGET_SELF,0,72,10,0
    };
    ESCX_StateMachineInit(&machine,&def,&data,active,1,0,0,0);
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineConfigureTimers(&machine,StartTimer,0,&timer));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&machine));
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineSendRequest(&machine,&request));
    EXPECT_EQ(ESCX_EVENT_ORIGIN_SCXML,machine.delayed_sends[0].origin_type);
    EXPECT_EQ(ESCX_EVENT_TYPE_EXTERNAL,machine.delayed_sends[0].event_type);
    EXPECT_EQ(72,machine.delayed_sends[0].send_id);
}
