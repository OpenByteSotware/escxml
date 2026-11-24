#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int allow_b; int log[8]; int count;
	}; void Add(ESCX_StateMachine* m, int v)
	{
		Data* d = static_cast<Data*>(m->instance); d->log[d->count++] = v;
	}
	void ExitA(ESCX_StateMachine* m, void*)
	{
		Add(m, 1);
	} void ExitB(ESCX_StateMachine* m, void*)
	{
		Add(m, 2);
	} void ActionA(ESCX_StateMachine* m, void*)
	{
		Data* d = static_cast<Data*>(m->instance); Add(m, 3); d->allow_b = 0;
	} void ActionB(ESCX_StateMachine* m, void*)
	{
		Add(m, 4);
	} void EnterA(ESCX_StateMachine* m, void*)
	{
		Add(m, 5);
	} void EnterB(ESCX_StateMachine* m, void*)
	{
		Add(m, 6);
	}
	ESCX_UINT16_T SelectA(ESCX_StateMachine*, void*)
	{
		return 0;
	} ESCX_UINT16_T SelectB(ESCX_StateMachine* m, void*)
	{
		return static_cast<Data*>(m->instance)->allow_b ? 1 : ESCX_NO_TRANSITION;
	}
	const ESCX_StateDescriptor states[8] = { { 0 },{ 0,0,0,0,0,2,ESCX_NO_STATE,0,0,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,3,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,ExitA,0,2,0,0,0,2,2,ESCX_STATE_ATOMIC,0,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ EnterA,0,0,2,0,0,0,2,3,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,6,1,4,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,ExitB,0,5,0,0,0,2,5,ESCX_STATE_ATOMIC,1,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ EnterB,0,0,5,0,0,0,2,6,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[2] = { 2,5 }; const ESCX_StatePlan plans[4] = { { 0,1 },{ 1,1 },{ 2,1 },{ 3,1 } }; const ESCX_BYTE_T plan_states[4] = { 3,4,6,7 }; const ESCX_Transition transitions[2] = { { 0,1,ActionA,ESCX_TRANSITION_EXTERNAL,3,2,0 },{ 2,3,ActionB,ESCX_TRANSITION_EXTERNAL,6,2,1 } }; const ESCX_SelectorFunction selectors[2] = { SelectA,SelectB }; const ESCX_UINT16_T bases[8] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 }; const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR }; const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,8,2,2,4,4,2,1,1,1 };
	const ESCX_UINT16_T fast_bases[8] = { 0 }; const ESCX_BYTE_T fast_owners[1] = { 0 }; const ESCX_UINT16_T fast_slots[1] = { ESCX_NO_SELECTOR }; const ESCX_UINT16_T fast_eventless[8] = { ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,0,ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,1,ESCX_NO_SELECTOR }; const ESCX_FastDispatch fast = { fast_bases,fast_owners,fast_slots,1,fast_eventless,8 };
}
TEST(GlobalEventless, SelectsOrthogonalTransitionsBeforeActions)
{
	ESCX_StateMachine m = {}; 
	ESCX_BYTE_T active[1] = {}, exit_states[4] = {}, entry_states[4] = {}, exit_seen[1] = {}, entry_seen[1] = {};
	ESCX_MicrostepWorkspace w = { exit_states,4,0,entry_states,4,0,exit_seen,entry_seen,1 }; 
	Data data = { 1,{ 0 },0 }; ESCX_StateMachineInit(&m, &def, &data, active, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureFastDispatch(&m, &fast)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureWorkspace(&m, &w)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	ASSERT_EQ(6, data.count); EXPECT_EQ(2, data.log[0]); EXPECT_EQ(1, data.log[1]); 
	EXPECT_EQ(3, data.log[2]); EXPECT_EQ(4, data.log[3]); EXPECT_EQ(5, data.log[4]);
	EXPECT_EQ(6, data.log[5]); EXPECT_TRUE(ESCX_StateMachineIsInState(&m, 4));
	EXPECT_TRUE(ESCX_StateMachineIsInState(&m, 7));
}
