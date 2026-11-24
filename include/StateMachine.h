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
typedef signed long ESCX_TimerId;
#define ESCX_INVALID_TIMER_ID ((ESCX_TimerId)-1L)
#define ESCX_NO_HISTORY_SLOT ((ESCX_BYTE_T)255U)
#define ESCX_NO_INVOKE ((ESCX_BYTE_T)255U)
#define ESCX_NO_SEND_ID ((ESCX_UINT16_T)0U)
#define ESCX_NO_EVENT_PARENT ESCX_NO_EVENT
#define ESCX_ACTIVE_BYTES(count) (((count) + 7U) / 8U)

struct ESCX_StateMachine;
struct ESCX_InvokeDescriptor;
  struct ESCX_Event;

typedef void (*ESCX_ActionFunction)(struct ESCX_StateMachine *, void *);
typedef void (*ESCX_StateFunction)(struct ESCX_StateMachine *, void *);
  typedef int (*ESCX_StatusActionFunction)(struct ESCX_StateMachine *, void *);
  typedef int (*ESCX_StatusStateFunction)(struct ESCX_StateMachine *, void *);
  typedef int (*ESCX_DoneDataFunction)(struct ESCX_StateMachine *, void **);
typedef ESCX_UINT16_T (*ESCX_SelectorFunction)(struct ESCX_StateMachine *, void *);
typedef ESCX_TimerId (*ESCX_StartTimerFunction)(struct ESCX_StateMachine *, unsigned long, void *);
typedef ESCX_BOOL_T (*ESCX_CancelTimerFunction)(struct ESCX_StateMachine *, ESCX_TimerId, void *);
typedef int (*ESCX_InvokeStartFunction)(struct ESCX_StateMachine *, void *);
  typedef int (*ESCX_InvokeInitializeFunction)(struct ESCX_StateMachine *, void *, ESCX_BYTE_T);
typedef void (*ESCX_InvokeCancelFunction)(struct ESCX_StateMachine *, void *);
typedef int (*ESCX_InvokeForwardFunction)(struct ESCX_StateMachine *, void *, ESCX_BYTE_T, void *);
  typedef int (*ESCX_InvokeForwardEventFunction)(struct ESCX_StateMachine *,void *,const struct ESCX_Event *);
typedef void (*ESCX_InvokeFinalizeFunction)(struct ESCX_StateMachine *, void *, ESCX_BYTE_T, void *);
  struct ESCX_Event;
  typedef int (*ESCX_InvokeFinalizeStatusFunction)(struct ESCX_StateMachine *, void *, const struct ESCX_Event *);

typedef enum ESCX_EventType {
 ESCX_EVENT_TYPE_PLATFORM=0, ESCX_EVENT_TYPE_INTERNAL=1, ESCX_EVENT_TYPE_EXTERNAL=2
} ESCX_EventType;
typedef enum ESCX_EventOriginType {
 ESCX_EVENT_ORIGIN_NONE=0, ESCX_EVENT_ORIGIN_PLATFORM=1,
 ESCX_EVENT_ORIGIN_SCXML=2, ESCX_EVENT_ORIGIN_INVOKE=3,
 ESCX_EVENT_ORIGIN_TIMER=4
} ESCX_EventOriginType;
typedef struct ESCX_Event {
 ESCX_BYTE_T name_id; ESCX_BYTE_T type; ESCX_BYTE_T origin_type;
 ESCX_BYTE_T invoke_id; ESCX_UINT16_T send_id; void *data;
} ESCX_Event;
typedef struct ESCX_SystemContext {
 ESCX_UINT16_T session_id; ESCX_UINT16_T machine_name_id;
 const void *io_processors; void *platform_data;
} ESCX_SystemContext;
typedef enum ESCX_DataBinding { ESCX_DATA_BINDING_EARLY=0, ESCX_DATA_BINDING_LATE=1 } ESCX_DataBinding;
  typedef enum ESCX_StateType { ESCX_STATE_ATOMIC=0, ESCX_STATE_COMPOUND=1, ESCX_STATE_PARALLEL=2, ESCX_STATE_FINAL=3, ESCX_STATE_HISTORY_SHALLOW=4, ESCX_STATE_HISTORY_DEEP=5 } ESCX_StateType;
