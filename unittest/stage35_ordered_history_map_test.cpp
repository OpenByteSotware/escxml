#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}

namespace
{
	const ESCX_StateDescriptor states[4] = {
		{ 0 },
		{ 0,0,0,ESCX_NO_STATE,0,2,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
		{ 0,0,0,1,0,0,ESCX_NO_STATE,1,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
		{ 0,0,0,1,0,0,ESCX_NO_STATE,1,2,ESCX_STATE_HISTORY_DEEP,ESCX_NO_SELECTOR,ESCX_NO_EVENT,0,0 }
	};
	const ESCX_BYTE_T children[2] = { 2,3 };
	const ESCX_Transition transitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } };
	const ESCX_StatePlan plans[1] = { { 0,1 } };
	const ESCX_BYTE_T plan_states[1] = { 2 };
	const ESCX_SelectorFunction selectors[1] = { 0 };
	const ESCX_UINT16_T bases[4] = { 0,0,0,0 };
	const ESCX_BYTE_T owners[1] = { 0 };
	const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,4,2,1,1,1,1,1,1,2 };
}

TEST(OrderedHistoryMap, AcceptsGeneratedRangesBeforeStart)
{
	ESCX_StateMachine machine = {};
	ESCX_BYTE_T active[1] = {}, valid[1] = {}, history_bits[1] = {};
	ESCX_HistoryStorage history = { valid,history_bits,1,1 };
	const ESCX_UINT16_T first[1] = { 0 };
	const ESCX_BYTE_T count[1] = { 1 };
	const ESCX_BYTE_T mapped_states[1] = { 2 };
	const ESCX_HistoryMap map = { first,count,mapped_states,1,1 };
	ESCX_StateMachineInit(&machine, &def, 0, active, 1, &history, 0, 0);
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureHistoryMap(&machine, &map));
	EXPECT_EQ(&map, machine.history_map);
}

TEST(OrderedHistoryMap, RejectsRangePastGeneratedStateList)
{
	ESCX_StateMachine machine = {};
	ESCX_BYTE_T active[1] = {}, valid[1] = {}, history_bits[1] = {};
	ESCX_HistoryStorage history = { valid,history_bits,1,1 };
	const ESCX_UINT16_T first[1] = { 1 };
	const ESCX_BYTE_T count[1] = { 1 };
	const ESCX_BYTE_T mapped_states[1] = { 2 };
	const ESCX_HistoryMap map = { first,count,mapped_states,1,1 };
	ESCX_StateMachineInit(&machine, &def, 0, active, 1, &history, 0, 0);
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureHistoryMap(&machine, &map));
}
