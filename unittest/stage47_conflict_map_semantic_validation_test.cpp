#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	const ESCX_StateDescriptor states[6] = 
	{ { 0 },{ 0,0,0,0,0,2,ESCX_NO_STATE,0,0,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,3,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,2,0,0,0,2,2,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,1,0,0,5,1,3,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },{ 0,0,0,4,0,0,0,2,4,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN } };
	const ESCX_BYTE_T children[2] = { 2,4 };
	/* t0 exits leaf 3; t1 exits leaf 5; t2 exits both leaves and common ancestors; t3 targetless. */
	const ESCX_StatePlan plans[6] = { { 0,1 },{ 1,1 },{ 2,1 },{ 3,1 },{ 4,5 },{ 9,5 } };
	const ESCX_BYTE_T ps[14] = { 3,3,5,5,3,5,2,4,1,1,2,4,3,5 };
	const ESCX_Transition tr[4] = { { 0,1,0,ESCX_TRANSITION_EXTERNAL,3,2,0 },{ 2,3,0,ESCX_TRANSITION_EXTERNAL,5,2,1 },{ 4,5,0,ESCX_TRANSITION_EXTERNAL,1,0,2 },{ ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,3,2,3 } };
	const ESCX_SelectorFunction selectors[1] = { 0 };
	const ESCX_UINT16_T bases[6] = { 0 }; const ESCX_BYTE_T owners[1] = { 0 }; 
	const ESCX_UINT16_T slots[1] = { ESCX_NO_SELECTOR };
	const ESCX_StateMachineDefinition def = 
	{ states,children,tr,plans,ps,selectors,bases,owners,slots,6,2,4,6,14,1,1,1,2 };
	/* Rows: 0 conflicts with 0,2; 1 with 1,2; 2 with 0,1,2; targetless only with itself. */
	const ESCX_BYTE_T valid_bits[4] = { 0x05U,0x06U,0x07U,0x08U }; 
	const ESCX_ConflictMap valid = { valid_bits,4,1 };
	void Init(ESCX_StateMachine* m, ESCX_BYTE_T* b)
	{
		ESCX_StateMachineInit(m, &def, 0, b, 1, 0, 0, 0);
	}
}
TEST(ConflictMapSemanticValidation, AcceptsExactSymmetricExitSetRelation)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; Init(&m, b); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureConflictMap(&m, &valid)); 
	EXPECT_EQ(&valid, m.conflict_map);
}
TEST(ConflictMapSemanticValidation, RejectsMissingConflictAndAsymmetry)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; const ESCX_BYTE_T bits[4] = 
	{ 0x01U,0x06U,0x07U,0x08U }; const ESCX_ConflictMap bad = { bits,4,1 };
	Init(&m, b); 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureConflictMap(&m, &bad)); 
	EXPECT_EQ(nullptr, m.conflict_map);
}
TEST(ConflictMapSemanticValidation, RejectsSpuriousOrthogonalOrTargetlessConflict)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; 
	const ESCX_BYTE_T orthogonal[4] = { 0x07U,0x07U,0x07U,0x08U }; 
	const ESCX_BYTE_T targetless[4] = { 0x0dU,0x06U,0x07U,0x09U }; 
	ESCX_ConflictMap bad = { orthogonal,4,1 }; Init(&m, b); 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureConflictMap(&m, &bad)); 
	bad.bits = targetless; 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureConflictMap(&m, &bad));
}
TEST(ConflictMapSemanticValidation, RejectsMissingDiagonalBit)
{
	ESCX_StateMachine m = {}; ESCX_BYTE_T b[1] = {}; 
	const ESCX_BYTE_T bits[4] = { 0x04U,0x06U,0x07U,0x08U }; 
	const ESCX_ConflictMap bad = { bits,4,1 }; 
	Init(&m, b); 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigureConflictMap(&m, &bad));
}
