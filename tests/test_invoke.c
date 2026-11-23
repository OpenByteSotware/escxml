#include "InvokeMachine.h"
#include <stdio.h>
static int TestComplete(void){ESCX_StateMachine m;InvokeData d;ESCX_InvokeInstance invokes[1];
ESCX_BYTE_T active[ESCX_ACTIVE_BYTES(InvokeMachine_STATE_COUNT)];
d.starts=d.cancels=d.forwards=d.finalizes=0;d.last_event=0U;
InvokeMachine_Init(&m,&d,active,(ESCX_BYTE_T)sizeof(active),(ESCX_HistoryStorage*)0,(ESCX_StartTimerFunction)0,(ESCX_CancelTimerFunction)0);if(InvokeMachine_Configure(&m,invokes)!=ESCX_STATUS_PROCESSED)return 1;if(ESCX_StateMachineStart(&m)!=ESCX_STATUS_PROCESSED)return 2;if(d.starts!=1||invokes[0].active==ESCX_FALSE)return 3;if(ESCX_StateMachineSend(&m,IM_Ping,(void*)0)!=ESCX_STATUS_PROCESSED)return 4;if(d.forwards!=1||d.last_event!=IM_Ping)return 5;if(ESCX_StateMachineReceiveInvokeEvent(&m,0U,IM_ChildEvent,(void*)0)!=ESCX_STATUS_PROCESSED)return 6;if(d.finalizes!=1||d.last_event!=IM_ChildEvent)return 7;if(ESCX_StateMachineCompleteInvoke(&m,0U,(void*)0)!=ESCX_STATUS_PROCESSED)return 8;if(d.finalizes!=2||!ESCX_StateMachineIsInState(&m,IM_Done)||invokes[0].active!=ESCX_FALSE)return 9;return 0;}
static int TestCancel(void){ESCX_StateMachine m;InvokeData d;ESCX_InvokeInstance invokes[1];
ESCX_BYTE_T active[ESCX_ACTIVE_BYTES(InvokeMachine_STATE_COUNT)];
d.starts=d.cancels=d.forwards=d.finalizes=0;d.last_event=0U;
InvokeMachine_Init(&m,&d,active,(ESCX_BYTE_T)sizeof(active),(ESCX_HistoryStorage*)0,
	(ESCX_StartTimerFunction)0,(ESCX_CancelTimerFunction)0);
if(InvokeMachine_Configure(&m,invokes)!=ESCX_STATUS_PROCESSED)return 20;
if(ESCX_StateMachineStart(&m)!=ESCX_STATUS_PROCESSED)return 21;ESCX_StateMachineStop(&m);
if(d.cancels!=1||invokes[0].active!=ESCX_FALSE)return 22;return 0;}
int main(void){int r;r=TestComplete();if(r)return r;
r=TestCancel();if(r)return r;printf("invoked child machine tests passed\n");return 0;}
