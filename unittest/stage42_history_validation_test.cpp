#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	const ESCX_StateDescriptor states[9] = { { 0 },{ 0,0,0,0,0,3,2,0,0,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,3,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,2,0,0,0,2,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,5,1,3,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,4,0,0,0,2,4,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,ESCX_NO_STATE,1,5,ESCX_STATE_HISTORY_DEEP,ESCX_NO_SELECTOR,ESCX_NO_EVENT,0,0 },{ 0,0,0,1,0,0,ESCX_NO_STATE,1,6,ESCX_STATE_HISTORY_SHALLOW,ESCX_NO_SELECTOR,ESCX_NO_EVENT,1,0 },{ 0,0,0,0,0,0,0,0,7,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[3] = { 2,4,6 }; const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } }; const ESCX_StatePlan plans[1] = { { 0,1 } }; const ESCX_BYTE_T plan_states[1] = { 3 }; const ESCX_SelectorFunction selectors[1] = { 0 }; const ESCX_UINT16_T bases[9] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR }; const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,9,3,1,1,1,1,1,1,2 };
	void Init(ESCX_StateMachine* m, ESCX_HistoryStorage* h, ESCX_BYTE_T* a)
	{
		ESCX_StateMachineInit(m, &def, 0, a, 2, h, 0, 0);
	}
}
TEST(HistoryValidation, AcceptsOrderedDeepAndDirectShallowRanges)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T a[2] = {}, v[2] = {}, b[4] = {}; 
	ESCX_HistoryStorage h = { v,b,2,2 }; const ESCX_UINT16_T f[2] = { 0,4 };
	const ESCX_BYTE_T c[2] = { 4,2 }; const ESCX_BYTE_T st[6] = { 2,4,3,5,2,4 }; 
	const ESCX_HistoryMap map = { f,c,st,6,2 }; Init(&m, &h, a); EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureHistoryMap(&m, &map));
}
TEST(HistoryValidation, RejectsDuplicateOutsideOwnerAndHistoryPseudoState)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T a[2] = {}, v[2] = {}, b[4] = {}; 
	ESCX_HistoryStorage h = { v,b,2,2 }; const ESCX_UINT16_T f[2] = { 0,2 };
	const ESCX_BYTE_T c[2] = { 2,2 }; const ESCX_BYTE_T duplicate[4] = { 2,2,2,4 };
	const ESCX_BYTE_T outside[4] = { 2,8,2,4 }; const ESCX_BYTE_T pseudo[4] = { 2,6,2,4 }; 
	ESCX_HistoryMap map = { f,c,duplicate,4,2 }; Init(&m, &h, a);
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureHistoryMap(&m, &map)); 
	map.states = outside; 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureHistoryMap(&m, &map)); 
	map.states = pseudo; EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureHistoryMap(&m, &map));
}
TEST(HistoryValidation, RejectsDeepChildBeforeAncestorAndShallowGrandchild)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T a[2] = {}, v[2] = {}, b[4] = {}; 
	ESCX_HistoryStorage h = { v,b,2,2 }; const ESCX_UINT16_T f[2] = { 0,2 }; 
	const ESCX_BYTE_T c[2] = { 2,2 }; const ESCX_BYTE_T wrong_order[4] = { 3,2,2,4 }; 
	const ESCX_BYTE_T shallow_grandchild[4] = { 2,4,3,4 }; 
	ESCX_HistoryMap map = { f,c,wrong_order,4,2 }; Init(&m, &h, a); 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureHistoryMap(&m, &map)); 
	map.states = shallow_grandchild; 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureHistoryMap(&m, &map));
}
