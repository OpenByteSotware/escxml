#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	const ESCX_StateDescriptor states[8] = {
	 { 0 },
	 { 0,0,0,0,0,0,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,1,0,2,ESCX_NO_STATE,1,1,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,2,2,1,4,2,2,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,3,0,0,ESCX_NO_STATE,3,3,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,2,3,1,6,2,4,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,5,0,0,ESCX_NO_STATE,3,5,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,1,0,0,ESCX_NO_STATE,1,6,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN }
	};
	const ESCX_BYTE_T children[4] = { 3,5,4,6 };
	const ESCX_SelectorFunction selectors[1] = { 0 }; const ESCX_UINT16_T bases[8] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_BYTE_T targets[1] = { 2 }; const ESCX_TransitionDomainDescriptor domains[1] = { { 1,ESCX_TRANSITION_EXTERNAL,0,1 } }; const ESCX_TransitionDomainMap map = { domains,targets,1,1 };
	void Init(ESCX_StateMachine* m, ESCX_BYTE_T* active, const ESCX_StatePlan* plans, const ESCX_BYTE_T* ps, ESCX_UINT16_T count)
	{
		static ESCX_Transition transition; static ESCX_StateMachineDefinition def;
		transition = ESCX_Transition{ 0,1,0,ESCX_TRANSITION_EXTERNAL,7,1,0 };
		def = ESCX_StateMachineDefinition{ states,children,&transition,plans,ps,selectors,bases,owners,slots,8,4,1,2,count,1,1,1,2 };
		ESCX_StateMachineInit(m, &def, 0, active, 1, 0, 0, 0);
	}
}
TEST(EntryConfigurationValidation, AcceptsCompleteParallelRegionInitialClosure)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; const ESCX_StatePlan plans[2] = { { 0,1 },{ 1,5 } }; const ESCX_BYTE_T ps[6] = { 7,2,3,5,4,6 }; Init(&m, active, plans, ps, 6);
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureTransitionDomainMap(&m, &map));
}
TEST(EntryConfigurationValidation, RejectsMissingParallelRegion)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; const ESCX_StatePlan plans[2] = { { 0,1 },{ 1,3 } }; const ESCX_BYTE_T ps[4] = { 7,2,3,4 }; Init(&m, active, plans, ps, 4);
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureTransitionDomainMap(&m, &map));
}
TEST(EntryConfigurationValidation, RejectsRegionWithoutCompoundInitialChild)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; const ESCX_StatePlan plans[2] = { { 0,1 },{ 1,4 } }; const ESCX_BYTE_T ps[5] = { 7,2,3,5,6 }; Init(&m, active, plans, ps, 5);
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureTransitionDomainMap(&m, &map));
}
