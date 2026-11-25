#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	const ESCX_StateDescriptor states[5] = 
	{ { 0 },{ 0,0,0,0,0,0,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,3,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,2,0,0,0,2,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,2,0,0,0,2,3,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; 
	const ESCX_SelectorFunction selectors[1] = { 0 };
	const ESCX_UINT16_T bases[5] = { 0 }; 
	const ESCX_BYTE_T owners[1] = { 0 };
	const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	void Init(ESCX_StateMachine* m, const ESCX_StateMachineDefinition* d, ESCX_BYTE_T* b)
	{
		ESCX_StateMachineInit(m, d, 0, b, 1, 0, 0, 0);
	}
}
TEST(TransitionDomainValidation, AcceptsExternalAndInternalGeneratedDomains)
{
	const ESCX_StatePlan plans[4] = { { 0,3 },{ 3,3 },{ 6,1 },{ 7,1 } }; 
	const ESCX_BYTE_T ps[8] = { 3,2,1,1,2,4,3,4 };
	const ESCX_Transition tr[2] = { { 0,1,0,ESCX_TRANSITION_EXTERNAL,3,2,0 },{ 2,3,0,ESCX_TRANSITION_INTERNAL,2,1,1 } };
	const ESCX_StateMachineDefinition d = { states,children,tr,plans,ps,selectors,bases,owners,slots,5,1,2,4,8,1,1,1,2 }; ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; Init(&m, &d, b); EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineValidateTransitionDomains(&m));
}
TEST(TransitionDomainValidation, RejectsExternalPlanThatDoesNotExitSourceDomain)
{
	const ESCX_StatePlan plans[2] = { { 0,1 },{ 1,1 } }; const ESCX_BYTE_T ps[2] = { 4,4 };
	const ESCX_Transition tr[1] = { { 0,1,0,ESCX_TRANSITION_EXTERNAL,3,2,0 } };
	const ESCX_StateMachineDefinition d = { states,children,tr,plans,ps,selectors,bases,owners,slots,5,1,1,2,2,1,1,1,2 }; ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; Init(&m, &d, b); EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineValidateTransitionDomains(&m));
}
TEST(TransitionDomainValidation, RejectsInternalPlanThatExitsCompoundSource)
{
	const ESCX_StatePlan plans[2] = { { 0,1 },{ 1,1 } }; const ESCX_BYTE_T ps[2] = { 2,4 }; 
	const ESCX_Transition tr[1] = { { 0,1,0,ESCX_TRANSITION_INTERNAL,2,1,0 } };
	const ESCX_StateMachineDefinition d = { states,children,tr,plans,ps,selectors,bases,owners,slots,5,1,1,2,2,1,1,1,2 }; ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; Init(&m, &d, b); EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineValidateTransitionDomains(&m));
}
TEST(TransitionDomainValidation, RejectsIncorrectPlanOrderAndTargetlessPlans)
{
	const ESCX_StatePlan plans[2] = { { 0,2 },{ 2,1 } }; const ESCX_BYTE_T ps[3] = { 1,3,4 }; 
	const ESCX_Transition tr[1] = { { 0,1,0,ESCX_TRANSITION_EXTERNAL,3,2,0 } };
	const ESCX_StateMachineDefinition d = { states,children,tr,plans,ps,selectors,bases,owners,slots,5,1,1,2,3,1,1,1,2 }; ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; Init(&m, &d, b); EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineValidateTransitionDomains(&m));
	ESCX_Transition targetless = { 0,1,0,ESCX_TRANSITION_TARGETLESS,ESCX_NO_STATE,0,0 }; 
	ESCX_StateMachineDefinition td = d; td.transitions = &targetless; 
	Init(&m, &td, b); 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineValidateTransitionDomains(&m));
}