typedef enum ESCX_TransitionType { ESCX_TRANSITION_EXTERNAL=0, ESCX_TRANSITION_INTERNAL=1, ESCX_TRANSITION_TARGETLESS=2 } ESCX_TransitionType;
typedef struct ESCX_StateDescriptor {
 ESCX_StateFunction on_entry; ESCX_StateFunction on_exit; ESCX_StateFunction on_state;
 ESCX_BYTE_T parent_state; ESCX_UINT16_T first_child_index; ESCX_BYTE_T child_count;
 ESCX_BYTE_T initial_child; ESCX_BYTE_T depth; ESCX_BYTE_T document_order; ESCX_BYTE_T state_type;
 ESCX_UINT16_T eventless_selector; ESCX_BYTE_T done_event; ESCX_BYTE_T history_slot; ESCX_UINT16_T history_default_plan;
 ESCX_StatusStateFunction on_entry_status; ESCX_StatusStateFunction on_exit_status; ESCX_StatusStateFunction on_state_status;
 ESCX_StatusStateFunction data_init_status; ESCX_DoneDataFunction done_data;
 ESCX_StatusActionFunction initial_action; ESCX_StatusActionFunction history_default_action;
 ESCX_UINT16_T initial_entry_plan_encoded;
} ESCX_StateDescriptor;
typedef struct ESCX_HistoryStorage { ESCX_BYTE_T *valid; ESCX_BYTE_T *bits; ESCX_BYTE_T slot_count; ESCX_BYTE_T bytes_per_slot; } ESCX_HistoryStorage;
typedef struct ESCX_HistoryMap {
 const ESCX_UINT16_T *slot_first; const ESCX_BYTE_T *slot_count;
 const ESCX_BYTE_T *states; ESCX_UINT16_T state_count; ESCX_BYTE_T slot_count_total;
} ESCX_HistoryMap;
typedef struct ESCX_StatePlan { ESCX_UINT16_T first_state; ESCX_BYTE_T state_count; } ESCX_StatePlan;
typedef struct ESCX_Transition {
 ESCX_UINT16_T exit_plan; ESCX_UINT16_T entry_plan; ESCX_ActionFunction action;
 ESCX_BYTE_T type; ESCX_BYTE_T source_state; ESCX_BYTE_T source_depth; ESCX_BYTE_T document_order;
 ESCX_StatusActionFunction status_action;
} ESCX_Transition;
typedef enum ESCX_ErrorCode {
   ESCX_ERROR_NONE=0, ESCX_ERROR_EXECUTION=1, ESCX_ERROR_COMMUNICATION=2,
   ESCX_ERROR_PLATFORM=3
  } ESCX_ErrorCode;
  typedef enum ESCX_SendTarget {
   ESCX_SEND_TARGET_SELF=0, ESCX_SEND_TARGET_INTERNAL=1,
   ESCX_SEND_TARGET_PARENT=2, ESCX_SEND_TARGET_INVOKE=3
  } ESCX_SendTarget;
  typedef struct ESCX_SendRequest {
   ESCX_BYTE_T event_id; ESCX_BYTE_T target; ESCX_BYTE_T target_id;
   ESCX_UINT16_T send_id; unsigned long delay_ticks; void *event_data;
  } ESCX_SendRequest;
  typedef struct ESCX_TargetedSendRequest ESCX_TargetedSendRequest;
  typedef enum ESCX_ExecutableKind {
   ESCX_EXECUTABLE_GENERATED=0, ESCX_EXECUTABLE_RAISE=1,
   ESCX_EXECUTABLE_SEND=2, ESCX_EXECUTABLE_CANCEL=3,
   ESCX_EXECUTABLE_TARGETED_SEND=4
  } ESCX_ExecutableKind;
  typedef int (*ESCX_ExecutableFunction)(struct ESCX_StateMachine *, void *);
  typedef struct ESCX_ExecutableStep {
   ESCX_BYTE_T kind; ESCX_ExecutableFunction function; void *context;
   const ESCX_Event *event; const ESCX_SendRequest *send_request; ESCX_UINT16_T send_id;
   const ESCX_TargetedSendRequest *targeted_send_request;
  } ESCX_ExecutableStep;
  typedef struct ESCX_ExecutableBlock {
   const ESCX_ExecutableStep *steps; ESCX_UINT16_T step_count;
  } ESCX_ExecutableBlock;
  typedef enum ESCX_IOProcessorType { ESCX_IO_PROCESSOR_SCXML=0, ESCX_IO_PROCESSOR_INTERNAL=1, ESCX_IO_PROCESSOR_CUSTOM=2 } ESCX_IOProcessorType;
  typedef enum ESCX_IOTargetType { ESCX_IO_TARGET_CURRENT_SESSION=0, ESCX_IO_TARGET_INTERNAL=1, ESCX_IO_TARGET_PARENT_SESSION=2, ESCX_IO_TARGET_INVOKE=3, ESCX_IO_TARGET_SESSION=4, ESCX_IO_TARGET_CUSTOM=5 } ESCX_IOTargetType;
  typedef struct ESCX_SendTargetDescriptor { ESCX_BYTE_T processor; ESCX_BYTE_T target_type; ESCX_UINT16_T target_id; } ESCX_SendTargetDescriptor;
  typedef int (*ESCX_IOProcessorSendFunction)(struct ESCX_StateMachine *,const ESCX_SendTargetDescriptor *,const ESCX_Event *,void *);
  struct ESCX_TargetedSendRequest {
   ESCX_Event event; ESCX_SendTargetDescriptor target;
   unsigned long delay_ticks;
  };
  typedef struct ESCX_DelayedSend {
   ESCX_BOOL_T active; ESCX_BYTE_T event_id; ESCX_BYTE_T target; ESCX_BYTE_T target_id;
   ESCX_UINT16_T send_id; ESCX_TimerId timer_id; ESCX_BYTE_T event_type; ESCX_BYTE_T origin_type; ESCX_BYTE_T invoke_id; void *event_data;
 ESCX_BOOL_T uses_target_descriptor; ESCX_SendTargetDescriptor target_descriptor;
  } ESCX_DelayedSend;
  typedef enum ESCX_ProcessStatus {
 ESCX_STATUS_IGNORED=0, ESCX_STATUS_PROCESSED=1, ESCX_STATUS_INVALID_EVENT=2,
 ESCX_STATUS_NOT_RUNNING=3, ESCX_STATUS_QUEUE_FULL=4, ESCX_STATUS_INVALID_ARGUMENT=5,
 ESCX_STATUS_HIERARCHY_LIMIT=6, ESCX_STATUS_MICROSTEP_LIMIT=7,
   ESCX_STATUS_NO_PARENT=8, ESCX_STATUS_INVALID_TARGET=9, ESCX_STATUS_SEND_NOT_FOUND=10,
   ESCX_STATUS_EXECUTION_ERROR=11, ESCX_STATUS_COMMUNICATION_ERROR=12,
 ESCX_STATUS_TIMER_START_FAILED=13, ESCX_STATUS_TIMER_CANCEL_FAILED=14,
   ESCX_STATUS_CALLBACK_FAILED=15
} ESCX_ProcessStatus;
typedef struct ESCX_MacrostepResult { ESCX_BOOL_T event_accepted; ESCX_BOOL_T transition_taken; ESCX_UINT16_T microstep_count; } ESCX_MacrostepResult;
typedef struct ESCX_ErrorEventData { ESCX_BYTE_T code; ESCX_ProcessStatus cause; ESCX_Event triggering_event; } ESCX_ErrorEventData;
typedef struct ESCX_InvokeDescriptor {
 ESCX_BYTE_T owner_state; ESCX_BYTE_T done_event; ESCX_BOOL_T autoforward;
 void *child_storage; ESCX_InvokeStartFunction start; ESCX_InvokeCancelFunction cancel;
 ESCX_InvokeForwardFunction forward; ESCX_InvokeFinalizeFunction finalize; ESCX_DoneDataFunction done_data;
 ESCX_InvokeFinalizeStatusFunction finalize_status; ESCX_InvokeInitializeFunction initialize; ESCX_InvokeForwardEventFunction forward_event;
} ESCX_InvokeDescriptor;
typedef struct ESCX_InvokeInstance { ESCX_BOOL_T active; } ESCX_InvokeInstance;
typedef struct ESCX_InvokeMap {
 const ESCX_UINT16_T *state_first; const ESCX_BYTE_T *state_count;
 ESCX_UINT16_T state_count_total; ESCX_BYTE_T invoke_count_total;
} ESCX_InvokeMap;
typedef struct ESCX_ExitOrder {
 const ESCX_BYTE_T *states; ESCX_UINT16_T state_count;
} ESCX_ExitOrder;
typedef struct ESCX_StateMachineDefinition {
 const ESCX_StateDescriptor *states; const ESCX_BYTE_T *children;
 const ESCX_Transition *transitions; const ESCX_StatePlan *plans; const ESCX_BYTE_T *plan_states;
 const ESCX_SelectorFunction *selectors; const ESCX_UINT16_T *state_base;
 const ESCX_BYTE_T *slot_owner; const ESCX_UINT16_T *slot_selector;
 ESCX_UINT16_T state_count, child_count, transition_count, plan_count, plan_state_count;
 ESCX_UINT16_T selector_count, slot_count; ESCX_BYTE_T initial_state, event_count;
} ESCX_StateMachineDefinition;
typedef struct ESCX_MicrostepWorkspace {
 ESCX_BYTE_T *exit_states; ESCX_BYTE_T exit_capacity; ESCX_BYTE_T exit_count;
 ESCX_BYTE_T *entry_states; ESCX_BYTE_T entry_capacity; ESCX_BYTE_T entry_count;
 ESCX_BYTE_T *exit_seen; ESCX_BYTE_T *entry_seen; ESCX_BYTE_T seen_byte_count;
} ESCX_MicrostepWorkspace;
typedef struct ESCX_FastDispatch {
 const ESCX_UINT16_T *state_base; const ESCX_BYTE_T *slot_owner;
 const ESCX_UINT16_T *slot_selector; ESCX_UINT16_T slot_count;
 const ESCX_UINT16_T *eventless_selector; ESCX_UINT16_T eventless_count;
} ESCX_FastDispatch;
typedef struct ESCX_ConflictMap {
 const ESCX_BYTE_T *bits; ESCX_UINT16_T transition_count; ESCX_UINT16_T bytes_per_row;
} ESCX_ConflictMap;
typedef struct ESCX_StateMachine {
 const ESCX_StateMachineDefinition *definition; void *instance;
 ESCX_BYTE_T *active_bits; ESCX_BYTE_T active_byte_count;
 ESCX_BYTE_T *completion_bits; ESCX_BYTE_T completion_byte_count;
 const ESCX_FastDispatch *fast_dispatch; const ESCX_ConflictMap *conflict_map; ESCX_MicrostepWorkspace *workspace;
 ESCX_BYTE_T active_atomic_states[ESCX_MAX_ACTIVE_ATOMIC_STATES]; ESCX_BYTE_T active_atomic_count;
 ESCX_BYTE_T current_event; ESCX_Event current_event_descriptor; ESCX_Event triggering_event_descriptor; ESCX_BOOL_T has_triggering_event;
 ESCX_SystemContext system_context;
 ESCX_IOProcessorSendFunction io_processor_send; void *io_processor_context;
   const ESCX_BYTE_T *event_parent; ESCX_BYTE_T event_parent_count; ESCX_BYTE_T wildcard_event; ESCX_BYTE_T error_execution_event; ESCX_BYTE_T error_communication_event;
   ESCX_BYTE_T last_error; ESCX_ProcessStatus last_error_status; ESCX_MacrostepResult last_macrostep_result; ESCX_ErrorEventData error_event_data; ESCX_ErrorEventData pending_error_event_data; ESCX_BOOL_T reporting_error; ESCX_BOOL_T running; ESCX_BOOL_T processing; ESCX_BOOL_T finished; ESCX_BOOL_T finish_pending;
 ESCX_HistoryStorage *history; const ESCX_HistoryMap *history_map;
   ESCX_BYTE_T data_binding; ESCX_BYTE_T *data_initialized_bits; ESCX_BYTE_T data_initialized_byte_count;
 const ESCX_InvokeDescriptor *invokes; ESCX_InvokeInstance *invoke_instances; ESCX_BYTE_T invoke_count; const ESCX_InvokeMap *invoke_map; const ESCX_ExitOrder *exit_order; ESCX_BOOL_T pending_invokes[ESCX_MAX_INVOKES];
   struct ESCX_StateMachine *parent_machine; ESCX_BYTE_T parent_invoke_id;
#if ESCX_ENABLE_DELAYED_SENDS
   ESCX_DelayedSend delayed_sends[ESCX_MAX_DELAYED_SENDS];
#endif
#if ESCX_ENABLE_TIMERS
 ESCX_StartTimerFunction start_timer; ESCX_CancelTimerFunction cancel_timer; void *timer_context;
#endif
 ESCX_EventQueue internal_queue;
 ESCX_EventQueue external_queue;
} ESCX_StateMachine;

