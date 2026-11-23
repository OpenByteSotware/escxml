#ifndef ESCX_STATE_MACHINE_H
#define ESCX_STATE_MACHINE_H
#include "DataTypes.h"
#include "ESCXConfig.h"
#include "ESCXRom.h"
#include "StateMachineEventQueue.h"
#ifdef __cplusplus
extern "C" {
#endif
#define ESCX_NO_STATE ((ESCX_BYTE_T)0U)
#define ESCX_TIMER_EVENT ((ESCX_BYTE_T)0U)
#define ESCX_NO_EVENT ((ESCX_BYTE_T)255U)
#define ESCX_NO_OWNER ((ESCX_BYTE_T)255U)
#define ESCX_INVALID_INDEX ((ESCX_UINT16_T)65535U)
#define ESCX_NO_SELECTOR ESCX_INVALID_INDEX
#define ESCX_NO_TRANSITION ESCX_INVALID_INDEX
#define ESCX_NO_PLAN ESCX_INVALID_INDEX
#define ESCX_INVALID_TIMER_ID (-1)
#define ESCX_ACTIVE_BYTES(count) (((count) + 7U) / 8U)

struct ESCX_StateMachine;
typedef void (*ESCX_ActionFunction)(struct ESCX_StateMachine *, void *);
typedef void (*ESCX_StateFunction)(struct ESCX_StateMachine *, void *);
typedef ESCX_UINT16_T (*ESCX_SelectorFunction)(struct ESCX_StateMachine *, void *);
typedef int (*ESCX_StartTimerFunction)(struct ESCX_StateMachine *, const char *, int, int);
typedef int (*ESCX_CancelTimerFunction)(struct ESCX_StateMachine *, const char *, int);

typedef enum ESCX_StateType {
 ESCX_STATE_ATOMIC=0, ESCX_STATE_COMPOUND=1, ESCX_STATE_PARALLEL=2,
 ESCX_STATE_FINAL=3, ESCX_STATE_HISTORY_SHALLOW=4, ESCX_STATE_HISTORY_DEEP=5
} ESCX_StateType;
typedef enum ESCX_TransitionType {
 ESCX_TRANSITION_EXTERNAL=0, ESCX_TRANSITION_INTERNAL=1, ESCX_TRANSITION_TARGETLESS=2
} ESCX_TransitionType;

typedef struct ESCX_StateDescriptor {
 ESCX_StateFunction on_entry; ESCX_StateFunction on_exit; ESCX_StateFunction on_state;
 ESCX_BYTE_T parent_state; ESCX_UINT16_T first_child_index; ESCX_BYTE_T child_count;
 ESCX_BYTE_T initial_child; ESCX_BYTE_T depth; ESCX_BYTE_T document_order; ESCX_BYTE_T state_type;
} ESCX_StateDescriptor;
typedef struct ESCX_StatePlan { ESCX_UINT16_T first_state; ESCX_BYTE_T state_count; } ESCX_StatePlan;
typedef struct ESCX_Transition {
 ESCX_UINT16_T exit_plan; ESCX_UINT16_T entry_plan; ESCX_ActionFunction action; ESCX_BYTE_T type;
} ESCX_Transition;
typedef enum ESCX_ProcessStatus {
 ESCX_STATUS_IGNORED=0, ESCX_STATUS_PROCESSED=1, ESCX_STATUS_INVALID_EVENT=2,
 ESCX_STATUS_NOT_RUNNING=3, ESCX_STATUS_QUEUE_FULL=4, ESCX_STATUS_INVALID_ARGUMENT=5,
 ESCX_STATUS_HIERARCHY_LIMIT=6
} ESCX_ProcessStatus;
typedef struct ESCX_StateMachineDefinition {
 const ESCX_StateDescriptor *states; const ESCX_BYTE_T *children;
 const ESCX_Transition *transitions; const ESCX_StatePlan *plans; const ESCX_BYTE_T *plan_states;
 const ESCX_SelectorFunction *selectors; const ESCX_UINT16_T *state_base;
 const ESCX_BYTE_T *slot_owner; const ESCX_UINT16_T *slot_selector;
 ESCX_UINT16_T state_count, child_count, transition_count, plan_count, plan_state_count;
 ESCX_UINT16_T selector_count, slot_count; ESCX_BYTE_T initial_state, event_count;
} ESCX_StateMachineDefinition;
typedef struct ESCX_StateMachine {
 const ESCX_StateMachineDefinition *definition; void *instance;
 ESCX_BYTE_T *active_bits; ESCX_BYTE_T active_byte_count; ESCX_BYTE_T active_atomic_state;
 ESCX_BYTE_T current_event; ESCX_BOOL_T running; ESCX_BOOL_T processing;
#if ESCX_ENABLE_TIMERS
 int timer_id; ESCX_StartTimerFunction start_timer; ESCX_CancelTimerFunction cancel_timer;
#endif
 ESCX_EventQueue event_queue;
} ESCX_StateMachine;

void ESCX_StateMachineInit(ESCX_StateMachine *,const ESCX_StateMachineDefinition *,void *,ESCX_BYTE_T *,ESCX_BYTE_T,ESCX_StartTimerFunction,ESCX_CancelTimerFunction);
ESCX_ProcessStatus ESCX_StateMachineStart(ESCX_StateMachine *);
void ESCX_StateMachineStop(ESCX_StateMachine *);
ESCX_ProcessStatus ESCX_StateMachineSend(ESCX_StateMachine *,ESCX_BYTE_T,void *);
ESCX_ProcessStatus ESCX_StateMachineTimerExpired(ESCX_StateMachine *,int,void *);
ESCX_BOOL_T ESCX_StateMachineIsInState(const ESCX_StateMachine *,ESCX_BYTE_T);

#define ESCX_STATE_ENUM_BEGIN(n) typedef enum n##_StateId { n##_NO_STATE=0,
#define ESCX_STATE_ENUM(v) v,
#define ESCX_STATE_ENUM_END(n) n##_STATE_COUNT } n##_StateId;
#define ESCX_EVENT_ENUM_BEGIN(n) typedef enum n##_EventId { n##_TIMER_EVENT=0,
#define ESCX_EVENT_ENUM(v) v,
#define ESCX_EVENT_ENUM_END(n) n##_EVENT_COUNT } n##_EventId;
#define ESCX_STATES_BEGIN(n) static const ESCX_StateDescriptor n##_states[] ESCX_ROM={
#define ESCX_STATE_EX(en,ex,run,parent,first,count,initial,depth,order,type) {en,ex,run,(ESCX_BYTE_T)(parent),(ESCX_UINT16_T)(first),(ESCX_BYTE_T)(count),(ESCX_BYTE_T)(initial),(ESCX_BYTE_T)(depth),(ESCX_BYTE_T)(order),(ESCX_BYTE_T)(type)},
#define ESCX_STATE(en,ex,run) ESCX_STATE_EX(en,ex,run,ESCX_NO_STATE,0U,0U,ESCX_NO_STATE,0U,0U,ESCX_STATE_ATOMIC)
#define ESCX_STATES_END() };
#define ESCX_CHILDREN_BEGIN(n) static const ESCX_BYTE_T n##_children[] ESCX_ROM={
#define ESCX_CHILD(v) (ESCX_BYTE_T)(v),
#define ESCX_CHILDREN_END() };
#define ESCX_PLANS_BEGIN(n) static const ESCX_StatePlan n##_plans[] ESCX_ROM={
#define ESCX_PLAN(first,count) {(ESCX_UINT16_T)(first),(ESCX_BYTE_T)(count)},
#define ESCX_PLANS_END() };
#define ESCX_PLAN_STATES_BEGIN(n) static const ESCX_BYTE_T n##_plan_states[] ESCX_ROM={
#define ESCX_PLAN_STATE(v) (ESCX_BYTE_T)(v),
#define ESCX_PLAN_STATES_END() };
#define ESCX_TRANSITIONS_BEGIN(n) static const ESCX_Transition n##_transitions[] ESCX_ROM={
#define ESCX_TRANSITION_EX(exit_plan,entry_plan,action,type) {(ESCX_UINT16_T)(exit_plan),(ESCX_UINT16_T)(entry_plan),action,(ESCX_BYTE_T)(type)},
#define ESCX_TRANSITION(exit_plan,entry_plan,action) ESCX_TRANSITION_EX(exit_plan,entry_plan,action,ESCX_TRANSITION_EXTERNAL)
#define ESCX_TARGETLESS_TRANSITION(action) ESCX_TRANSITION_EX(ESCX_NO_PLAN,ESCX_NO_PLAN,action,ESCX_TRANSITION_TARGETLESS)
#define ESCX_TRANSITIONS_END() };
#define ESCX_SELECTOR_BEGIN(n) static ESCX_UINT16_T n(ESCX_StateMachine *machine,void *event_data){
#define ESCX_SELECTOR_SWITCH(x) switch(x){
#define ESCX_SELECTOR_CASE(v,t) case(v):return(ESCX_UINT16_T)(t);
#define ESCX_SELECTOR_DEFAULT(t) default:return(ESCX_UINT16_T)(t);
#define ESCX_SELECTOR_SWITCH_END() }
#define ESCX_SELECTOR_RETURN(t) return(ESCX_UINT16_T)(t);
#define ESCX_SELECTOR_END() }
#define ESCX_SELECTORS_BEGIN(n) static const ESCX_SelectorFunction n##_selectors[] ESCX_ROM={
#define ESCX_SELECTOR(f) f,
#define ESCX_SELECTORS_END() };
#define ESCX_STATE_BASE_BEGIN(n) static const ESCX_UINT16_T n##_state_base[] ESCX_ROM={
#define ESCX_INDEX_U16(v) (ESCX_UINT16_T)(v),
#define ESCX_STATE_BASE_END() };
#define ESCX_SLOT_OWNER_BEGIN(n) static const ESCX_BYTE_T n##_slot_owner[] ESCX_ROM={
#define ESCX_INDEX_U8(v) (ESCX_BYTE_T)(v),
#define ESCX_SLOT_OWNER_END() };
#define ESCX_SLOT_SELECTOR_BEGIN(n) static const ESCX_UINT16_T n##_slot_selector[] ESCX_ROM={
#define ESCX_SLOT_SELECTOR_END() };
#define ESCX_DEFINE_MACHINE(n,initial) \
 ESCX_COMPILE_ASSERT_N((sizeof(n##_states)/sizeof(n##_states[0]))<=ESCX_MAX_STATES,states); \
 ESCX_COMPILE_ASSERT_N((sizeof(n##_transitions)/sizeof(n##_transitions[0]))<=ESCX_MAX_TRANSITIONS,transitions); \
 ESCX_COMPILE_ASSERT_N((sizeof(n##_state_base)/sizeof(n##_state_base[0]))==(sizeof(n##_states)/sizeof(n##_states[0])),bases); \
 ESCX_COMPILE_ASSERT_N((sizeof(n##_slot_owner)/sizeof(n##_slot_owner[0]))==(sizeof(n##_slot_selector)/sizeof(n##_slot_selector[0])),slots); \
 static const ESCX_StateMachineDefinition n##_definition={n##_states,n##_children,n##_transitions,n##_plans,n##_plan_states,n##_selectors,n##_state_base,n##_slot_owner,n##_slot_selector,(ESCX_UINT16_T)(sizeof(n##_states)/sizeof(n##_states[0])),(ESCX_UINT16_T)(sizeof(n##_children)/sizeof(n##_children[0])),(ESCX_UINT16_T)(sizeof(n##_transitions)/sizeof(n##_transitions[0])),(ESCX_UINT16_T)(sizeof(n##_plans)/sizeof(n##_plans[0])),(ESCX_UINT16_T)(sizeof(n##_plan_states)/sizeof(n##_plan_states[0])),(ESCX_UINT16_T)(sizeof(n##_selectors)/sizeof(n##_selectors[0])),(ESCX_UINT16_T)(sizeof(n##_slot_owner)/sizeof(n##_slot_owner[0])),(ESCX_BYTE_T)(initial),(ESCX_BYTE_T)(n##_EVENT_COUNT)}
#define ESCX_DECLARE_MACHINE(n) void n##_Init(ESCX_StateMachine *,void *,ESCX_BYTE_T *,ESCX_BYTE_T,ESCX_StartTimerFunction,ESCX_CancelTimerFunction)
#define ESCX_IMPLEMENT_MACHINE(n) void n##_Init(ESCX_StateMachine*m,void*i,ESCX_BYTE_T*b,ESCX_BYTE_T z,ESCX_StartTimerFunction s,ESCX_CancelTimerFunction c){ESCX_StateMachineInit(m,&n##_definition,i,b,z,s,c);}
#ifdef __cplusplus
}
#endif
#endif
