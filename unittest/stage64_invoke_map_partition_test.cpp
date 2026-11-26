#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	int Start(ESCX_StateMachine*, void*, ESCX_InvokeToken)
	{
		return ESCX_STATUS_PROCESSED;
	}
	const ESCX_StateDescriptor states[3] = 
	{ { 0 },{ 0,0,0,0,0,0,0,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,0,0,0,0,0,1,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[1] = { 0 }; 
	const ESCX_StatePlan plans[1] = { { 0,0 } }; 
	const ESCX_BYTE_T plan_states[1] = { 0 };
	const ESCX_Transition transitions[1] = 
	{ { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } }; 
	const ESCX_SelectorFunction selectors[1] = { 0 };
	const ESCX_UINT16_T bases[3] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 };
	const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = 
	{ states,children,transitions,plans,plan_states,selectors,bases,owners,slots,3,1,1,1,1,1,1,1,2 };
	void Init(ESCX_StateMachine* m, ESCX_BYTE_T* active,
		ESCX_InvokeInstance* instances, ESCX_InvokeDescriptor* invokes)
	{
		ESCX_StateMachineInit(m, &def, 0, active, 1, 0, 0, 0);
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokes(m, invokes, instances, 2));
	}
}
TEST(InvokeMapPartition, AcceptsExactDescriptorPartition)
{
	ESCX_StateMachine m = {};
	ESCX_BYTE_T active[1] = {};
	ESCX_InvokeInstance instances[2] = {};
	ESCX_InvokeDescriptor invokes[2] = { { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start },{ 2,ESCX_NO_EVENT,ESCX_FALSE,0,Start } };
	const ESCX_UINT16_T first[3] = { 0,0,1 }; const ESCX_BYTE_T count[3] = { 0,1,1 }; 
	const ESCX_InvokeMap map = { first,count,3,2 };
	Init(&m, active, instances, invokes); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureInvokeMap(&m, &map)); 
	EXPECT_EQ(&map, m.invoke_map);
}
TEST(InvokeMapPartition, RejectsOverlappingRanges)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instances[2] = {};
	ESCX_InvokeDescriptor invokes[2] = { { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start },{ 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start } };
	const ESCX_UINT16_T first[3] = { 0,0,0 }; 
	const ESCX_BYTE_T count[3] = { 0,2,1 }; 
	const ESCX_InvokeMap map = { first,count,3,2 };
	Init(&m, active, instances, invokes); EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureInvokeMap(&m, &map)); EXPECT_EQ(nullptr, m.invoke_map);
}
TEST(InvokeMapPartition, RejectsOmittedDescriptor)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instances[2] = {};
	ESCX_InvokeDescriptor invokes[2] = { { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start },{ 2,ESCX_NO_EVENT,ESCX_FALSE,0,Start } };
	const ESCX_UINT16_T first[3] = { 0,0,0 }; const ESCX_BYTE_T count[3] = { 0,1,0 }; 
	const ESCX_InvokeMap map = { first,count,3,2 };
	Init(&m, active, instances, invokes); EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureInvokeMap(&m, &map)); EXPECT_EQ(nullptr, m.invoke_map);
}
TEST(InvokeMapPartition, RejectsInvalidDescriptorOwner)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T active[1] = {}; ESCX_InvokeInstance instances[2] = {};
	ESCX_InvokeDescriptor invokes[2] = { { 1,ESCX_NO_EVENT,ESCX_FALSE,0,Start },{ ESCX_NO_STATE,ESCX_NO_EVENT,ESCX_FALSE,0,Start } };
	const ESCX_UINT16_T first[3] = { 0,0,1 }; 
	const ESCX_BYTE_T count[3] = { 0,1,1 }; 
	const ESCX_InvokeMap map = { first,count,3,2 };
	Init(&m, active, instances, invokes); 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureInvokeMap(&m, &map)); 
	EXPECT_EQ(nullptr, m.invoke_map);
}