void ESCX_StateMachineInit(ESCX_StateMachine *,const ESCX_StateMachineDefinition *,void *,ESCX_BYTE_T *,ESCX_BYTE_T,ESCX_HistoryStorage *,ESCX_StartTimerFunction,ESCX_CancelTimerFunction);
ESCX_ProcessStatus ESCX_StateMachineStart(ESCX_StateMachine *);
void ESCX_StateMachineStop(ESCX_StateMachine *);
ESCX_ProcessStatus ESCX_StateMachineSend(ESCX_StateMachine *,ESCX_BYTE_T,void *);
  ESCX_ProcessStatus ESCX_StateMachineSendEvent(ESCX_StateMachine *,const ESCX_Event *);
  const ESCX_MacrostepResult *ESCX_StateMachineGetLastMacrostepResult(const ESCX_StateMachine *);
ESCX_ProcessStatus ESCX_StateMachineRaiseInternal(ESCX_StateMachine *,ESCX_BYTE_T,void *);
  ESCX_ProcessStatus ESCX_StateMachineRaiseEvent(ESCX_StateMachine *,const ESCX_Event *);
  const ESCX_Event *ESCX_StateMachineGetCurrentEvent(const ESCX_StateMachine *);
ESCX_ProcessStatus ESCX_StateMachineConfigureSystemContext(ESCX_StateMachine *,const ESCX_SystemContext *);
 const ESCX_SystemContext *ESCX_StateMachineGetSystemContext(const ESCX_StateMachine *);
  ESCX_ProcessStatus ESCX_StateMachineConfigureEventMatching(ESCX_StateMachine *,const ESCX_BYTE_T *,ESCX_BYTE_T,ESCX_BYTE_T);
  ESCX_ProcessStatus ESCX_StateMachineConfigureDataBinding(ESCX_StateMachine *,ESCX_DataBinding,ESCX_BYTE_T *,ESCX_BYTE_T);
 ESCX_ProcessStatus ESCX_StateMachineConfigureHistoryMap(ESCX_StateMachine *,const ESCX_HistoryMap *);
 ESCX_ProcessStatus ESCX_StateMachineConfigureCompletionStorage(ESCX_StateMachine *,ESCX_BYTE_T *,ESCX_BYTE_T);
 ESCX_ProcessStatus ESCX_StateMachineConfigureFastDispatch(ESCX_StateMachine *,const ESCX_FastDispatch *);
 ESCX_ProcessStatus ESCX_StateMachineConfigureConflictMap(ESCX_StateMachine *,const ESCX_ConflictMap *);
 ESCX_ProcessStatus ESCX_StateMachineConfigureWorkspace(ESCX_StateMachine *,ESCX_MicrostepWorkspace *);
  ESCX_BOOL_T ESCX_StateMachineIsDataInitialized(const ESCX_StateMachine *,ESCX_BYTE_T);
