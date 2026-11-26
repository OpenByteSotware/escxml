#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		ESCX_SystemContext seen; int count;
	};
	void Observe(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); d->seen = *ESCX_StateMachineGetSystemContext(m); ++d->count;
	}
	int Select(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[2] = { { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; const ESCX_StatePlan plans[1] = { { 0,0 } }; const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,Observe,ESCX_TRANSITION_TARGETLESS,1,0,0 } };
	const ESCX_SelectorFunction selectors[1] = { Select }; const ESCX_UINT16_T bases[2] = { 0,0 }; const ESCX_BYTE_T owners[2] = { 0,1 }; const ESCX_UINT16_T slots[2] = { ESCX_NO_SELECTOR,0 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,2,1,2 };
}
TEST(SystemContext, GeneratedContentReadsConfiguredSystemVariables)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; int io = 1, platform = 2; Data data = { { 0,0,0,0 },0 };
	ESCX_SystemContext context = { 17,29,&io,&platform }; 
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureSystemContext(&m, &context));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0));
	ASSERT_EQ(1, data.count); EXPECT_EQ(17, data.seen.session_id); 
	EXPECT_EQ(29, data.seen.machine_name_id); EXPECT_EQ(&io, data.seen.io_processors);
	EXPECT_EQ(&platform, data.seen.platform_data);
}
TEST(SystemContext, IsImmutableWhileRunningAndSurvivesRestart)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T bits[1] = {}; Data data = { { 0,0,0,0 },0 }; ESCX_SystemContext first = { 7,8,0,0 }, second = { 9,10,0,0 };
	ESCX_StateMachineInit(&m, &def, &data, bits, 1, 0, 0, 0);
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureSystemContext(&m, &first));
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m));
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureSystemContext(&m, &second));
	ESCX_StateMachineStop(&m); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	const ESCX_SystemContext* current = ESCX_StateMachineGetSystemContext(&m);
	ASSERT_NE(nullptr, current); EXPECT_EQ(7, current->session_id);
	EXPECT_EQ(8, current->machine_name_id);
}
