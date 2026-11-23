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
#define ESCX_NO_RANGE ESCX_INVALID_INDEX
#define ESCX_INVALID_TIMER_ID (-1)

struct ESCX_StateMachine;
typedef ESCX_BOOL_T (*ESCX_GuardFunction)(struct ESCX_StateMachine *, void *);
typedef void (*ESCX_ActionFunction)(struct ESCX_StateMachine *, void *);
typedef void (*ESCX_StateFunction)(struct ESCX_StateMachine *, void *);
typedef int (*ESCX_StartTimerFunction)(struct ESCX_StateMachine *, const char *, int, int);
typedef int (*ESCX_CancelTimerFunction)(struct ESCX_StateMachine *, const char *, int);

typedef struct ESCX_StateDescriptor {
    ESCX_StateFunction on_entry;
    ESCX_StateFunction on_exit;
    ESCX_StateFunction on_state;
} ESCX_StateDescriptor;

typedef struct ESCX_Transition {
    ESCX_BYTE_T target_state;
    ESCX_GuardFunction guard;
    ESCX_ActionFunction action;
} ESCX_Transition;

typedef struct ESCX_TransitionRange {
    ESCX_UINT16_T first_transition;
    ESCX_BYTE_T transition_count;
} ESCX_TransitionRange;

typedef enum ESCX_ProcessStatus {
    ESCX_STATUS_IGNORED = 0,
    ESCX_STATUS_PROCESSED = 1,
    ESCX_STATUS_INVALID_EVENT = 2,
    ESCX_STATUS_NOT_RUNNING = 3,
    ESCX_STATUS_QUEUE_FULL = 4,
    ESCX_STATUS_INVALID_ARGUMENT = 5
} ESCX_ProcessStatus;

typedef struct ESCX_StateMachineDefinition {
    const ESCX_StateDescriptor *states;
    const ESCX_Transition *transitions;
    const ESCX_TransitionRange *ranges;
    const ESCX_UINT16_T *state_base;
    const ESCX_BYTE_T *slot_owner;
    const ESCX_UINT16_T *slot_range;
    ESCX_UINT16_T state_count;
    ESCX_UINT16_T transition_count;
    ESCX_UINT16_T range_count;
    ESCX_UINT16_T slot_count;
    ESCX_BYTE_T initial_state;
    ESCX_BYTE_T event_count;
} ESCX_StateMachineDefinition;

typedef struct ESCX_StateMachine {
    const ESCX_StateMachineDefinition *definition;
    void *instance;
    ESCX_BYTE_T current_state;
    ESCX_BYTE_T current_event;
    ESCX_BOOL_T running;
    ESCX_BOOL_T processing;
#if ESCX_ENABLE_TIMERS
    int timer_id;
    ESCX_StartTimerFunction start_timer;
    ESCX_CancelTimerFunction cancel_timer;
#endif
    ESCX_EventQueue event_queue;
} ESCX_StateMachine;

void ESCX_StateMachineInit(ESCX_StateMachine *, const ESCX_StateMachineDefinition *, void *, ESCX_StartTimerFunction, ESCX_CancelTimerFunction);
ESCX_ProcessStatus ESCX_StateMachineStart(ESCX_StateMachine *);
void ESCX_StateMachineStop(ESCX_StateMachine *);
ESCX_ProcessStatus ESCX_StateMachineSend(ESCX_StateMachine *, ESCX_BYTE_T, void *);
ESCX_ProcessStatus ESCX_StateMachineTimerExpired(ESCX_StateMachine *, int, void *);
ESCX_BOOL_T ESCX_StateMachineIsInState(const ESCX_StateMachine *, ESCX_BYTE_T);