ESCX_ProcessStatus ESCX_StateMachineTimerExpired(ESCX_StateMachine *,ESCX_TimerId);
ESCX_ProcessStatus ESCX_StateMachineConfigureTimers(ESCX_StateMachine *,ESCX_StartTimerFunction,ESCX_CancelTimerFunction,void *);
ESCX_BOOL_T ESCX_StateMachineIsInState(const ESCX_StateMachine *,ESCX_BYTE_T);
ESCX_BOOL_T ESCX_StateMachineIsFinished(const ESCX_StateMachine *);
 ESCX_BOOL_T ESCX_StateMachineIsRunning(const ESCX_StateMachine *);
ESCX_ProcessStatus ESCX_StateMachineConfigureInvokes(ESCX_StateMachine *,const ESCX_InvokeDescriptor *,ESCX_InvokeInstance *,ESCX_BYTE_T);
  ESCX_ProcessStatus ESCX_StateMachineConfigureInvokeMap(ESCX_StateMachine *,const ESCX_InvokeMap *);
  ESCX_ProcessStatus ESCX_StateMachineConfigureExitOrder(ESCX_StateMachine *,const ESCX_ExitOrder *);
ESCX_ProcessStatus ESCX_StateMachineCompleteInvoke(ESCX_StateMachine *,ESCX_BYTE_T,void *);
  ESCX_ProcessStatus ESCX_StateMachineCompleteInvokeEvent(ESCX_StateMachine *,ESCX_BYTE_T,const ESCX_Event *);
