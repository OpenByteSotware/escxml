#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
#include <cstring>
namespace
{
	const ESCX_StateDescriptor kStates[2] = {
	 { 0,0,0,0,0,0,0,0,0,0,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,ESCX_NO_STATE,0,0,ESCX_NO_STATE,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T kChildren[1] = { 0 }; const ESCX_Transition kTransitions[1] = { { ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0 } };
	const ESCX_StatePlan kPlans[1] = { { 0,0 } }; const ESCX_BYTE_T kPlanStates[1] = { 0 }; const ESCX_SelectorFunction kSelectors[1] = { 0 };
	const ESCX_UINT16_T kBases[2] = { 0,0 }; const ESCX_BYTE_T kOwners[1] = { 0 }; const ESCX_UINT16_T kSlots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition kDefinition = { kStates,kChildren,kTransitions,kPlans,kPlanStates,kSelectors,kBases,kOwners,kSlots,2,1,1,1,1,1,1,1,4 };
	class MachineTest : public ::testing::Test
	{
	protected: ESCX_StateMachine machine; ESCX_BYTE_T bits[1]; void SetUp() override
	{
		std::memset(&machine, 0, sizeof(machine)); 
		ESCX_StateMachineInit(&machine, &kDefinition, 0, bits, 1, 0, 0, 0); 
		ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&machine));
	}
	};
	TEST_F(MachineTest, ConfiguresAndClearsErrorState)
	{
		EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureErrorEvents(&machine, 2, 3)); 
		EXPECT_EQ(ESCX_ERROR_NONE, ESCX_StateMachineGetLastError(&machine)); 
		EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineGetLastErrorStatus(&machine)); 
		ESCX_StateMachineClearError(&machine);
		EXPECT_EQ(ESCX_ERROR_NONE, ESCX_StateMachineGetLastError(&machine));
	}
	TEST_F(MachineTest, RejectsOutOfRangeErrorEvent)
	{
		EXPECT_EQ(ESCX_STATUS_INVALID_EVENT, ESCX_StateMachineConfigureErrorEvents(&machine, 4, ESCX_NO_EVENT));
	}
}
