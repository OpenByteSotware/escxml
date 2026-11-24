#include <gtest/gtest.h>
extern "C" {
#include "StateMachine.h"
}
namespace {
struct Data{int exits[4];int count;};void ExitRoot(ESCX_StateMachine*m,void*){static_cast<Data*>(m->instance)->exits[static_cast<Data*>(m->instance)->count++]=1;}void ExitFinal(ESCX_StateMachine*m,void*){static_cast<Data*>(m->instance)->exits[static_cast<Data*>(m->instance)->count++]=2;}
const ESCX_StateDescriptor states[3]={{0},{0,ExitRoot,0,0,0,0,2,0,0,ESCX_STATE_COMPOUND,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN},{0,ExitFinal,0,1,0,0,0,1,1,ESCX_STATE_FINAL,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN}};
const ESCX_BYTE_T children[1]={0};const ESCX_StatePlan plans[1]={{0,0}};const ESCX_BYTE_T plan_states[1]={0};const ESCX_Transition transitions[1]={{ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0}};const ESCX_SelectorFunction selectors[1]={0};const ESCX_UINT16_T bases[3]={0};const ESCX_BYTE_T owners[1]={0};const ESCX_UINT16_T slots[1]={ESCX_NO_SELECTOR};const ESCX_StateMachineDefinition def={states,children,transitions,plans,plan_states,selectors,bases,owners,slots,3,1,1,1,1,1,1,1,2};
}
TEST(InterpreterTermination, TopLevelCompletionExitsAndStopsButRemainsFinished){ESCX_StateMachine m={};ESCX_BYTE_T bits[1]={};Data data={{0},0};ESCX_StateMachineInit(&m,&def,&data,bits,1,0,0,0);ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&m));EXPECT_EQ(2,data.count);EXPECT_EQ(2,data.exits[0]);EXPECT_EQ(1,data.exits[1]);EXPECT_FALSE(ESCX_StateMachineIsRunning(&m));EXPECT_TRUE(ESCX_StateMachineIsFinished(&m));EXPECT_FALSE(ESCX_StateMachineIsInState(&m,1));EXPECT_EQ(ESCX_STATUS_NOT_RUNNING,ESCX_StateMachineSend(&m,1,0));}
TEST(InterpreterTermination, FinishedMachineCanStartNewSession){ESCX_StateMachine m={};ESCX_BYTE_T bits[1]={};Data data={{0},0};ESCX_StateMachineInit(&m,&def,&data,bits,1,0,0,0);ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&m));ASSERT_TRUE(ESCX_StateMachineIsFinished(&m));data.count=0;ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineStart(&m));EXPECT_TRUE(ESCX_StateMachineIsFinished(&m));EXPECT_EQ(2,data.count);}