ESCX_ProcessStatus ESCX_StateMachineReceiveInvokeEvent(ESCX_StateMachine *,ESCX_BYTE_T,ESCX_BYTE_T,void *);
  ESCX_ProcessStatus ESCX_StateMachineReceiveInvokeEventDescriptor(ESCX_StateMachine *,ESCX_BYTE_T,const ESCX_Event *);
  ESCX_ProcessStatus ESCX_StateMachineConfigureParent(ESCX_StateMachine *,ESCX_StateMachine *,ESCX_BYTE_T);
  ESCX_ProcessStatus ESCX_StateMachineSendRequest(ESCX_StateMachine *,const ESCX_SendRequest *);
  ESCX_ProcessStatus ESCX_StateMachineConfigureIOProcessor(ESCX_StateMachine *,ESCX_IOProcessorSendFunction,void *);
  ESCX_ProcessStatus ESCX_StateMachineSendToTarget(ESCX_StateMachine *,const ESCX_SendTargetDescriptor *,const ESCX_Event *);
  ESCX_ProcessStatus ESCX_StateMachineSendTargetedRequest(ESCX_StateMachine *,const ESCX_TargetedSendRequest *);
  ESCX_ProcessStatus ESCX_StateMachineCancelSend(ESCX_StateMachine *,ESCX_UINT16_T);
  ESCX_ProcessStatus ESCX_StateMachineExecuteContent(ESCX_StateMachine *,const ESCX_ExecutableBlock *);
  ESCX_ProcessStatus ESCX_StateMachineConfigureErrorEvents(ESCX_StateMachine *,ESCX_BYTE_T,ESCX_BYTE_T);
  ESCX_BYTE_T ESCX_StateMachineGetLastError(const ESCX_StateMachine *);
  ESCX_ProcessStatus ESCX_StateMachineGetLastErrorStatus(const ESCX_StateMachine *);
  const ESCX_ErrorEventData *ESCX_StateMachineGetLastErrorData(const ESCX_StateMachine *);
  void ESCX_StateMachineClearError(ESCX_StateMachine *);
  ESCX_ProcessStatus ESCX_StateMachineReportError(ESCX_StateMachine *,ESCX_ErrorCode,ESCX_ProcessStatus);

#define ESCX_SEND_TARGET_SCXML_CURRENT {ESCX_IO_PROCESSOR_SCXML,ESCX_IO_TARGET_CURRENT_SESSION,0U}
#define ESCX_SEND_TARGET_INTERNAL_QUEUE {ESCX_IO_PROCESSOR_INTERNAL,ESCX_IO_TARGET_INTERNAL,0U}
#define ESCX_SEND_TARGET_SCXML_PARENT {ESCX_IO_PROCESSOR_SCXML,ESCX_IO_TARGET_PARENT_SESSION,0U}
#define ESCX_SEND_TARGET_SCXML_INVOKE(id) {ESCX_IO_PROCESSOR_SCXML,ESCX_IO_TARGET_INVOKE,(ESCX_UINT16_T)(id)}
#define ESCX_SEND_TARGET_SCXML_SESSION(id) {ESCX_IO_PROCESSOR_SCXML,ESCX_IO_TARGET_SESSION,(ESCX_UINT16_T)(id)}
#define ESCX_SEND_TARGET_CUSTOM(processor_id,target_id) {(ESCX_BYTE_T)(processor_id),ESCX_IO_TARGET_CUSTOM,(ESCX_UINT16_T)(target_id)}
#define ESCX_EXECUTABLE_STEPS_BEGIN(n) static const ESCX_ExecutableStep n##_executable_steps[] ESCX_ROM={
#define ESCX_EXECUTABLE_GENERATED_STEP(function,context) {ESCX_EXECUTABLE_GENERATED,function,context,(const ESCX_Event*)0,(const ESCX_SendRequest*)0,ESCX_NO_SEND_ID,(const ESCX_TargetedSendRequest*)0},
#define ESCX_EXECUTABLE_RAISE_STEP(event_ptr) {ESCX_EXECUTABLE_RAISE,(ESCX_ExecutableFunction)0,(void*)0,event_ptr,(const ESCX_SendRequest*)0,ESCX_NO_SEND_ID,(const ESCX_TargetedSendRequest*)0},
#define ESCX_EXECUTABLE_SEND_STEP(request_ptr) {ESCX_EXECUTABLE_SEND,(ESCX_ExecutableFunction)0,(void*)0,(const ESCX_Event*)0,request_ptr,ESCX_NO_SEND_ID,(const ESCX_TargetedSendRequest*)0},
#define ESCX_EXECUTABLE_TARGETED_SEND_STEP(request_ptr) {ESCX_EXECUTABLE_TARGETED_SEND,(ESCX_ExecutableFunction)0,(void*)0,(const ESCX_Event*)0,(const ESCX_SendRequest*)0,ESCX_NO_SEND_ID,request_ptr},
#define ESCX_EXECUTABLE_CANCEL_STEP(send_id_value) {ESCX_EXECUTABLE_CANCEL,(ESCX_ExecutableFunction)0,(void*)0,(const ESCX_Event*)0,(const ESCX_SendRequest*)0,(ESCX_UINT16_T)(send_id_value),(const ESCX_TargetedSendRequest*)0},
#define ESCX_EXECUTABLE_STEPS_END() };
#define ESCX_DEFINE_EXECUTABLE_BLOCK(n) static const ESCX_ExecutableBlock n##_executable_block={n##_executable_steps,(ESCX_UINT16_T)(sizeof(n##_executable_steps)/sizeof(n##_executable_steps[0]))}
#define ESCX_EXIT_ORDER_BEGIN(n) static const ESCX_BYTE_T n##_exit_order_states[] ESCX_ROM={
#define ESCX_EXIT_ORDER_STATE(v) (ESCX_BYTE_T)(v),
#define ESCX_EXIT_ORDER_END() };
#define ESCX_DEFINE_EXIT_ORDER(n) static const ESCX_ExitOrder n##_exit_order={n##_exit_order_states,(ESCX_UINT16_T)(sizeof(n##_exit_order_states)/sizeof(n##_exit_order_states[0]))}
#define ESCX_INVOKE_STATE_FIRST_BEGIN(n) static const ESCX_UINT16_T n##_invoke_state_first[] ESCX_ROM={
#define ESCX_INVOKE_STATE_FIRST(v) (ESCX_UINT16_T)(v),
#define ESCX_INVOKE_STATE_FIRST_END() };
#define ESCX_INVOKE_STATE_COUNT_BEGIN(n) static const ESCX_BYTE_T n##_invoke_state_count[] ESCX_ROM={
#define ESCX_INVOKE_STATE_COUNT(v) (ESCX_BYTE_T)(v),
#define ESCX_INVOKE_STATE_COUNT_END() };
#define ESCX_DEFINE_INVOKE_MAP(n,invoke_count) static const ESCX_InvokeMap n##_invoke_map={n##_invoke_state_first,n##_invoke_state_count,(ESCX_UINT16_T)(sizeof(n##_invoke_state_first)/sizeof(n##_invoke_state_first[0])),(ESCX_BYTE_T)(invoke_count)}
#define ESCX_INVOKES_BEGIN(n) static const ESCX_InvokeDescriptor n##_invokes[] ESCX_ROM = {
#define ESCX_INVOKE(owner,done_event,autoforward,storage,start,cancel,forward,finalize) {(ESCX_BYTE_T)(owner),(ESCX_BYTE_T)(done_event),(ESCX_BOOL_T)(autoforward),storage,start,cancel,forward,finalize,(ESCX_DoneDataFunction)0,(ESCX_InvokeFinalizeStatusFunction)0,(ESCX_InvokeInitializeFunction)0,(ESCX_InvokeForwardEventFunction)0},
  #define ESCX_INVOKE_DONE_DATA(owner,done_event,autoforward,storage,start,cancel,forward,finalize,done_data) {(ESCX_BYTE_T)(owner),(ESCX_BYTE_T)(done_event),(ESCX_BOOL_T)(autoforward),storage,start,cancel,forward,finalize,done_data,(ESCX_InvokeFinalizeStatusFunction)0,(ESCX_InvokeInitializeFunction)0,(ESCX_InvokeForwardEventFunction)0},
