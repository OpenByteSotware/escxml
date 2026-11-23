#ifndef MACROSTEP_MACHINE_H
#define MACROSTEP_MACHINE_H
#include "StateMachine.h"
typedef struct MacroData { int log[32]; int count; int allow_eventless; } MacroData;
ESCX_STATE_ENUM_BEGIN(MacrostepMachine)
 ESCX_STATE_ENUM(MS_Root) ESCX_STATE_ENUM(MS_Idle) ESCX_STATE_ENUM(MS_Work) ESCX_STATE_ENUM(MS_Done)
ESCX_STATE_ENUM_END(MacrostepMachine)
ESCX_EVENT_ENUM_BEGIN(MacrostepMachine)
 ESCX_EVENT_ENUM(MS_Start) ESCX_EVENT_ENUM(MS_InternalFinish)
ESCX_EVENT_ENUM_END(MacrostepMachine)
ESCX_DECLARE_MACHINE(MacrostepMachine);
#endif
