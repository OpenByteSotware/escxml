#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	const ESCX_StateDescriptor states[6] = 
	{ { 0 },{ 0,0,0,0,0,0,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,3,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,2,0,0,0,2,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,5,1,3,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,4,0,0,0,2,4,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; 
	const ESCX_SelectorFunction selectors[1] = { 0 };
	const ESCX_UINT16_T bases[6] = { 0 }; 
	const ESCX_BYTE_T owners[1] = { 0 }; 
	const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StatePlan plans[2] = { { 0,2 },{ 2,2 } }; 
	const ESCX_BYTE_T plan_states[4] = { 3,2,4,5 };
	void Init(ESCX_StateMachine* m, 
		const ESCX_Transition* t, ESCX_BYTE_T* b)
	{
		static ESCX_StateMachineDefinition d; d = { states,children,t,plans,plan_states,selectors,bases,owners,slots,6,1,1,2,4,1,1,1,2 }; ESCX_StateMachineInit(m, &d, 0, b, 1, 0, 0, 0);
	}
}
TEST(TransitionDomainMap, AcceptsExactExternalLCCA)
{
	ESCX_Transition t = { 0,1,0,ESCX_TRANSITION_EXTERNAL,3,2,0 }; 
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; 
	const ESCX_BYTE_T targets[1] = { 5 }; 
	const ESCX_TransitionDomainDescriptor td[1] = { { 1,ESCX_TRANSITION_EXTERNAL,0,1 } }; 
	const ESCX_TransitionDomainMap map = { td,targets,1,1 }; Init(&m, &t, b); EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureTransitionDomainMap(&m, &map)); EXPECT_EQ(&map, m.transition_domain_map);
}
TEST(TransitionDomainMap, NormalizesInternalFromAtomicSourceToExternal)
{
	ESCX_Transition t = { 0,1,0,ESCX_TRANSITION_INTERNAL,3,2,0 }; 
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; const ESCX_BYTE_T targets[1] = { 5 }; 
	const ESCX_TransitionDomainDescriptor td[1] = { { 1,ESCX_TRANSITION_EXTERNAL,0,1 } }; 
	const ESCX_TransitionDomainMap map = { td,targets,1,1 }; Init(&m, &t, b); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineValidateTransitionDomains(&m));
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureTransitionDomainMap(&m, &map));
}
TEST(TransitionDomainMap, RejectsWrongLCCAOrInternalEligibility)
{
	ESCX_Transition t = { 0,1,0,ESCX_TRANSITION_INTERNAL,3,2,0 }; 
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; 
	const ESCX_BYTE_T targets[1] = { 5 }; 
	const ESCX_TransitionDomainDescriptor wrong[1] = { { 2,ESCX_TRANSITION_INTERNAL,0,1 } };
	const ESCX_TransitionDomainMap map = { wrong,targets,1,1 }; 
	Init(&m, &t, b); 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureTransitionDomainMap(&m, &map));
	EXPECT_EQ(nullptr, m.transition_domain_map);
}