#define ESCX_INVOKE_STATUS(owner,done_event,autoforward,storage,start,cancel,forward,finalize_status,done_data) {(ESCX_BYTE_T)(owner),(ESCX_BYTE_T)(done_event),(ESCX_BOOL_T)(autoforward),storage,start,cancel,forward,(ESCX_InvokeFinalizeFunction)0,done_data,finalize_status,(ESCX_InvokeInitializeFunction)0,(ESCX_InvokeForwardEventFunction)0},
#define ESCX_INVOKE_INITIALIZED(owner,done_event,autoforward,storage,initialize,start,cancel,forward,finalize_status,done_data) {(ESCX_BYTE_T)(owner),(ESCX_BYTE_T)(done_event),(ESCX_BOOL_T)(autoforward),storage,start,cancel,forward,(ESCX_InvokeFinalizeFunction)0,done_data,finalize_status,initialize,(ESCX_InvokeForwardEventFunction)0},
#define ESCX_INVOKE_EVENT_FORWARD(owner,done_event,autoforward,storage,initialize,start,cancel,forward_event,finalize_status,done_data) {(ESCX_BYTE_T)(owner),(ESCX_BYTE_T)(done_event),(ESCX_BOOL_T)(autoforward),storage,start,cancel,(ESCX_InvokeForwardFunction)0,(ESCX_InvokeFinalizeFunction)0,done_data,finalize_status,initialize,forward_event},
#define ESCX_INVOKES_END() };
#define ESCX_STATE_ENUM_BEGIN(n) typedef enum n##_StateId { n##_NO_STATE=0,
#define ESCX_STATE_ENUM(v) v,
#define ESCX_STATE_ENUM_END(n) n##_STATE_COUNT } n##_StateId;
#define ESCX_EVENT_ENUM_BEGIN(n) typedef enum n##_EventId { n##_TIMER_EVENT=0,
#define ESCX_EVENT_ENUM(v) v,
#define ESCX_EVENT_ENUM_END(n) n##_EVENT_COUNT } n##_EventId;
  #define ESCX_EVENT_PARENTS_BEGIN(n) static const ESCX_BYTE_T n##_event_parents[] ESCX_ROM={
  #define ESCX_EVENT_PARENT(v) (ESCX_BYTE_T)(v),
  #define ESCX_EVENT_PARENTS_END() };