#define ESCX_STATE_ENUM_BEGIN(name) typedef enum name##_StateId { name##_NO_STATE = 0,
#define ESCX_STATE_ENUM(value) value,
#define ESCX_STATE_ENUM_END(name) name##_STATE_COUNT } name##_StateId;
#define ESCX_EVENT_ENUM_BEGIN(name) typedef enum name##_EventId { name##_TIMER_EVENT = 0,
#define ESCX_EVENT_ENUM(value) value,
#define ESCX_EVENT_ENUM_END(name) name##_EVENT_COUNT } name##_EventId;
#define ESCX_STATES_BEGIN(name) static const ESCX_StateDescriptor name##_states[] ESCX_ROM = {
#define ESCX_STATE(entry_fn, exit_fn, state_fn) { entry_fn, exit_fn, state_fn },
#define ESCX_STATES_END() };
#define ESCX_TRANSITIONS_BEGIN(name) static const ESCX_Transition name##_transitions[] ESCX_ROM = {
#define ESCX_TRANSITION(target, guard_fn, action_fn) { (ESCX_BYTE_T)(target), guard_fn, action_fn },
#define ESCX_TRANSITIONS_END() };
#define ESCX_RANGES_BEGIN(name) static const ESCX_TransitionRange name##_ranges[] ESCX_ROM = {
#define ESCX_RANGE(first, count) { (ESCX_UINT16_T)(first), (ESCX_BYTE_T)(count) },
#define ESCX_RANGES_END() };
#define ESCX_STATE_BASE_BEGIN(name) static const ESCX_UINT16_T name##_state_base[] ESCX_ROM = {
#define ESCX_INDEX_U16(value) (ESCX_UINT16_T)(value),
#define ESCX_STATE_BASE_END() };
#define ESCX_SLOT_OWNER_BEGIN(name) static const ESCX_BYTE_T name##_slot_owner[] ESCX_ROM = {
#define ESCX_INDEX_U8(value) (ESCX_BYTE_T)(value),
#define ESCX_SLOT_OWNER_END() };
#define ESCX_SLOT_RANGE_BEGIN(name) static const ESCX_UINT16_T name##_slot_range[] ESCX_ROM = {
#define ESCX_SLOT_RANGE_END() };
#define ESCX_DEFINE_MACHINE(name, initial) \
    ESCX_COMPILE_ASSERT_N((sizeof(name##_states)/sizeof(name##_states[0])) <= ESCX_MAX_STATES, states); \
    ESCX_COMPILE_ASSERT_N((sizeof(name##_transitions)/sizeof(name##_transitions[0])) <= ESCX_MAX_TRANSITIONS, transitions); \
    ESCX_COMPILE_ASSERT_N(name##_EVENT_COUNT <= ESCX_MAX_EVENTS, events); \
    ESCX_COMPILE_ASSERT_N((sizeof(name##_state_base)/sizeof(name##_state_base[0])) == (sizeof(name##_states)/sizeof(name##_states[0])), bases); \
    ESCX_COMPILE_ASSERT_N((sizeof(name##_slot_owner)/sizeof(name##_slot_owner[0])) == (sizeof(name##_slot_range)/sizeof(name##_slot_range[0])), slots); \
    ESCX_COMPILE_ASSERT_N((sizeof(name##_ranges)/sizeof(name##_ranges[0])) <= ESCX_MAX_RANGES, ranges_limit); \
    ESCX_COMPILE_ASSERT_N((sizeof(name##_slot_owner)/sizeof(name##_slot_owner[0])) <= ESCX_MAX_SLOTS, slots_limit); \
    ESCX_COMPILE_ASSERT_N((sizeof(name##_ranges)/sizeof(name##_ranges[0])) < 65536UL, ranges_width); \
    ESCX_COMPILE_ASSERT_N((sizeof(name##_slot_owner)/sizeof(name##_slot_owner[0])) < 65536UL, slots_width); \
    static const ESCX_StateMachineDefinition name##_definition = { \
      name##_states, name##_transitions, name##_ranges, name##_state_base, name##_slot_owner, name##_slot_range, \
      (ESCX_UINT16_T)(sizeof(name##_states)/sizeof(name##_states[0])), \
      (ESCX_UINT16_T)(sizeof(name##_transitions)/sizeof(name##_transitions[0])), \
      (ESCX_UINT16_T)(sizeof(name##_ranges)/sizeof(name##_ranges[0])), \
      (ESCX_UINT16_T)(sizeof(name##_slot_owner)/sizeof(name##_slot_owner[0])), \
      (ESCX_BYTE_T)(initial), (ESCX_BYTE_T)(name##_EVENT_COUNT) }
#define ESCX_DECLARE_MACHINE(name) void name##_Init(ESCX_StateMachine *, void *, ESCX_StartTimerFunction, ESCX_CancelTimerFunction)
#define ESCX_IMPLEMENT_MACHINE(name) \
 void name##_Init(ESCX_StateMachine *m, void *i, ESCX_StartTimerFunction s, ESCX_CancelTimerFunction c) \
 { ESCX_StateMachineInit(m, &name##_definition, i, s, c); }
#ifdef __cplusplus
}
#endif
#endif
