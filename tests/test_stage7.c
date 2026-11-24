#include "StateMachine.h"
#include <assert.h>
#include <string.h>
static const ESCX_StateDescriptor states[2]={
	{0,0,0,0,0,0,0,0,0,0,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN},
	{0,0,0,ESCX_NO_STATE,0,0,ESCX_NO_STATE,0,0,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR,ESCX_NO_EVENT,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN}};
static const ESCX_BYTE_T children[1]={0};
static const ESCX_Transition transitions[1]={{
		ESCX_NO_PLAN,ESCX_NO_PLAN,0,ESCX_TRANSITION_TARGETLESS,0,0,0}};
static const ESCX_StatePlan plans[1]={{0,0}};static const ESCX_BYTE_T plan_states[1]={0};
static const ESCX_SelectorFunction selectors[1]={0};
static const ESCX_UINT16_T bases[2]={0,0};static const ESCX_BYTE_T owners[1]={0};
static const ESCX_UINT16_T slots[1]={ESCX_NO_SELECTOR};
static const ESCX_StateMachineDefinition def={states,children,transitions,plans,plan_states,selectors,bases,owners,slots,2,1,1,1,1,1,1,1,2};
int main(void){
	ESCX_StateMachine m;ESCX_BYTE_T bits[1];ESCX_SendRequest q;
memset(&m,0,sizeof(m));ESCX_StateMachineInit(&m,&def,0,bits,1,0,0,0);
assert(ESCX_StateMachineStart(&m)==ESCX_STATUS_PROCESSED);
q.event_id=1;q.target=ESCX_SEND_TARGET_INTERNAL;q.target_id=0;q.send_id=7;
q.delay_ticks=10;q.event_data=0;assert(ESCX_StateMachineSendRequest(&m,&q)==ESCX_STATUS_PROCESSED);
assert(ESCX_StateMachineCancelSend(&m,7)==ESCX_STATUS_PROCESSED);
assert(ESCX_StateMachineCancelSend(&m,7)==ESCX_STATUS_SEND_NOT_FOUND);
q.send_id=8;assert(ESCX_StateMachineSendRequest(&m,&q)==ESCX_STATUS_PROCESSED);
assert(ESCX_StateMachineAdvanceTime(&m,9)==ESCX_STATUS_PROCESSED);
assert(ESCX_StateMachineAdvanceTime(&m,10)==ESCX_STATUS_PROCESSED);
ESCX_StateMachineStop(&m);return 0;}