#define ESCX_STATES_BEGIN(n) static const ESCX_StateDescriptor n##_states[] ESCX_ROM={
#define ESCX_STATE_EX_HISTORY(en,ex,run,parent,first,count,initial,depth,order,type,eventless,done_event,history_slot,history_default_plan) {en,ex,run,(ESCX_BYTE_T)(parent),(ESCX_UINT16_T)(first),(ESCX_BYTE_T)(count),(ESCX_BYTE_T)(initial),(ESCX_BYTE_T)(depth),(ESCX_BYTE_T)(order),(ESCX_BYTE_T)(type),(ESCX_UINT16_T)(eventless),(ESCX_BYTE_T)(done_event),(ESCX_BYTE_T)(history_slot),(ESCX_UINT16_T)(history_default_plan)},
#define ESCX_STATE_EX_DONE(en,ex,run,parent,first,count,initial,depth,order,type,eventless,done_event) ESCX_STATE_EX_HISTORY(en,ex,run,parent,first,count,initial,depth,order,type,eventless,done_event,ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN)
#define ESCX_HISTORY_STATE(parent,depth,order,type,slot,default_plan) ESCX_STATE_EX_HISTORY((ESCX_StateFunction)0,(ESCX_StateFunction)0,(ESCX_StateFunction)0,parent,0U,0U,ESCX_NO_STATE,depth,order,type,ESCX_NO_SELECTOR,ESCX_NO_EVENT,slot,default_plan)
#define ESCX_STATE_EX(en,ex,run,parent,first,count,initial,depth,order,type,eventless) ESCX_STATE_EX_DONE(en,ex,run,parent,first,count,initial,depth,order,type,eventless,ESCX_NO_EVENT)
#define ESCX_STATE(en,ex,run) ESCX_STATE_EX(en,ex,run,ESCX_NO_STATE,0U,0U,ESCX_NO_STATE,0U,0U,ESCX_STATE_ATOMIC,ESCX_NO_SELECTOR)
#define ESCX_STATE_INITIAL_EX(en,ex,run,initial_action,parent,first,count,initial,depth,order,type,eventless,done_event) {en,ex,run,(ESCX_BYTE_T)(parent),(ESCX_UINT16_T)(first),(ESCX_BYTE_T)(count),(ESCX_BYTE_T)(initial),(ESCX_BYTE_T)(depth),(ESCX_BYTE_T)(order),(ESCX_BYTE_T)(type),(ESCX_UINT16_T)(eventless),(ESCX_BYTE_T)(done_event),ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,(ESCX_StatusStateFunction)0,(ESCX_StatusStateFunction)0,(ESCX_StatusStateFunction)0,(ESCX_StatusStateFunction)0,(ESCX_DoneDataFunction)0,initial_action,(ESCX_StatusActionFunction)0,ESCX_NO_PLAN},
#define ESCX_HISTORY_STATE_ACTION(parent,depth,order,type,slot,default_plan,default_action) {(ESCX_StateFunction)0,(ESCX_StateFunction)0,(ESCX_StateFunction)0,(ESCX_BYTE_T)(parent),0U,0U,ESCX_NO_STATE,(ESCX_BYTE_T)(depth),(ESCX_BYTE_T)(order),(ESCX_BYTE_T)(type),ESCX_NO_SELECTOR,ESCX_NO_EVENT,(ESCX_BYTE_T)(slot),(ESCX_UINT16_T)(default_plan),(ESCX_StatusStateFunction)0,(ESCX_StatusStateFunction)0,(ESCX_StatusStateFunction)0,(ESCX_StatusStateFunction)0,(ESCX_DoneDataFunction)0,(ESCX_StatusActionFunction)0,default_action,ESCX_NO_PLAN},
#define ESCX_STATE_INITIAL_PLAN_EX(en,ex,run,initial_action,initial_plan,parent,first,count,depth,order,type,eventless,done_event) {en,ex,run,(ESCX_BYTE_T)(parent),(ESCX_UINT16_T)(first),(ESCX_BYTE_T)(count),ESCX_NO_STATE,(ESCX_BYTE_T)(depth),(ESCX_BYTE_T)(order),(ESCX_BYTE_T)(type),(ESCX_UINT16_T)(eventless),(ESCX_BYTE_T)(done_event),ESCX_NO_HISTORY_SLOT,ESCX_NO_PLAN,(ESCX_StatusStateFunction)0,(ESCX_StatusStateFunction)0,(ESCX_StatusStateFunction)0,(ESCX_StatusStateFunction)0,(ESCX_DoneDataFunction)0,initial_action,(ESCX_StatusActionFunction)0,(ESCX_UINT16_T)((initial_plan)+1U)},
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
#define ESCX_TRANSITION_EX(exit_plan,entry_plan,action,type,source,depth,order) {(ESCX_UINT16_T)(exit_plan),(ESCX_UINT16_T)(entry_plan),action,(ESCX_BYTE_T)(type),(ESCX_BYTE_T)(source),(ESCX_BYTE_T)(depth),(ESCX_BYTE_T)(order)},
#define ESCX_TRANSITION(exit_plan,entry_plan,action) ESCX_TRANSITION_EX(exit_plan,entry_plan,action,ESCX_TRANSITION_EXTERNAL,ESCX_NO_STATE,0U,0U)
#define ESCX_TRANSITION_META(exit_plan,entry_plan,action,type,source,depth,order) ESCX_TRANSITION_EX(exit_plan,entry_plan,action,type,source,depth,order)
#define ESCX_TARGETLESS_TRANSITION(action) ESCX_TRANSITION_EX(ESCX_NO_PLAN,ESCX_NO_PLAN,action,ESCX_TRANSITION_TARGETLESS,ESCX_NO_STATE,0U,0U)
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
#define ESCX_DECLARE_MACHINE(n) void n##_Init(ESCX_StateMachine *,void *,ESCX_BYTE_T *,ESCX_BYTE_T,ESCX_HistoryStorage *,ESCX_StartTimerFunction,ESCX_CancelTimerFunction)
#define ESCX_IMPLEMENT_MACHINE(n) void n##_Init(ESCX_StateMachine*m,void*i,ESCX_BYTE_T*b,ESCX_BYTE_T z,ESCX_HistoryStorage*h,ESCX_StartTimerFunction s,ESCX_CancelTimerFunction c){ESCX_StateMachineInit(m,&n##_definition,i,b,z,h,s,c);}

