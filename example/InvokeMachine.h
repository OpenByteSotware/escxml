#ifndef INVOKE_MACHINE_H
#define INVOKE_MACHINE_H
#include "StateMachine.h"
typedef struct InvokeData { int starts; int cancels; int forwards; int finalizes; ESCX_BYTE_T last_event; } InvokeData;
ESCX_STATE_ENUM_BEGIN(InvokeMachine)
 ESCX_STATE_ENUM(IM_Root) ESCX_STATE_ENUM(IM_Invoking) ESCX_STATE_ENUM(IM_Done)
ESCX_STATE_ENUM_END(InvokeMachine)
ESCX_EVENT_ENUM_BEGIN(InvokeMachine)
 ESCX_EVENT_ENUM(IM_DoneInvoke) ESCX_EVENT_ENUM(IM_Ping) ESCX_EVENT_ENUM(IM_ChildEvent)
ESCX_EVENT_ENUM_END(InvokeMachine)
ESCX_DECLARE_MACHINE(InvokeMachine);
ESCX_ProcessStatus InvokeMachine_Configure(ESCX_StateMachine *,ESCX_InvokeInstance *);
#endif
