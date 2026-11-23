#ifndef ESCX_STATE_MACHINE_H
#define ESCX_STATE_MACHINE_H

#include "DataTypes.h"
#include "StateMachineEventQueue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESCX_NO_STATE ((ESCX_BYTE_T)0U)
#define ESCX_TIMER_EVENT ((ESCX_BYTE_T)0U)
#define ESCX_NO_EVENT ((ESCX_BYTE_T)255U)
#define ESCX_INVALID_TIMER_ID (-1)

struct ESCX_StateMachine;
typedef ESCX_BOOL_T (*ESCX_GuardFunction)(struct ESCX_StateMachine *machine, void *event_data);
typedef void (*ESCX_ActionFunction)(struct ESCX_StateMachine *machine, void *event_data);
typedef void (*ESCX_StateFunction)(struct ESCX_StateMachine *machine, void *event_data);
typedef int (*ESCX_StartTimerFunction)(struct ESCX_StateMachine *machine, const char *name, int type, int value);
typedef int (*ESCX_CancelTimerFunction)(struct ESCX_StateMachine *machine, const char *name, int timer_id);

typedef struct ESCX_StateDescriptor
{
    ESCX_StateFunction on_entry;
    ESCX_StateFunction on_exit;
    ESCX_StateFunction on_state;
} ESCX_StateDescriptor;

typedef struct ESCX_Transition
{
    ESCX_BYTE_T source_state;
    ESCX_BYTE_T event_id;
    ESCX_BYTE_T target_state;
    ESCX_GuardFunction guard;
    ESCX_ActionFunction action;
} ESCX_Transition;

typedef enum ESCX_ProcessStatus
{
    ESCX_STATUS_IGNORED = 0,
    ESCX_STATUS_PROCESSED = 1,
    ESCX_STATUS_INVALID_EVENT = 2,
    ESCX_STATUS_NOT_RUNNING = 3,
    ESCX_STATUS_QUEUE_FULL = 4,
    ESCX_STATUS_INVALID_ARGUMENT = 5
} ESCX_ProcessStatus;

typedef struct ESCX_StateMachineDefinition
{
    const ESCX_StateDescriptor *states;
    unsigned int state_count;
    const ESCX_Transition *transitions;
    unsigned int transition_count;
    ESCX_BYTE_T initial_state;
    ESCX_BYTE_T event_count;
} ESCX_StateMachineDefinition;

typedef struct ESCX_StateMachine
{
    const ESCX_StateMachineDefinition *definition;
    void *instance;
    ESCX_BYTE_T current_state;
    ESCX_BYTE_T current_event;
    ESCX_BOOL_T running;
    ESCX_BOOL_T processing;
    int timer_id;
    ESCX_StartTimerFunction start_timer;
    ESCX_CancelTimerFunction cancel_timer;
    ESCX_EventQueue event_queue;
} ESCX_StateMachine;

void ESCX_StateMachineInit(ESCX_StateMachine *machine, const ESCX_StateMachineDefinition *definition,
                          void *instance, ESCX_StartTimerFunction start_timer,
                          ESCX_CancelTimerFunction cancel_timer);
ESCX_ProcessStatus ESCX_StateMachineStart(ESCX_StateMachine *machine);
void ESCX_StateMachineStop(ESCX_StateMachine *machine);
ESCX_ProcessStatus ESCX_StateMachineSend(ESCX_StateMachine *machine, ESCX_BYTE_T event_id, void *event_data);
ESCX_ProcessStatus ESCX_StateMachineTimerExpired(ESCX_StateMachine *machine, int timer_id, void *event_data);
ESCX_BOOL_T ESCX_StateMachineIsInState(const ESCX_StateMachine *machine, ESCX_BYTE_T state_id);

#define ESCX_STATE_ENUM_BEGIN(name) typedef enum name##_StateId { name##_NO_STATE = 0,
#define ESCX_STATE_ENUM(value) value,
#define ESCX_STATE_ENUM_END(name) name##_STATE_COUNT } name##_StateId;
#define ESCX_EVENT_ENUM_BEGIN(name) typedef enum name##_EventId { name##_TIMER_EVENT = 0,
#define ESCX_EVENT_ENUM(value) value,
#define ESCX_EVENT_ENUM_END(name) name##_EVENT_COUNT } name##_EventId;

#define ESCX_STATES_BEGIN(name) static const ESCX_StateDescriptor name##_states[] = {
#define ESCX_STATE(entry_fn, exit_fn, state_fn) { entry_fn, exit_fn, state_fn },
#define ESCX_STATES_END() };

#define ESCX_TRANSITIONS_BEGIN(name) static const ESCX_Transition name##_transitions[] = {
#define ESCX_TRANSITION(source, event_id, target, guard_fn, action_fn) \
    { (ESCX_BYTE_T)(source), (ESCX_BYTE_T)(event_id), (ESCX_BYTE_T)(target), guard_fn, action_fn },
#define ESCX_TRANSITIONS_END() };

#define ESCX_DEFINE_MACHINE(name, initial_state) \
    static const ESCX_StateMachineDefinition name##_definition = { \
        name##_states, (unsigned int)(sizeof(name##_states) / sizeof(name##_states[0])), \
        name##_transitions, (unsigned int)(sizeof(name##_transitions) / sizeof(name##_transitions[0])), \
        (ESCX_BYTE_T)(initial_state), (ESCX_BYTE_T)(name##_EVENT_COUNT) \
    }

#define ESCX_DECLARE_MACHINE(name) \
    void name##_Init(ESCX_StateMachine *machine, void *instance, \
                     ESCX_StartTimerFunction start_timer, ESCX_CancelTimerFunction cancel_timer)

#define ESCX_IMPLEMENT_MACHINE(name) \
    void name##_Init(ESCX_StateMachine *machine, void *instance, \
                     ESCX_StartTimerFunction start_timer, ESCX_CancelTimerFunction cancel_timer) \
    { ESCX_StateMachineInit(machine, &name##_definition, instance, start_timer, cancel_timer); }

#ifdef __cplusplus
}
#endif
#endif
