#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Timer
	{
		ESCX_TimerId id; ESCX_DelayedSendToken token;
	};
	struct Data
	{
		int actions; ESCX_Event seen;
	};
	ESCX_TimerId Start(ESCX_StateMachine*, unsigned long, ESCX_DelayedSendToken token, void* context)
	{
		Timer* timer = static_cast<Timer*>(context); timer->token = token; return timer->id;
	}
	void Observe(ESCX_StateMachine* machine, void*)
	{
		Data* data = static_cast<Data*>(machine->instance); ++data->actions; data->seen = *ESCX_StateMachineGetCurrentEvent(machine);
	}
	int Select(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id)
	{
		*transition_id = 0U; return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[2] = 
	{ { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; 
	const ESCX_StatePlan plans[1] = { { 0,0 } }; 
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = 
	{ { ESCX_NO_PLAN,ESCX_NO_PLAN,Observe,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { Select }; const ESCX_UINT16_T bases[2] = { 0,0 }; 
	const ESCX_BYTE_T owners[2] = { 0,1 }; 
	const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = 
	{ states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,2,1,2 };
}
TEST(TransactionalDelayedExpiry, FullExternalQueueRetainsExactExpiryForRetry)
{
	ESCX_StateMachine machine = {}; 
	ESCX_BYTE_T active[1] = {};
	Data data = {}; Timer timer = { 47,{ 0,0 } }; 
	int payload = 9;
	const ESCX_TargetedSendRequest request = 
		{ { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_SCXML,ESCX_NO_INVOKE,71,&payload },ESCX_SEND_TARGET_SCXML_CURRENT,5 };
	const ESCX_Event filler = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,ESCX_NO_INVOKE,0,0 };
	ESCX_StateMachineInit(&machine, &def, &data, active, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureTimers(&machine, Start, 0, &timer)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&machine)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendTargetedRequest(&machine, &request));
	machine.processing = ESCX_TRUE; 
	for (ESCX_BYTE_T i = 0U; i < ESCX_QUEUE_SIZE; ++i) 
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendEvent(&machine, &filler));
	EXPECT_EQ(ESCX_STATUS_QUEUE_FULL, ESCX_StateMachineTimerExpired(&machine, timer.token, timer.id));
	EXPECT_TRUE(machine.delayed_sends[timer.token.slot].active); 
	EXPECT_EQ(timer.token.generation, machine.delayed_sends[timer.token.slot].generation);
	EXPECT_EQ(timer.id, machine.delayed_sends[timer.token.slot].timer_id); 
	
	EXPECT_EQ(0, data.actions);
	ESCX_EventQueueInit(&machine.external_queue); machine.processing = ESCX_FALSE; 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineTimerExpired(&machine, timer.token, timer.id)); 
	EXPECT_FALSE(machine.delayed_sends[timer.token.slot].active); 
	EXPECT_EQ(1, data.actions); 
	EXPECT_EQ(71, data.seen.send_id); 
	EXPECT_EQ(&payload, data.seen.data); 
	EXPECT_EQ(ESCX_STATUS_IGNORED, ESCX_StateMachineTimerExpired(&machine, timer.token, timer.id));
}
