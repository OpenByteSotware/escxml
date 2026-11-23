#ifndef FINAL_MACHINE_H
#define FINAL_MACHINE_H
#include "StateMachine.h"
typedef struct FinalData { int value; } FinalData;
ESCX_STATE_ENUM_BEGIN(FinalMachine)
 ESCX_STATE_ENUM(FM_Root) ESCX_STATE_ENUM(FM_Work) ESCX_STATE_ENUM(FM_Running) ESCX_STATE_ENUM(FM_WorkFinal) ESCX_STATE_ENUM(FM_TopFinal)
ESCX_STATE_ENUM_END(FinalMachine)
ESCX_EVENT_ENUM_BEGIN(FinalMachine) ESCX_EVENT_ENUM(FM_Go) ESCX_EVENT_ENUM(FM_DoneWork) ESCX_EVENT_ENUM_END(FinalMachine)
ESCX_DECLARE_MACHINE(FinalMachine);
#endif
