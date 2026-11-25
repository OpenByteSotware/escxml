#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		ESCX_Event seen; int count;
	};
	ESCX_UINT16_T NoEventless(ESCX_StateMachine*, void*)
	{
		return ESCX_NO_TRANSITION;
	}
	void Observe(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); d->seen = *ESCX_StateMachineGetCurrentEvent(m); ++d->count;
	}
	ESCX_UINT16_T SelectExternal(ESCX_StateMachine*, void*)
	{
		return 0;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,1,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; 
	const ESCX_StatePlan plans[1] = { { 0,0 } }; 
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,Observe,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[2] = { SelectExternal,NoEventless }; 
	const ESCX_UINT16_T bases[2] = { 0,0 };
	const ESCX_BYTE_T owners[2] = { 0,1 };
	const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,2,2,1,2 };
}
TEST(EventlessDescriptorRegression, StabilizationRetainsDrivingExternalEventInCurrentScope)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; 
	Data data = { { ESCX_NO_EVENT,0,0,ESCX_NO_INVOKE,0,0 },0 }; int payload = 42;
	ESCX_Event event = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,
		ESCX_NO_INVOKE,77,&payload };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendEvent(&m, &event)); 
	ASSERT_EQ(1, data.count);
	EXPECT_EQ(1, data.seen.name_id); 
	EXPECT_EQ(ESCX_EVENT_TYPE_EXTERNAL, data.seen.type);
	EXPECT_EQ(ESCX_EVENT_ORIGIN_PLATFORM, data.seen.origin_type);
	EXPECT_EQ(77, data.seen.send_id);
	EXPECT_EQ(&payload, data.seen.data);
	const ESCX_Event* current = ESCX_StateMachineGetCurrentEvent(&m); 
	ASSERT_NE(nullptr, current);
	EXPECT_EQ(1, current->name_id);
	EXPECT_EQ(ESCX_EVENT_TYPE_EXTERNAL, current->type);
	EXPECT_EQ(ESCX_EVENT_ORIGIN_PLATFORM, current->origin_type);
	EXPECT_EQ(77, current->send_id);
	EXPECT_EQ(&payload, current->data);
}