#define ESCX_FAST_STATE_BASE_BEGIN(n) static const ESCX_UINT16_T n##_fast_state_base[] ESCX_ROM={
#define ESCX_FAST_STATE_BASE_END() };
#define ESCX_FAST_SLOT_OWNER_BEGIN(n) static const ESCX_BYTE_T n##_fast_slot_owner[] ESCX_ROM={
#define ESCX_FAST_SLOT_OWNER_END() };
#define ESCX_FAST_SLOT_SELECTOR_BEGIN(n) static const ESCX_UINT16_T n##_fast_slot_selector[] ESCX_ROM={
#define ESCX_FAST_SLOT_SELECTOR_END() };
#define ESCX_DEFINE_FAST_DISPATCH(n) static const ESCX_FastDispatch n##_fast_dispatch={n##_fast_state_base,n##_fast_slot_owner,n##_fast_slot_selector,(ESCX_UINT16_T)(sizeof(n##_fast_slot_owner)/sizeof(n##_fast_slot_owner[0])),(const ESCX_UINT16_T*)0,0U}
#define ESCX_FAST_EVENTLESS_BEGIN(n) static const ESCX_UINT16_T n##_fast_eventless_selector[] ESCX_ROM={
#define ESCX_FAST_EVENTLESS_SELECTOR(v) (ESCX_UINT16_T)(v),
#define ESCX_FAST_EVENTLESS_END() };
#define ESCX_DEFINE_FAST_DISPATCH_EVENTLESS(n) static const ESCX_FastDispatch n##_fast_dispatch={n##_fast_state_base,n##_fast_slot_owner,n##_fast_slot_selector,(ESCX_UINT16_T)(sizeof(n##_fast_slot_owner)/sizeof(n##_fast_slot_owner[0])),n##_fast_eventless_selector,(ESCX_UINT16_T)(sizeof(n##_fast_eventless_selector)/sizeof(n##_fast_eventless_selector[0]))}
#define ESCX_HISTORY_SLOT_FIRST_BEGIN(n) static const ESCX_UINT16_T n##_history_slot_first[] ESCX_ROM={
#define ESCX_HISTORY_SLOT_FIRST(v) (ESCX_UINT16_T)(v),
#define ESCX_HISTORY_SLOT_FIRST_END() };
#define ESCX_HISTORY_SLOT_COUNT_BEGIN(n) static const ESCX_BYTE_T n##_history_slot_count[] ESCX_ROM={
#define ESCX_HISTORY_SLOT_COUNT(v) (ESCX_BYTE_T)(v),
#define ESCX_HISTORY_SLOT_COUNT_END() };
#define ESCX_HISTORY_STATES_BEGIN(n) static const ESCX_BYTE_T n##_history_states[] ESCX_ROM={
#define ESCX_HISTORY_MAP_STATE(v) (ESCX_BYTE_T)(v),
#define ESCX_HISTORY_STATES_END() };
#define ESCX_DEFINE_HISTORY_MAP(n) static const ESCX_HistoryMap n##_history_map={n##_history_slot_first,n##_history_slot_count,n##_history_states,(ESCX_UINT16_T)(sizeof(n##_history_states)/sizeof(n##_history_states[0])),(ESCX_BYTE_T)(sizeof(n##_history_slot_first)/sizeof(n##_history_slot_first[0]))}
#define ESCX_DECLARE_WORKSPACE(n,exit_capacity,entry_capacity,state_count) \
 static ESCX_BYTE_T n##_exit_states[(exit_capacity)]; \
 static ESCX_BYTE_T n##_entry_states[(entry_capacity)]; \
 static ESCX_BYTE_T n##_exit_seen[ESCX_ACTIVE_BYTES(state_count)]; \
 static ESCX_BYTE_T n##_entry_seen[ESCX_ACTIVE_BYTES(state_count)]; \
 static ESCX_MicrostepWorkspace n##_workspace={n##_exit_states,(ESCX_BYTE_T)(exit_capacity),0U,n##_entry_states,(ESCX_BYTE_T)(entry_capacity),0U,n##_exit_seen,n##_entry_seen,(ESCX_BYTE_T)ESCX_ACTIVE_BYTES(state_count)}

#define ESCX_CONFLICT_BYTES(count) (((count)+7U)/8U)
#define ESCX_CONFLICT_BITS_BEGIN(n) static const ESCX_BYTE_T n##_conflict_bits[] ESCX_ROM={
#define ESCX_CONFLICT_BYTE(v) (ESCX_BYTE_T)(v),
#define ESCX_CONFLICT_BITS_END() };
#define ESCX_DEFINE_CONFLICT_MAP(n,transition_count) static const ESCX_ConflictMap n##_conflict_map={n##_conflict_bits,(ESCX_UINT16_T)(transition_count),(ESCX_UINT16_T)ESCX_CONFLICT_BYTES(transition_count)}
#ifdef __cplusplus
}
#endif
#endif
