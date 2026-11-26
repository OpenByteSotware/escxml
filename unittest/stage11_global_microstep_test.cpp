#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct LogData
	{
		int values[16]; int count;
	};
	void Log(ESCX_StateMachine* m, int v)
	{
		LogData* d = static_cast<LogData*>(m->instance); d->values[d->count++] = v;
	}
	void ExitA(ESCX_StateMachine* m, void*)
	{
		Log(m, 1);
	} void ExitB(ESCX_StateMachine* m, void*)
	{
		Log(m, 2);
	}
	void ActionA(ESCX_StateMachine* m, void*)
	{
		Log(m, 3);
	} void ActionB(ESCX_StateMachine* m, void*)
	{
		Log(m, 4);
	}
	void EnterA(ESCX_StateMachine* m, void*)
	{
		Log(m, 5);
	} void EnterB(ESCX_StateMachine* m, void*)
	{
		Log(m, 6);
	}
	int SelectA(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(0); return ESCX_STATUS_PROCESSED;
	} int SelectB(ESCX_StateMachine*, void*, ESCX_UINT16_T* transition_id) {
		*transition_id = (ESCX_UINT16_T)(1); return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[8] = { { 0,0,0,0,0,0,0,0,0,0,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,0,0,2,ESCX_NO_STATE,0,0,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,3,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,ExitA,0,2,0,0,0,2,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ EnterA,0,0,2,0,0,0,2,3,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,6,1,4,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,ExitB,0,5,0,0,0,2,5,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ EnterB,0,0,5,0,0,0,2,6,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[2] = { 2,5 }; const ESCX_StatePlan plans[4] = { { 0,1 },{ 1,1 },{ 2,1 },{ 3,1 } }; const ESCX_BYTE_T plan_states[4] = { 3,4,6,7 };
	const ESCX_Transition transitions[2] = { { 0,1,ActionA,ESCX_TRANSITION_EXTERNAL,3,2,1 },{ 2,3,ActionB,ESCX_TRANSITION_EXTERNAL,6,2,0 } }; const ESCX_SelectorFunction selectors[2] = { SelectA,SelectB };
	const ESCX_UINT16_T bases[8] = { 0,0,0,2,0,0,0,0 }; const ESCX_BYTE_T owners[8] = { 0,6,0,3,0,0,0,0 }; const ESCX_UINT16_T slots[8] = { ESCX_NO_SELECTOR,1,ESCX_NO_SELECTOR,0,ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,ESCX_NO_SELECTOR,ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,8,2,2,4,4,2,8,1,2 };
}
TEST(GlobalMicrostep, ExitsThenActionsThenEntriesInNormativeOrder)
{
	ESCX_StateMachine m; ESCX_BYTE_T bits[1];
	LogData d = { { 0 },0 }; ESCX_StateMachineInit(&m, &def, &d, bits, 1, 0, 0, 0); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); d.count = 0;
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0)); 
	ASSERT_EQ(6, d.count); EXPECT_EQ(2, d.values[0]); 
	EXPECT_EQ(1, d.values[1]); EXPECT_EQ(4, d.values[2]); 
	EXPECT_EQ(3, d.values[3]); EXPECT_EQ(5, d.values[4]);
	EXPECT_EQ(6, d.values[5]);
}
