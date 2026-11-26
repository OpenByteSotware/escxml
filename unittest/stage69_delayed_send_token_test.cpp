#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Timer
	{
		ESCX_TimerId id; ESCX_DelayedSendToken token; int starts;
	};
	ESCX_TimerId Start(ESCX_StateMachine*, unsigned long, ESCX_DelayedSendToken token, void* context)
	{
		Timer* timer = static_cast<Timer*>(context); timer->token = token; ++timer->starts; return timer->id;
	}
	ESCX_BOOL_T Cancel(ESCX_StateMachine*, ESCX_TimerId, void*)
	{
		return ESCX_TRUE;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } }; const ESCX_SelectorFunction selectors[1] = { 0 };
	const ESCX_UINT16_T bases[2] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,1,1,2 };
}
TEST(DelayedSendToken, RejectsLateExpiryAfterCancelAndSlotReuse)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; Timer timer = { 41,{ 0,0 },0 };
	const ESCX_SendRequest first = { 1,ESCX_SEND_TARGET_SELF,0,10,5,0 };
	const ESCX_SendRequest second = { 1,ESCX_SEND_TARGET_SELF,0,11,5,0 };
	ESCX_StateMachineInit(&m, &def, 0, active, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureTimers(&m, Start, Cancel, &timer));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendRequest(&m, &first));
	const ESCX_DelayedSendToken stale = timer.token;
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineCancelSend(&m, 10));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendRequest(&m, &second));
	const ESCX_DelayedSendToken current = timer.token;
	EXPECT_NE(stale.generation, current.generation);
	EXPECT_EQ(ESCX_STATUS_IGNORED, ESCX_StateMachineTimerExpired(&m, stale, 41));
	EXPECT_TRUE(m.delayed_sends[current.slot].active);
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineTimerExpired(&m, current, 41));
	EXPECT_FALSE(m.delayed_sends[current.slot].active);
}
