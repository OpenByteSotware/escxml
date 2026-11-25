#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace
{
	struct Data
	{
		int deep; int right;
	};
	void Deep(ESCX_StateMachine* m, void*)
	{
		++static_cast<Data*>(m->instance)->deep;
	}
	void Right(ESCX_StateMachine* m, void*)
	{
		++static_cast<Data*>(m->instance)->right;
	}
	ESCX_UINT16_T SelectDeep(ESCX_StateMachine*, void*)
	{
		return 0;
	}
	ESCX_UINT16_T SelectRight(ESCX_StateMachine*, void*)
	{
		return 1;
	}
	/* Sources 4 and 6 are in unrelated parallel branches. Source 4 is deeper,
	   but transition 1 is earlier in document order and must win. */
	const ESCX_StateDescriptor states[7] = {
	 { 0 },
	 { 0,0,0,0,0,2,ESCX_NO_STATE,0,0,ESCX_STATE_PARALLEL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,1,0,0,3,1,1,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,2,0,0,4,2,2,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,3,0,0,0,3,3,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,1,0,0,6,1,4,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN },
	 { 0,0,0,5,0,0,0,2,5,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN }
	};
	const ESCX_BYTE_T children[2] = { 2,5 };
	const ESCX_StatePlan plans[2] = { { 0,6 },{ 6,6 } };
	const ESCX_BYTE_T plan_states[12] = { 4,6,3,5,2,1, 1,2,5,3,6,4 };
	const ESCX_Transition transitions[2] = {
	 { 0,1,Deep,ESCX_TRANSITION_EXTERNAL,4,3,1 },
	 { 0,1,Right,ESCX_TRANSITION_EXTERNAL,6,2,0 }
	};
	const ESCX_SelectorFunction selectors[2] = { SelectDeep,SelectRight };
	const ESCX_UINT16_T bases[7] = { 0,0,0,0,0,0,0 };
	const ESCX_BYTE_T owners[2] = { 4,6 };
	const ESCX_UINT16_T slots[2] = { 0,1 };
	const ESCX_StateMachineDefinition def = { states,children,transitions,plans,plan_states,selectors,bases,owners,slots,7,2,2,2,12,2,2,1,2 };
	/* candidate rows: deep does not preempt earlier unrelated right; right preempts deep. */
	const ESCX_BYTE_T normative_bits[2] = { 0x00U,0x01U };
	const ESCX_PreemptionMap normative = { normative_bits,2,1 };
	void Init(ESCX_StateMachine* m, ESCX_BYTE_T* active, Data* d)
	{
		ESCX_StateMachineInit(m, &def, d, active, 1, 0, 0, 0);
	}
}
TEST(PreemptionValidation, AcceptsSourceAncestryAndDocumentOrderRelation)
{
	ESCX_StateMachine m = {}; 
	ESCX_BYTE_T active[1] = {}; 
	Data d = { 0,0 }; Init(&m, active, &d); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigurePreemptionMap(&m, &normative)); 
	EXPECT_EQ(&normative, m.preemption_map);
}
TEST(PreemptionValidation, RejectsDepthBasedRelationForUnrelatedSources)
{
	ESCX_StateMachine m = {}; 
	ESCX_BYTE_T active[1] = {}; 
	Data d = { 0,0 }; 
	const ESCX_BYTE_T depth_bits[2] = { 0x02U,0x00U }; 
	const ESCX_PreemptionMap bad = { depth_bits,2,1 }; 
	Init(&m, active, &d); 
	EXPECT_EQ(ESCX_STATUS_INVALID_ARGUMENT, ESCX_StateMachineConfigurePreemptionMap(&m, &bad)); 
	EXPECT_EQ(nullptr, m.preemption_map);
}
TEST(PreemptionValidation, GeneratedMapSelectsEarlierUnrelatedTransition)
{
	ESCX_StateMachine m = {}; 
	ESCX_BYTE_T active[1] = {}; 
	Data d = { 0,0 }; 
	Init(&m, active, &d); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigurePreemptionMap(&m, &normative)); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0)); 
	EXPECT_EQ(0, d.deep); 
	EXPECT_EQ(1, d.right);
}
TEST(PreemptionValidation, CorrectFallbackAlsoUsesAncestryNotDepth)
{
	ESCX_StateMachine m = {}; 
	ESCX_BYTE_T active[1] = {}; 
	Data d = { 0,0 }; 
	Init(&m, active, &d); 
	ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineStart(&m)); 
	EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSend(&m, 1, 0)); 
	EXPECT_EQ(0, d.deep); 
	EXPECT_EQ(1, d.right);
}
