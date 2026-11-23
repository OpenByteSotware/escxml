#ifndef HIERARCHY_MACHINE_H
#define HIERARCHY_MACHINE_H
#include "StateMachine.h"
typedef struct HierarchyData { int log[32]; int count; } HierarchyData;
ESCX_STATE_ENUM_BEGIN(HierarchyMachine)
 ESCX_STATE_ENUM(HM_Root) ESCX_STATE_ENUM(HM_Parent) ESCX_STATE_ENUM(HM_ChildA) ESCX_STATE_ENUM(HM_ChildB) ESCX_STATE_ENUM(HM_Outside)
ESCX_STATE_ENUM_END(HierarchyMachine)
ESCX_EVENT_ENUM_BEGIN(HierarchyMachine)
 ESCX_EVENT_ENUM(HM_Next) ESCX_EVENT_ENUM(HM_OutsideEvent) ESCX_EVENT_ENUM(HM_ParentEvent) ESCX_EVENT_ENUM(HM_Targetless)
ESCX_EVENT_ENUM_END(HierarchyMachine)
ESCX_DECLARE_MACHINE(HierarchyMachine);
#endif
