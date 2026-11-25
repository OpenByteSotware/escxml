#include "StateMachine.h"
#include "AssertHandler.h"
#include <stddef.h>
#include <string.h>
#define OFFSETOF_STATE(member) ((unsigned int)offsetof(ESCX_StateDescriptor,member))
static ESCX_StateFunction ReadStateFn(const ESCX_StateFunction* p)
{
#if defined(ESCX_PLATFORM_AVR)&&(ESCX_PLATFORM_AVR!=0)
	ESCX_StateFunction r; (void)ESCX_ROM_COPY(&r, p, sizeof(r)); return r;
#else
	return *p;
#endif
}
static ESCX_ActionFunction ReadAction(const ESCX_ActionFunction* p)
{
#if defined(ESCX_PLATFORM_AVR)&&(ESCX_PLATFORM_AVR!=0)
	ESCX_ActionFunction r; (void)ESCX_ROM_COPY(&r, p, sizeof(r)); return r;
#else
	return *p;
#endif
}
static void ReadExecutableStep(const ESCX_ExecutableStep* p, ESCX_ExecutableStep* out)
{
	(void)ESCX_ROM_COPY(out, p, sizeof(*out));
}

static ESCX_StatusActionFunction ReadStatusAction(const ESCX_StatusActionFunction* p)
{
#if defined(ESCX_PLATFORM_AVR)&&(ESCX_PLATFORM_AVR!=0)
	ESCX_StatusActionFunction r; (void)ESCX_ROM_COPY(&r, p, sizeof(r)); return r;
#else
	return *p;
#endif
}
static ESCX_StatusStateFunction ReadStatusState(const ESCX_StatusStateFunction* p)
{
#if defined(ESCX_PLATFORM_AVR)&&(ESCX_PLATFORM_AVR!=0)
	ESCX_StatusStateFunction r; (void)ESCX_ROM_COPY(&r, p, sizeof(r)); return r;
#else
	return *p;
#endif
}
static ESCX_DoneDataFunction ReadDoneData(const ESCX_DoneDataFunction* p)
{
#if defined(ESCX_PLATFORM_AVR)&&(ESCX_PLATFORM_AVR!=0)
	ESCX_DoneDataFunction r; (void)ESCX_ROM_COPY(&r, p, sizeof(r)); return r;
#else
	return *p;
#endif
}
static ESCX_InvokeFinalizeStatusFunction ReadInvokeFinalizeStatus(const ESCX_InvokeFinalizeStatusFunction* p)
{
#if defined(ESCX_PLATFORM_AVR)&&(ESCX_PLATFORM_AVR!=0)
	ESCX_InvokeFinalizeStatusFunction r; (void)ESCX_ROM_COPY(&r, p, sizeof(r)); return r;
#else
	return *p;
#endif
}
static ESCX_InvokeInitializeFunction ReadInvokeInitialize(const ESCX_InvokeInitializeFunction* p)
{
#if defined(ESCX_PLATFORM_AVR)&&(ESCX_PLATFORM_AVR!=0)
	ESCX_InvokeInitializeFunction r; (void)ESCX_ROM_COPY(&r, p, sizeof(r)); return r;
#else
	return *p;
#endif
}
static ESCX_SelectorFunction ReadSelector(const ESCX_SelectorFunction* p)
{
#if defined(ESCX_PLATFORM_AVR)&&(ESCX_PLATFORM_AVR!=0)
	ESCX_SelectorFunction r; (void)ESCX_ROM_COPY(&r, p, sizeof(r)); return r;
#else
	return *p;
#endif
}
static ESCX_BYTE_T StateU8(const ESCX_StateMachine* m, ESCX_BYTE_T id, unsigned int off)
{
	return ESCX_ROM_READ_U8(((const ESCX_BYTE_T*)&m->definition->states[id]) + off);
}
static ESCX_UINT16_T StateU16(const ESCX_StateMachine* m, ESCX_BYTE_T id, unsigned int off)
{
	return ESCX_ROM_READ_U16((const ESCX_UINT16_T*)(((const ESCX_BYTE_T*)&m->definition->states[id]) + off));
}
static ESCX_BYTE_T Parent(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	return StateU8(m, s, OFFSETOF_STATE(parent_state));
}
static ESCX_BYTE_T Initial(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	return StateU8(m, s, OFFSETOF_STATE(initial_child));
}
static ESCX_BYTE_T Type(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	return StateU8(m, s, OFFSETOF_STATE(state_type));
}
static ESCX_BYTE_T StateDepth(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	return StateU8(m, s, OFFSETOF_STATE(depth));
}
static ESCX_BYTE_T StateOrder(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	return StateU8(m, s, OFFSETOF_STATE(document_order));
}
static ESCX_BYTE_T HistorySlot(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	return StateU8(m, s, OFFSETOF_STATE(history_slot));
}
static ESCX_UINT16_T HistoryDefaultPlan(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	return StateU16(m, s, OFFSETOF_STATE(history_default_plan));
}
static ESCX_BYTE_T DoneEvent(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	return StateU8(m, s, OFFSETOF_STATE(done_event));
}
static ESCX_UINT16_T EventlessSelector(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	return StateU16(m, s, OFFSETOF_STATE(eventless_selector));
}
static ESCX_UINT16_T InitialEntryPlan(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	ESCX_UINT16_T encoded; encoded = StateU16(m, s, OFFSETOF_STATE(initial_entry_plan_encoded));
	return(encoded == 0U) ? ESCX_NO_PLAN : (ESCX_UINT16_T)(encoded - 1U);
}
static void SetActive(ESCX_StateMachine* m, ESCX_BYTE_T s, ESCX_BOOL_T on)
{
	ESCX_BYTE_T i, mask; i = (ESCX_BYTE_T)(s >> 3); mask = (ESCX_BYTE_T)(1U << (s & 7U));
	if (i >= m->active_byte_count)return;
	if (on)m->active_bits[i] = (ESCX_BYTE_T)(m->active_bits[i] | mask);
	else m->active_bits[i] = (ESCX_BYTE_T)(m->active_bits[i] & ((ESCX_BYTE_T)~mask));
}
static ESCX_BOOL_T IsActive(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	ESCX_BYTE_T i, mask; i = (ESCX_BYTE_T)(s >> 3);
	if (i >= m->active_byte_count)return ESCX_FALSE; mask = (ESCX_BYTE_T)(1U << (s & 7U));
	return((m->active_bits[i] & mask) != 0U) ? ESCX_TRUE : ESCX_FALSE;
}
static ESCX_BOOL_T CompletionWasEmitted(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	ESCX_BYTE_T i, mask; if (m->completion_bits == (ESCX_BYTE_T*)0)return ESCX_FALSE;
	i = (ESCX_BYTE_T)(s >> 3);
	if (i >= m->completion_byte_count)return ESCX_FALSE; mask = (ESCX_BYTE_T)(1U << (s & 7U)); return((m->completion_bits[i] & mask) != 0U) ? ESCX_TRUE : ESCX_FALSE;
}
static void SetCompletionEmitted(ESCX_StateMachine* m, ESCX_BYTE_T s, ESCX_BOOL_T on)
{
	ESCX_BYTE_T i, mask; if (m->completion_bits == (ESCX_BYTE_T*)0)return; i = (ESCX_BYTE_T)(s >> 3); if (i >= m->completion_byte_count)return; mask = (ESCX_BYTE_T)(1U << (s & 7U)); if (on != ESCX_FALSE)m->completion_bits[i] = (ESCX_BYTE_T)(m->completion_bits[i] | mask); else m->completion_bits[i] = (ESCX_BYTE_T)(m->completion_bits[i] & ((ESCX_BYTE_T)~mask));
}
static ESCX_BOOL_T DataIsInitialized(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	ESCX_BYTE_T i, mask; if (m->data_initialized_bits == (ESCX_BYTE_T*)0)return ESCX_FALSE; i = (ESCX_BYTE_T)(s >> 3); if (i >= m->data_initialized_byte_count)return ESCX_FALSE; mask = (ESCX_BYTE_T)(1U << (s & 7U)); return((m->data_initialized_bits[i] & mask) != 0U) ? ESCX_TRUE : ESCX_FALSE;
}
static void DataSetInitialized(ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	ESCX_BYTE_T i, mask; if (m->data_initialized_bits == (ESCX_BYTE_T*)0)return; i = (ESCX_BYTE_T)(s >> 3); if (i >= m->data_initialized_byte_count)return; mask = (ESCX_BYTE_T)(1U << (s & 7U)); m->data_initialized_bits[i] = (ESCX_BYTE_T)(m->data_initialized_bits[i] | mask);
}
ESCX_BOOL_T ESCX_StateMachineIsDataInitialized(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	if ((m == (const ESCX_StateMachine*)0) || (s == ESCX_NO_STATE) || ((ESCX_UINT16_T)s >= m->definition->state_count))return ESCX_FALSE; return DataIsInitialized(m, s);
}
static ESCX_BOOL_T AddAtomic(ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	ESCX_BYTE_T i, order; for (i = 0U; i < m->active_atomic_count; ++i)if (m->active_atomic_states[i] == s)return ESCX_TRUE; if (m->active_atomic_count >= ESCX_MAX_ACTIVE_ATOMIC_STATES)return ESCX_FALSE; order = StateOrder(m, s); i = m->active_atomic_count; while ((i != 0U) && (StateOrder(m, m->active_atomic_states[i - 1U]) > order)) {
		m->active_atomic_states[i] = m->active_atomic_states[i - 1U]; --i;
	}m->active_atomic_states[i] = s; ++m->active_atomic_count; return ESCX_TRUE;
}
static void RemoveAtomic(ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	ESCX_BYTE_T i, j; for (i = 0U; i < m->active_atomic_count; ++i) {
		if (m->active_atomic_states[i] == s) {
			for (j = i; (ESCX_BYTE_T)(j + 1U) < m->active_atomic_count; ++j)m->active_atomic_states[j] = m->active_atomic_states[j + 1U]; --m->active_atomic_count; return;
		}
	}
}
ESCX_BOOL_T ESCX_StateMachineIsInState(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	if ((m == (const ESCX_StateMachine*)0) || (m->running == ESCX_FALSE) || (s == ESCX_NO_STATE))return ESCX_FALSE; return IsActive(m, s);
}
ESCX_BOOL_T ESCX_StateMachineIsFinished(const ESCX_StateMachine* m)
{
	return((m != (const ESCX_StateMachine*)0) && (m->finished != ESCX_FALSE)) ? ESCX_TRUE : ESCX_FALSE;
}
ESCX_BOOL_T ESCX_StateMachineIsRunning(const ESCX_StateMachine* m)
{
	return((m != (const ESCX_StateMachine*)0) && (m->running != ESCX_FALSE)) ? ESCX_TRUE : ESCX_FALSE;
}
static ESCX_ProcessStatus ReportErrorInternal(ESCX_StateMachine*, ESCX_ErrorCode, ESCX_ProcessStatus);
static ESCX_BOOL_T IsDescendantOf(const ESCX_StateMachine*, ESCX_BYTE_T, ESCX_BYTE_T);
static void SetCurrentEvent(ESCX_StateMachine*, const ESCX_Event*);
static void SetNeutralCurrentEvent(ESCX_StateMachine*);
static void SetTriggeringEvent(ESCX_StateMachine*, const ESCX_Event*);
static ESCX_BOOL_T IsPendingErrorEvent(const ESCX_StateMachine*, const ESCX_QueuedEvent*);
static void ReadInvokeDescriptor(const ESCX_StateMachine* m, ESCX_BYTE_T index, ESCX_InvokeDescriptor* out)
{
	(void)ESCX_ROM_COPY(out, &m->invokes[index], sizeof(*out));
}
static ESCX_BOOL_T InvokeRange(const ESCX_StateMachine* m, ESCX_BYTE_T state, ESCX_UINT16_T* first, ESCX_BYTE_T* count)
{
	if (m->invoke_map == (const ESCX_InvokeMap*)0)return ESCX_FALSE; if ((ESCX_UINT16_T)state >= m->invoke_map->state_count_total)return ESCX_FALSE; *first = ESCX_ROM_READ_U16(&m->invoke_map->state_first[state]); *count = ESCX_ROM_READ_U8(&m->invoke_map->state_count[state]); return ESCX_TRUE;
}
static void ScheduleInvokesForState(ESCX_StateMachine* m, ESCX_BYTE_T state)
{
	ESCX_BYTE_T i, count; ESCX_UINT16_T first, index; ESCX_InvokeDescriptor d; if ((m->invokes == (const ESCX_InvokeDescriptor*)0) || (m->invoke_instances == (ESCX_InvokeInstance*)0))return; if (InvokeRange(m, state, &first, &count) != ESCX_FALSE) {
		for (index = first; index < (ESCX_UINT16_T)(first + count); ++index)if (m->invoke_instances[index].active == ESCX_FALSE)m->pending_invokes[index] = ESCX_TRUE; return;
	}for (i = 0U; i < m->invoke_count; ++i) {
		ReadInvokeDescriptor(m, i, &d); if ((d.owner_state == state) && (m->invoke_instances[i].active == ESCX_FALSE))m->pending_invokes[i] = ESCX_TRUE;
	}
}
static ESCX_ProcessStatus StartPendingInvokes(ESCX_StateMachine* m)
{
	ESCX_BYTE_T i; ESCX_InvokeDescriptor d; ESCX_InvokeInitializeFunction initialize; int status; for (i = 0U; i < m->invoke_count; ++i) {
		if (m->pending_invokes[i] == ESCX_FALSE)continue; m->pending_invokes[i] = ESCX_FALSE; ReadInvokeDescriptor(m, i, &d); if ((d.owner_state == ESCX_NO_STATE) || (IsActive(m, d.owner_state) == ESCX_FALSE))continue; if (m->invoke_instances[i].active != ESCX_FALSE)continue; initialize = ReadInvokeInitialize(&d.initialize); if (initialize != (ESCX_InvokeInitializeFunction)0) {
			status = initialize(m, d.child_storage, i); if (status != (int)ESCX_STATUS_PROCESSED)return (ESCX_ProcessStatus)status;
		}if (d.start == (ESCX_InvokeStartFunction)0)return ESCX_STATUS_INVALID_ARGUMENT; status = d.start(m, d.child_storage); if (status != (int)ESCX_STATUS_PROCESSED)return (ESCX_ProcessStatus)status; m->invoke_instances[i].active = ESCX_TRUE;
	}return ESCX_STATUS_PROCESSED;
}
static void CancelInvokesForState(ESCX_StateMachine* m, ESCX_BYTE_T state)
{
	ESCX_BYTE_T i, count; ESCX_UINT16_T first, index; ESCX_InvokeDescriptor d; if ((m->invokes == (const ESCX_InvokeDescriptor*)0) || (m->invoke_instances == (ESCX_InvokeInstance*)0))return; if (InvokeRange(m, state, &first, &count) != ESCX_FALSE) {
		for (index = first; index < (ESCX_UINT16_T)(first + count); ++index) {
			m->pending_invokes[index] = ESCX_FALSE; if (m->invoke_instances[index].active != ESCX_FALSE) {
				ReadInvokeDescriptor(m, (ESCX_BYTE_T)index, &d); if (d.cancel != (ESCX_InvokeCancelFunction)0)d.cancel(m, d.child_storage); m->invoke_instances[index].active = ESCX_FALSE;
			}
		}return;
	}for (i = 0U; i < m->invoke_count; ++i) {
		ReadInvokeDescriptor(m, i, &d); if (d.owner_state == state) {
			m->pending_invokes[i] = ESCX_FALSE; if (m->invoke_instances[i].active != ESCX_FALSE) {
				if (d.cancel != (ESCX_InvokeCancelFunction)0)d.cancel(m, d.child_storage); m->invoke_instances[i].active = ESCX_FALSE;
			}
		}
	}
}
static ESCX_ProcessStatus CallState(ESCX_StateMachine* m, ESCX_BYTE_T s, void* d, int which)
{
	const ESCX_StateDescriptor* x; ESCX_StateFunction legacy; ESCX_StatusStateFunction checked; int status;
	x = &m->definition->states[s]; if (which == 1)CancelInvokesForState(m, s);
	checked = (which == 0) ? ReadStatusState(&x->on_entry_status) : ((which == 1) ? ReadStatusState(&x->on_exit_status) : ReadStatusState(&x->on_state_status));
	if (checked != (ESCX_StatusStateFunction)0) {
		status = checked(m, d); if (status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, (ESCX_ProcessStatus)status);
	}
	else {
		legacy = (which == 0) ? ReadStateFn(&x->on_entry) : ((which == 1) ? ReadStateFn(&x->on_exit) : ReadStateFn(&x->on_state)); if (legacy != (ESCX_StateFunction)0)legacy(m, d);
	}
	if (which == 0)ScheduleInvokesForState(m, s);
	return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus ExecuteGeneratedAction(ESCX_StateMachine* m, const ESCX_StatusActionFunction* p, void* d)
{
	ESCX_StatusActionFunction action; int status; action = ReadStatusAction(p); if (action == (ESCX_StatusActionFunction)0)return ESCX_STATUS_PROCESSED; status = action(m, d); if (status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, (ESCX_ProcessStatus)status); return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus InitializeStateData(ESCX_StateMachine*, ESCX_BYTE_T, void*);
static ESCX_ProcessStatus NotifyFinalEntered(ESCX_StateMachine* m, ESCX_BYTE_T final_state);
static ESCX_ProcessStatus RunPlan(ESCX_StateMachine* m, ESCX_UINT16_T pid, void* d, ESCX_BOOL_T entering);
static ESCX_ProcessStatus RestoreHistory(ESCX_StateMachine* m, ESCX_BYTE_T history_state, void* d);
static ESCX_UINT16_T FirstChild(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	return ESCX_ROM_READ_U16(&m->definition->states[s].first_child_index);
}
static ESCX_BYTE_T ChildCount(const ESCX_StateMachine* m, ESCX_BYTE_T s)
{
	return StateU8(m, s, OFFSETOF_STATE(child_count));
}
static ESCX_ProcessStatus EnterStateTree(ESCX_StateMachine* m, ESCX_BYTE_T s, void* d, ESCX_BYTE_T depth)
{
	ESCX_BYTE_T type, count, child; ESCX_UINT16_T first, i; if (depth > ESCX_MAX_HIERARCHY_DEPTH)return ESCX_STATUS_HIERARCHY_LIMIT; if ((s == ESCX_NO_STATE) || ((ESCX_UINT16_T)s >= m->definition->state_count))return ESCX_STATUS_INVALID_ARGUMENT; if ((Type(m, s) == ESCX_STATE_HISTORY_SHALLOW) || (Type(m, s) == ESCX_STATE_HISTORY_DEEP))return RestoreHistory(m, s, d); if (IsActive(m, s) == ESCX_FALSE) {
		{
			ESCX_ProcessStatus dr; if (m->data_binding == ESCX_DATA_BINDING_LATE) {
				dr = InitializeStateData(m, s, d); if (dr != ESCX_STATUS_PROCESSED)return dr;
			}
		}SetActive(m, s, ESCX_TRUE);
		{
			ESCX_ProcessStatus sr; sr = CallState(m, s, d, 0); if (sr != ESCX_STATUS_PROCESSED)return sr; sr = CallState(m, s, d, 2); if (sr != ESCX_STATUS_PROCESSED)return sr;
		}
	}type = Type(m, s); if (type == ESCX_STATE_COMPOUND) {
		ESCX_ProcessStatus ir; ESCX_UINT16_T initial_plan; ir = ExecuteGeneratedAction(m, &m->definition->states[s].initial_action, d); if (ir != ESCX_STATUS_PROCESSED)return ir; initial_plan = InitialEntryPlan(m, s); if (initial_plan != ESCX_NO_PLAN)return RunPlan(m, initial_plan, d, ESCX_TRUE); child = Initial(m, s); return EnterStateTree(m, child, d, (ESCX_BYTE_T)(depth + 1U));
	}if (type == ESCX_STATE_PARALLEL) {
		first = FirstChild(m, s); count = ChildCount(m, s); if ((ESCX_UINT16_T)(first + count) > m->definition->child_count)return ESCX_STATUS_INVALID_ARGUMENT; for (i = 0U; i < count; ++i) {
			child = ESCX_ROM_READ_U8(&m->definition->children[first + i]);
			{
				ESCX_ProcessStatus child_status; child_status = EnterStateTree(m, child, d, (ESCX_BYTE_T)(depth + 1U)); if (child_status != ESCX_STATUS_PROCESSED)return child_status;
			}
		}return ESCX_STATUS_PROCESSED;
	}if (AddAtomic(m, s) == ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT; if (type == ESCX_STATE_FINAL)return NotifyFinalEntered(m, s); return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus EnterInitial(ESCX_StateMachine* m, ESCX_BYTE_T s, void* d)
{
	return EnterStateTree(m, s, d, 0U);
}
void ESCX_StateMachineInit(ESCX_StateMachine* m, const ESCX_StateMachineDefinition* d, void* i, ESCX_BYTE_T* b, ESCX_BYTE_T z, ESCX_HistoryStorage* h, ESCX_StartTimerFunction st, ESCX_CancelTimerFunction ct)
{
	if ((m == (ESCX_StateMachine*)0) || (d == (const ESCX_StateMachineDefinition*)0) || (b == (ESCX_BYTE_T*)0) || (z < ESCX_ACTIVE_BYTES(d->state_count))) {
		ESCX_ASSERT_TRUE(0); return;
	}m->definition = d; m->instance = i; m->history = h; m->history_map = (const ESCX_HistoryMap*)0; m->data_binding = ESCX_DATA_BINDING_EARLY; m->data_initialized_bits = (ESCX_BYTE_T*)0; m->data_initialized_byte_count = 0U; m->invokes = (const ESCX_InvokeDescriptor*)0; m->invoke_instances = (ESCX_InvokeInstance*)0; m->invoke_count = 0U; m->invoke_map = (const ESCX_InvokeMap*)0; m->exit_order = (const ESCX_ExitOrder*)0;
	{
		ESCX_BYTE_T pi; for (pi = 0U; pi < ESCX_MAX_INVOKES; ++pi)m->pending_invokes[pi] = ESCX_FALSE;
	}m->parent_machine = (ESCX_StateMachine*)0; m->parent_invoke_id = ESCX_NO_INVOKE;
#if ESCX_ENABLE_DELAYED_SENDS
	{
		ESCX_BYTE_T di; for (di = 0U; di < ESCX_MAX_DELAYED_SENDS; ++di) {
			m->delayed_sends[di].active = ESCX_FALSE; m->delayed_sends[di].timer_id = ESCX_INVALID_TIMER_ID; m->delayed_sends[di].uses_target_descriptor = ESCX_FALSE;
		}
	}
#endif
	m->active_bits = b; m->active_byte_count = z; m->fast_dispatch = (const ESCX_FastDispatch*)0; m->conflict_map = (const ESCX_ConflictMap*)0; m->preemption_map = (const ESCX_PreemptionMap*)0; m->transition_domain_map = (const ESCX_TransitionDomainMap*)0; m->workspace = (ESCX_MicrostepWorkspace*)0; m->completion_bits = (ESCX_BYTE_T*)0; m->completion_byte_count = 0U; memset(b, 0, z); m->active_atomic_count = 0U; m->current_event = ESCX_NO_EVENT; m->system_context.session_id = 0U; m->system_context.machine_name_id = 0U; m->system_context.io_processors = (const void*)0; m->system_context.platform_data = (void*)0; m->io_processor_send = (ESCX_IOProcessorSendFunction)0; m->io_processor_context = (void*)0; m->event_parent = (const ESCX_BYTE_T*)0; m->event_parent_count = 0U; m->wildcard_event = ESCX_NO_EVENT; m->current_event_descriptor.name_id = ESCX_NO_EVENT; m->current_event_descriptor.type = ESCX_EVENT_TYPE_INTERNAL; m->current_event_descriptor.origin_type = ESCX_EVENT_ORIGIN_NONE; m->current_event_descriptor.invoke_id = ESCX_NO_INVOKE; m->current_event_descriptor.send_id = ESCX_NO_SEND_ID; m->current_event_descriptor.data = (void*)0; m->triggering_event_descriptor = m->current_event_descriptor; m->has_triggering_event = ESCX_FALSE; m->error_execution_event = ESCX_NO_EVENT; m->error_communication_event = ESCX_NO_EVENT; m->last_error = ESCX_ERROR_NONE; m->last_error_status = ESCX_STATUS_PROCESSED; m->last_macrostep_result.event_accepted = ESCX_FALSE; m->last_macrostep_result.transition_taken = ESCX_FALSE; m->last_macrostep_result.microstep_count = 0U; m->error_event_data.code = ESCX_ERROR_NONE; m->error_event_data.cause = ESCX_STATUS_PROCESSED; m->error_event_data.triggering_event = (m->has_triggering_event != ESCX_FALSE) ? m->triggering_event_descriptor : m->current_event_descriptor; m->pending_error_event_data = m->error_event_data; m->reporting_error = ESCX_FALSE; m->running = ESCX_FALSE; m->processing = ESCX_FALSE; m->reporting_error = ESCX_FALSE; m->finished = ESCX_FALSE; m->finish_pending = ESCX_FALSE;
#if ESCX_ENABLE_TIMERS
	m->start_timer = st; m->cancel_timer = ct; m->timer_context = (void*)0;
#else
	(void)st; (void)ct;
#endif
	if (h != (ESCX_HistoryStorage*)0) {
		ESCX_BYTE_T hi; if ((h->valid == (ESCX_BYTE_T*)0) || (h->bits == (ESCX_BYTE_T*)0) || (h->bytes_per_slot < z)) {
			ESCX_ASSERT_TRUE(0); return;
		}for (hi = 0U; hi < h->slot_count; ++hi)h->valid[hi] = 0U;
	}ESCX_EventQueueInit(&m->internal_queue); ESCX_EventQueueInit(&m->external_queue);
#if ESCX_ENABLE_DELAYED_SENDS
	{
		ESCX_BYTE_T di; for (di = 0U; di < ESCX_MAX_DELAYED_SENDS; ++di) {
			if ((m->delayed_sends[di].active != ESCX_FALSE) && (m->cancel_timer != (ESCX_CancelTimerFunction)0))(void)m->cancel_timer(m, m->delayed_sends[di].timer_id, m->timer_context); m->delayed_sends[di].active = ESCX_FALSE; m->delayed_sends[di].timer_id = ESCX_INVALID_TIMER_ID; m->delayed_sends[di].uses_target_descriptor = ESCX_FALSE;
		}
	}
#endif
}
static ESCX_ProcessStatus Stabilize(ESCX_StateMachine* m);
static void ExitCompleteConfiguration(ESCX_StateMachine* m, void* d);
static void FinishInterpreter(ESCX_StateMachine* m);
static ESCX_UINT16_T FindSelector(const ESCX_StateMachine*, ESCX_BYTE_T, ESCX_BYTE_T);
static ESCX_BOOL_T PlanConflict(const ESCX_StateMachine*, ESCX_UINT16_T, ESCX_UINT16_T);
static ESCX_BYTE_T OrderOfTransition(const ESCX_StateMachine*, ESCX_UINT16_T);
ESCX_ProcessStatus ESCX_StateMachineConfigureHistoryMap(ESCX_StateMachine* m, const ESCX_HistoryMap* map)
{
	ESCX_BYTE_T slot, count, state, history_state, owner, type, previous, seen[ESCX_ACTIVE_BYTES(ESCX_MAX_STATES)]; ESCX_UINT16_T first, i, j; ESCX_BOOL_T found;
	if ((m == (ESCX_StateMachine*)0) || (map == (const ESCX_HistoryMap*)0) || (map->slot_first == (const ESCX_UINT16_T*)0) || (map->slot_count == (const ESCX_BYTE_T*)0) || (map->states == (const ESCX_BYTE_T*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	if ((m->history == (ESCX_HistoryStorage*)0) || (map->slot_count_total != m->history->slot_count) || (m->running != ESCX_FALSE))return ESCX_STATUS_INVALID_ARGUMENT;
	for (slot = 0U; slot < m->history->slot_count; ++slot) {
		found = ESCX_FALSE; history_state = ESCX_NO_STATE;
		for (j = 1U; j < m->definition->state_count; ++j) {
			state = (ESCX_BYTE_T)j; type = Type(m, state); if (((type == ESCX_STATE_HISTORY_SHALLOW) || (type == ESCX_STATE_HISTORY_DEEP)) && (HistorySlot(m, state) == slot)) {
				if (found != ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT; found = ESCX_TRUE; history_state = state;
			}
		}
		if (found == ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;
		owner = Parent(m, history_state); if (owner == ESCX_NO_STATE)return ESCX_STATUS_INVALID_ARGUMENT;
		first = ESCX_ROM_READ_U16(&map->slot_first[slot]); count = ESCX_ROM_READ_U8(&map->slot_count[slot]); if ((ESCX_UINT16_T)(first + count) > map->state_count)return ESCX_STATUS_INVALID_ARGUMENT;
		memset(seen, 0, sizeof(seen)); previous = ESCX_NO_STATE;
		for (i = 0U; i < count; ++i) {
			state = ESCX_ROM_READ_U8(&map->states[first + i]); if ((state == ESCX_NO_STATE) || ((ESCX_UINT16_T)state >= m->definition->state_count) || ((seen[state >> 3] & (ESCX_BYTE_T)(1U << (state & 7U))) != 0U))return ESCX_STATUS_INVALID_ARGUMENT; if (IsDescendantOf(m, state, owner) == ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT; if ((Type(m, state) == ESCX_STATE_HISTORY_SHALLOW) || (Type(m, state) == ESCX_STATE_HISTORY_DEEP))return ESCX_STATUS_INVALID_ARGUMENT; if (Type(m, history_state) == ESCX_STATE_HISTORY_SHALLOW) {
				if (Parent(m, state) != owner)return ESCX_STATUS_INVALID_ARGUMENT;
			}
			else if (previous != ESCX_NO_STATE) {
				if (StateDepth(m, previous) > StateDepth(m, state))return ESCX_STATUS_INVALID_ARGUMENT; if ((StateDepth(m, previous) == StateDepth(m, state)) && (StateOrder(m, previous) >= StateOrder(m, state)))return ESCX_STATUS_INVALID_ARGUMENT;
			}
			seen[state >> 3] = (ESCX_BYTE_T)(seen[state >> 3] | (ESCX_BYTE_T)(1U << (state & 7U))); previous = state;
		}
	}
	m->history_map = map; return ESCX_STATUS_PROCESSED;
}
static ESCX_UINT16_T FastTableSelector(const ESCX_FastDispatch* fast, ESCX_BYTE_T state, ESCX_BYTE_T event)
{
	ESCX_UINT16_T base, slot; base = ESCX_ROM_READ_U16(&fast->state_base[state]); if (base > (ESCX_UINT16_T)(65535U - event))return ESCX_NO_SELECTOR; slot = (ESCX_UINT16_T)(base + event); if (slot >= fast->slot_count)return ESCX_NO_SELECTOR; if (ESCX_ROM_READ_U8(&fast->slot_owner[slot]) != state)return ESCX_NO_SELECTOR; return ESCX_ROM_READ_U16(&fast->slot_selector[slot]);
}
static ESCX_UINT16_T EffectiveGeneratedSelector(const ESCX_StateMachine* m, ESCX_BYTE_T leaf, ESCX_BYTE_T event)
{
	ESCX_BYTE_T state, current, parent, depth, probes; ESCX_UINT16_T selector; state = leaf; depth = 0U; while (state != ESCX_NO_STATE) {
		current = event; probes = 0U; for (;;) {
			selector = FindSelector(m, state, current); if (selector != ESCX_NO_SELECTOR)return selector; if ((m->event_parent == (const ESCX_BYTE_T*)0) || (current >= m->event_parent_count))break; parent = ESCX_ROM_READ_U8(&m->event_parent[current]); if ((parent == ESCX_NO_EVENT) || (parent == current))break; current = parent; if (++probes > ESCX_MAX_HIERARCHY_DEPTH)break;
		}if ((m->wildcard_event != ESCX_NO_EVENT) && (m->wildcard_event != event)) {
			selector = FindSelector(m, state, m->wildcard_event); if (selector != ESCX_NO_SELECTOR)return selector;
		}state = Parent(m, state); if (++depth > ESCX_MAX_HIERARCHY_DEPTH)break;
	}return ESCX_NO_SELECTOR;
}
static ESCX_UINT16_T EffectiveGeneratedEventlessSelector(const ESCX_StateMachine* m, ESCX_BYTE_T leaf)
{
	ESCX_BYTE_T state, depth; ESCX_UINT16_T selector; state = leaf; depth = 0U; while (state != ESCX_NO_STATE) {
		selector = EventlessSelector(m, state); if (selector != ESCX_NO_SELECTOR)return selector; state = Parent(m, state); if (++depth > ESCX_MAX_HIERARCHY_DEPTH)break;
	}return ESCX_NO_SELECTOR;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureFastDispatch(ESCX_StateMachine* m, const ESCX_FastDispatch* fast)
{
	ESCX_BYTE_T state, event; ESCX_UINT16_T expected, actual;
	if ((m == (ESCX_StateMachine*)0) || (fast == (const ESCX_FastDispatch*)0) || (fast->state_base == (const ESCX_UINT16_T*)0) || (fast->slot_owner == (const ESCX_BYTE_T*)0) || (fast->slot_selector == (const ESCX_UINT16_T*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	if (m->running != ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;
	if ((fast->eventless_selector != (const ESCX_UINT16_T*)0) && (fast->eventless_count < m->definition->state_count))return ESCX_STATUS_INVALID_ARGUMENT;
	for (state = 1U; (ESCX_UINT16_T)state < m->definition->state_count; ++state) {
		if ((Type(m, state) != ESCX_STATE_ATOMIC) && (Type(m, state) != ESCX_STATE_FINAL))continue;
		for (event = 0U; event < m->definition->event_count; ++event) {
			expected = EffectiveGeneratedSelector(m, state, event); actual = FastTableSelector(fast, state, event); if (actual != expected)return ESCX_STATUS_INVALID_ARGUMENT; if ((actual != ESCX_NO_SELECTOR) && (actual >= m->definition->selector_count))return ESCX_STATUS_INVALID_ARGUMENT;
		}
		if (fast->eventless_selector != (const ESCX_UINT16_T*)0) {
			expected = EffectiveGeneratedEventlessSelector(m, state); actual = ESCX_ROM_READ_U16(&fast->eventless_selector[state]); if (actual != expected)return ESCX_STATUS_INVALID_ARGUMENT; if ((actual != ESCX_NO_SELECTOR) && (actual >= m->definition->selector_count))return ESCX_STATUS_INVALID_ARGUMENT;
		}
	}
	m->fast_dispatch = fast; return ESCX_STATUS_PROCESSED;
}
static ESCX_BOOL_T ValidatePlanRange(const ESCX_StateMachine* m, ESCX_UINT16_T pid)
{
	const ESCX_StatePlan* p; ESCX_UINT16_T first, i; ESCX_BYTE_T count, state; if (pid == ESCX_NO_PLAN)return ESCX_TRUE; if (pid >= m->definition->plan_count)return ESCX_FALSE; p = &m->definition->plans[pid]; first = ESCX_ROM_READ_U16(&p->first_state); count = ESCX_ROM_READ_U8(&p->state_count); if ((ESCX_UINT16_T)(first + count) > m->definition->plan_state_count)return ESCX_FALSE; for (i = 0U; i < count; ++i) {
		state = ESCX_ROM_READ_U8(&m->definition->plan_states[first + i]); if ((state == ESCX_NO_STATE) || ((ESCX_UINT16_T)state >= m->definition->state_count))return ESCX_FALSE;
	}return ESCX_TRUE;
}
static ESCX_BOOL_T PlanContainsAncestorOfSource(const ESCX_StateMachine* m, ESCX_UINT16_T pid, ESCX_BYTE_T source)
{
	const ESCX_StatePlan* p; ESCX_UINT16_T first, i; ESCX_BYTE_T count, state; if (pid == ESCX_NO_PLAN)return ESCX_FALSE; p = &m->definition->plans[pid]; first = ESCX_ROM_READ_U16(&p->first_state); count = ESCX_ROM_READ_U8(&p->state_count); for (i = 0U; i < count; ++i) {
		state = ESCX_ROM_READ_U8(&m->definition->plan_states[first + i]); if (IsDescendantOf(m, source, state) != ESCX_FALSE)return ESCX_TRUE;
	}return ESCX_FALSE;
}
static ESCX_BOOL_T PlanAllDescendantsOfSource(const ESCX_StateMachine* m, ESCX_UINT16_T pid, ESCX_BYTE_T source)
{
	const ESCX_StatePlan* p; ESCX_UINT16_T first, i; ESCX_BYTE_T count, state; if (pid == ESCX_NO_PLAN)return ESCX_TRUE; p = &m->definition->plans[pid]; first = ESCX_ROM_READ_U16(&p->first_state); count = ESCX_ROM_READ_U8(&p->state_count); for (i = 0U; i < count; ++i) {
		state = ESCX_ROM_READ_U8(&m->definition->plan_states[first + i]); if ((state != source) && (IsDescendantOf(m, state, source) == ESCX_FALSE))return ESCX_FALSE;
	}return ESCX_TRUE;
}
static ESCX_BOOL_T ValidateTransitionPlanOrder(const ESCX_StateMachine* m, ESCX_UINT16_T pid, ESCX_BOOL_T entering)
{
	const ESCX_StatePlan* p; ESCX_UINT16_T first, i; ESCX_BYTE_T count, state, previous, depth, order, pdepth, porder, seen[ESCX_ACTIVE_BYTES(ESCX_MAX_STATES)]; if (pid == ESCX_NO_PLAN)return ESCX_TRUE; p = &m->definition->plans[pid]; first = ESCX_ROM_READ_U16(&p->first_state); count = ESCX_ROM_READ_U8(&p->state_count); memset(seen, 0, sizeof(seen)); previous = ESCX_NO_STATE; for (i = 0U; i < count; ++i) {
		state = ESCX_ROM_READ_U8(&m->definition->plan_states[first + i]); if ((seen[state >> 3] & (ESCX_BYTE_T)(1U << (state & 7U))) != 0U)return ESCX_FALSE; seen[state >> 3] = (ESCX_BYTE_T)(seen[state >> 3] | (ESCX_BYTE_T)(1U << (state & 7U))); if (previous != ESCX_NO_STATE) {
			depth = StateDepth(m, state); order = StateOrder(m, state); pdepth = StateDepth(m, previous); porder = StateOrder(m, previous); if (entering != ESCX_FALSE) {
				if ((pdepth > depth) || ((pdepth == depth) && (porder >= order)))return ESCX_FALSE;
			}
			else {
				if ((pdepth < depth) || ((pdepth == depth) && (porder <= order)))return ESCX_FALSE;
			}
		}previous = state;
	}return ESCX_TRUE;
}
static void ReadTransitionDomainDescriptor(const ESCX_TransitionDomainMap* map, ESCX_UINT16_T index, ESCX_TransitionDomainDescriptor* out)
{
	(void)ESCX_ROM_COPY(out, &map->transitions[index], sizeof(*out));
}
static ESCX_BYTE_T LeastCommonCompoundAncestor(const ESCX_StateMachine* m, ESCX_BYTE_T source, const ESCX_TransitionDomainMap* map, const ESCX_TransitionDomainDescriptor* d)
{
	ESCX_BYTE_T candidate, target; ESCX_UINT16_T i; candidate = source; while (candidate != ESCX_NO_STATE) {
		if ((Type(m, candidate) == ESCX_STATE_COMPOUND) || (Type(m, candidate) == ESCX_STATE_PARALLEL)) {
			for (i = 0U; i < d->target_count; ++i) {
				target = ESCX_ROM_READ_U8(&map->targets[d->first_target + i]); if (IsDescendantOf(m, target, candidate) == ESCX_FALSE)break;
			}if (i == d->target_count)return candidate;
		}candidate = Parent(m, candidate);
	}return ESCX_NO_STATE;
}
static ESCX_BOOL_T ExitPlanMatchesDomain(const ESCX_StateMachine* m, ESCX_UINT16_T pid, ESCX_BYTE_T domain)
{
	const ESCX_StatePlan* p; ESCX_UINT16_T first, i; ESCX_BYTE_T count, state; if (pid == ESCX_NO_PLAN)return ESCX_FALSE; p = &m->definition->plans[pid]; first = ESCX_ROM_READ_U16(&p->first_state); count = ESCX_ROM_READ_U8(&p->state_count); for (i = 0U; i < count; ++i) {
		state = ESCX_ROM_READ_U8(&m->definition->plan_states[first + i]); if ((state == domain) || (IsDescendantOf(m, state, domain) == ESCX_FALSE))return ESCX_FALSE;
	}return ESCX_TRUE;
}
static ESCX_BOOL_T EntryPlanContainsState(const ESCX_StateMachine* m, ESCX_UINT16_T pid, ESCX_BYTE_T required)
{
	const ESCX_StatePlan* p; ESCX_UINT16_T first, i; ESCX_BYTE_T count; if (pid == ESCX_NO_PLAN)return ESCX_FALSE; p = &m->definition->plans[pid]; first = ESCX_ROM_READ_U16(&p->first_state); count = ESCX_ROM_READ_U8(&p->state_count); for (i = 0U; i < count; ++i)if (ESCX_ROM_READ_U8(&m->definition->plan_states[first + i]) == required)return ESCX_TRUE; return ESCX_FALSE;
}
static ESCX_BOOL_T EntryPlanMatchesTargets(const ESCX_StateMachine* m, ESCX_UINT16_T pid, ESCX_BYTE_T domain, const ESCX_TransitionDomainMap* map, const ESCX_TransitionDomainDescriptor* d)
{
	const ESCX_StatePlan* p; ESCX_UINT16_T first, i, j; ESCX_BYTE_T count, state, target, depth; if (pid == ESCX_NO_PLAN)return ESCX_FALSE; p = &m->definition->plans[pid]; first = ESCX_ROM_READ_U16(&p->first_state); count = ESCX_ROM_READ_U8(&p->state_count); for (i = 0U; i < count; ++i) {
		state = ESCX_ROM_READ_U8(&m->definition->plan_states[first + i]); if ((state == domain) || (IsDescendantOf(m, state, domain) == ESCX_FALSE))return ESCX_FALSE;
	}
	for (j = 0U; j < d->target_count; ++j) {
		target = ESCX_ROM_READ_U8(&map->targets[d->first_target + j]); state = target; depth = 0U; while (state != domain) {
			if ((state == ESCX_NO_STATE) || (EntryPlanContainsState(m, pid, state) == ESCX_FALSE))return ESCX_FALSE; state = Parent(m, state); if (++depth > ESCX_MAX_HIERARCHY_DEPTH)return ESCX_FALSE;
		}
	}
	return ESCX_TRUE;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureTransitionDomainMap(ESCX_StateMachine* m, const ESCX_TransitionDomainMap* map)
{
	ESCX_UINT16_T i, j; ESCX_BYTE_T declared, source, target, expected_type, expected_domain; ESCX_TransitionDomainDescriptor d; if ((m == (ESCX_StateMachine*)0) || (map == (const ESCX_TransitionDomainMap*)0) || (map->transitions == (const ESCX_TransitionDomainDescriptor*)0) || (map->targets == (const ESCX_BYTE_T*)0) || (m->running != ESCX_FALSE))return ESCX_STATUS_INVALID_ARGUMENT; if (map->transition_count != m->definition->transition_count)return ESCX_STATUS_INVALID_ARGUMENT; for (i = 0U; i < map->transition_count; ++i) {
		ReadTransitionDomainDescriptor(map, i, &d); declared = ESCX_ROM_READ_U8(&m->definition->transitions[i].type); source = ESCX_ROM_READ_U8(&m->definition->transitions[i].source_state); if ((ESCX_UINT16_T)(d.first_target + d.target_count) > map->target_count)return ESCX_STATUS_INVALID_ARGUMENT; if (declared == ESCX_TRANSITION_TARGETLESS) {
			if ((d.target_count != 0U) || (d.domain_state != ESCX_NO_STATE) || (d.effective_type != ESCX_TRANSITION_TARGETLESS))return ESCX_STATUS_INVALID_ARGUMENT; continue;
		}if ((d.target_count == 0U) || (source == ESCX_NO_STATE))return ESCX_STATUS_INVALID_ARGUMENT; for (j = 0U; j < d.target_count; ++j) {
			target = ESCX_ROM_READ_U8(&map->targets[d.first_target + j]); if ((target == ESCX_NO_STATE) || ((ESCX_UINT16_T)target >= m->definition->state_count))return ESCX_STATUS_INVALID_ARGUMENT;
		}expected_type = declared; if (declared == ESCX_TRANSITION_INTERNAL) {
			if (Type(m, source) != ESCX_STATE_COMPOUND)expected_type = ESCX_TRANSITION_EXTERNAL; else for (j = 0U; j < d.target_count; ++j) {
				target = ESCX_ROM_READ_U8(&map->targets[d.first_target + j]); if ((target == source) || (IsDescendantOf(m, target, source) == ESCX_FALSE)) {
					expected_type = ESCX_TRANSITION_EXTERNAL; break;
				}
			}
		}expected_domain = (expected_type == ESCX_TRANSITION_INTERNAL) ? source : LeastCommonCompoundAncestor(m, source, map, &d); if ((d.effective_type != expected_type) || (d.domain_state != expected_domain) || (expected_domain == ESCX_NO_STATE))return ESCX_STATUS_INVALID_ARGUMENT; if (ExitPlanMatchesDomain(m, ESCX_ROM_READ_U16(&m->definition->transitions[i].exit_plan), expected_domain) == ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT; if (EntryPlanMatchesTargets(m, ESCX_ROM_READ_U16(&m->definition->transitions[i].entry_plan), expected_domain, map, &d) == ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;
	}m->transition_domain_map = map; return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineValidateTransitionDomains(const ESCX_StateMachine* m)
{
	ESCX_UINT16_T i, xp, ep; ESCX_BYTE_T type, source, source_depth; if ((m == (const ESCX_StateMachine*)0) || (m->definition == (const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT; if (m->running != ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT; for (i = 0U; i < m->definition->transition_count; ++i) {
		const ESCX_Transition* t = &m->definition->transitions[i]; type = ESCX_ROM_READ_U8(&t->type); source = ESCX_ROM_READ_U8(&t->source_state); source_depth = ESCX_ROM_READ_U8(&t->source_depth); xp = ESCX_ROM_READ_U16(&t->exit_plan); ep = ESCX_ROM_READ_U16(&t->entry_plan); if ((type != ESCX_TRANSITION_EXTERNAL) && (type != ESCX_TRANSITION_INTERNAL) && (type != ESCX_TRANSITION_TARGETLESS))return ESCX_STATUS_INVALID_ARGUMENT; if (type == ESCX_TRANSITION_TARGETLESS) {
			if ((xp != ESCX_NO_PLAN) || (ep != ESCX_NO_PLAN))return ESCX_STATUS_INVALID_ARGUMENT; continue;
		}if ((source == ESCX_NO_STATE) || ((ESCX_UINT16_T)source >= m->definition->state_count) || (source_depth != StateDepth(m, source)))return ESCX_STATUS_INVALID_ARGUMENT; if ((ValidatePlanRange(m, xp) == ESCX_FALSE) || (ValidatePlanRange(m, ep) == ESCX_FALSE))return ESCX_STATUS_INVALID_ARGUMENT; if ((ValidateTransitionPlanOrder(m, xp, ESCX_FALSE) == ESCX_FALSE) || (ValidateTransitionPlanOrder(m, ep, ESCX_TRUE) == ESCX_FALSE))return ESCX_STATUS_INVALID_ARGUMENT; if ((xp == ESCX_NO_PLAN) || (ep == ESCX_NO_PLAN))return ESCX_STATUS_INVALID_ARGUMENT; if (type == ESCX_TRANSITION_INTERNAL) {
			if ((Type(m, source) == ESCX_STATE_COMPOUND) && (PlanContainsAncestorOfSource(m, xp, source) == ESCX_FALSE) && (PlanAllDescendantsOfSource(m, ep, source) != ESCX_FALSE)) {
				/* Eligible internal transition: generated plans remain below source. */
			}
			else if (PlanContainsAncestorOfSource(m, xp, source) == ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;
		}
		else {
			if (PlanContainsAncestorOfSource(m, xp, source) == ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;
		}
	}
	return ESCX_STATUS_PROCESSED;
}
static ESCX_BOOL_T ConflictMapBit(const ESCX_ConflictMap* map, ESCX_UINT16_T a, ESCX_UINT16_T b)
{
	unsigned long index; ESCX_BYTE_T value, mask; index = ((unsigned long)a * (unsigned long)map->bytes_per_row) + (unsigned long)(b >> 3); value = ESCX_ROM_READ_U8(&map->bits[index]); mask = (ESCX_BYTE_T)(1U << (b & 7U)); return((value & mask) != 0U) ? ESCX_TRUE : ESCX_FALSE;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureConflictMap(ESCX_StateMachine* m, const ESCX_ConflictMap* map)
{
	ESCX_UINT16_T required, a, b; ESCX_BOOL_T expected, actual;
	if ((m == (ESCX_StateMachine*)0) || (map == (const ESCX_ConflictMap*)0) || (map->bits == (const ESCX_BYTE_T*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	if (m->running != ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;
	required = (ESCX_UINT16_T)ESCX_CONFLICT_BYTES(m->definition->transition_count);
	if ((map->transition_count != m->definition->transition_count) || (map->bytes_per_row < required))return ESCX_STATUS_INVALID_ARGUMENT;
	if (ESCX_StateMachineValidateTransitionDomains(m) != ESCX_STATUS_PROCESSED)return ESCX_STATUS_INVALID_ARGUMENT;
	for (a = 0U; a < m->definition->transition_count; ++a) {
		for (b = 0U; b < m->definition->transition_count; ++b) {
			expected = PlanConflict(m, a, b); actual = ConflictMapBit(map, a, b);
			if (actual != expected)return ESCX_STATUS_INVALID_ARGUMENT;
		}
	}
	m->conflict_map = map; return ESCX_STATUS_PROCESSED;
}
static ESCX_BYTE_T SourceOfTransition(const ESCX_StateMachine* m, ESCX_UINT16_T t)
{
	return ESCX_ROM_READ_U8(&m->definition->transitions[t].source_state);
}
static ESCX_BOOL_T NormativeTransitionPreempts(const ESCX_StateMachine* m, ESCX_UINT16_T candidate, ESCX_UINT16_T existing)
{
	ESCX_BYTE_T cs, es; cs = SourceOfTransition(m, candidate); es = SourceOfTransition(m, existing); if ((cs != ESCX_NO_STATE) && (es != ESCX_NO_STATE)) {
		if ((cs != es) && (IsDescendantOf(m, cs, es) != ESCX_FALSE))return ESCX_TRUE; if ((cs != es) && (IsDescendantOf(m, es, cs) != ESCX_FALSE))return ESCX_FALSE;
	}return(OrderOfTransition(m, candidate) < OrderOfTransition(m, existing)) ? ESCX_TRUE : ESCX_FALSE;
}
static ESCX_BOOL_T PreemptionMapBit(const ESCX_PreemptionMap* map, ESCX_UINT16_T candidate, ESCX_UINT16_T existing)
{
	unsigned long index; ESCX_BYTE_T value, mask; index = ((unsigned long)candidate * (unsigned long)map->bytes_per_row) + (unsigned long)(existing >> 3); value = ESCX_ROM_READ_U8(&map->bits[index]); mask = (ESCX_BYTE_T)(1U << (existing & 7U)); return((value & mask) != 0U) ? ESCX_TRUE : ESCX_FALSE;
}
ESCX_ProcessStatus ESCX_StateMachineConfigurePreemptionMap(ESCX_StateMachine* m, const ESCX_PreemptionMap* map)
{
	ESCX_UINT16_T required, candidate, existing; ESCX_BOOL_T expected, actual; if ((m == (ESCX_StateMachine*)0) || (map == (const ESCX_PreemptionMap*)0) || (map->bits == (const ESCX_BYTE_T*)0))return ESCX_STATUS_INVALID_ARGUMENT; if (m->running != ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT; required = (ESCX_UINT16_T)ESCX_CONFLICT_BYTES(m->definition->transition_count); if ((map->transition_count != m->definition->transition_count) || (map->bytes_per_row < required))return ESCX_STATUS_INVALID_ARGUMENT; if (ESCX_StateMachineValidateTransitionDomains(m) != ESCX_STATUS_PROCESSED)return ESCX_STATUS_INVALID_ARGUMENT; for (candidate = 0U; candidate < m->definition->transition_count; ++candidate)for (existing = 0U; existing < m->definition->transition_count; ++existing) {
		expected = (candidate == existing) ? ESCX_FALSE : NormativeTransitionPreempts(m, candidate, existing); actual = PreemptionMapBit(map, candidate, existing); if (actual != expected)return ESCX_STATUS_INVALID_ARGUMENT;
	}m->preemption_map = map; return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureWorkspace(ESCX_StateMachine* m, ESCX_MicrostepWorkspace* w)
{
	ESCX_BYTE_T required; if ((m == (ESCX_StateMachine*)0) || (w == (ESCX_MicrostepWorkspace*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	required = (ESCX_BYTE_T)ESCX_ACTIVE_BYTES(m->definition->state_count);
	if ((w->exit_states == (ESCX_BYTE_T*)0) || (w->entry_states == (ESCX_BYTE_T*)0) || (w->exit_seen == (ESCX_BYTE_T*)0) || (w->entry_seen == (ESCX_BYTE_T*)0) || (w->exit_capacity == 0U) || (w->entry_capacity == 0U) || (w->seen_byte_count < required))return ESCX_STATUS_INVALID_ARGUMENT;
	if (m->running != ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;
	w->exit_count = 0U; w->entry_count = 0U; m->workspace = w; return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureCompletionStorage(ESCX_StateMachine* m, ESCX_BYTE_T* bits, ESCX_BYTE_T byte_count)
{
	if ((m == (ESCX_StateMachine*)0) || (m->definition == (const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	if ((bits == (ESCX_BYTE_T*)0) || (byte_count < ESCX_ACTIVE_BYTES(m->definition->state_count)))return ESCX_STATUS_INVALID_ARGUMENT;
	if (m->running != ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;
	m->completion_bits = bits; m->completion_byte_count = byte_count; memset(bits, 0, byte_count); return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureDataBinding(ESCX_StateMachine* m, ESCX_DataBinding binding, ESCX_BYTE_T* initialized_bits, ESCX_BYTE_T byte_count)
{
	if ((m == (ESCX_StateMachine*)0) || ((binding != ESCX_DATA_BINDING_EARLY) && (binding != ESCX_DATA_BINDING_LATE)))return ESCX_STATUS_INVALID_ARGUMENT;
	if ((initialized_bits == (ESCX_BYTE_T*)0) || (byte_count < ESCX_ACTIVE_BYTES(m->definition->state_count)))return ESCX_STATUS_INVALID_ARGUMENT;
	if (m->running != ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;
	m->data_binding = (ESCX_BYTE_T)binding; m->data_initialized_bits = initialized_bits; m->data_initialized_byte_count = byte_count; memset(initialized_bits, 0, byte_count); return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus InitializeStateData(ESCX_StateMachine* m, ESCX_BYTE_T state, void* d)
{
	ESCX_StatusStateFunction init; int status; if (m->data_initialized_bits == (ESCX_BYTE_T*)0)return ESCX_STATUS_PROCESSED; if (DataIsInitialized(m, state) != ESCX_FALSE)return ESCX_STATUS_PROCESSED;
	init = ReadStatusState(&m->definition->states[state].data_init_status); if (init != (ESCX_StatusStateFunction)0) {
		status = init(m, d); if (status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, (ESCX_ProcessStatus)status);
	}
	DataSetInitialized(m, state); return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus InitializeAllData(ESCX_StateMachine* m)
{
	ESCX_BYTE_T state; ESCX_ProcessStatus status; for (state = 1U; (ESCX_UINT16_T)state < m->definition->state_count; ++state) {
		status = InitializeStateData(m, state, (void*)0); if (status != ESCX_STATUS_PROCESSED)return status;
	}return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureInvokes(ESCX_StateMachine* m, const ESCX_InvokeDescriptor* d, ESCX_InvokeInstance* i, ESCX_BYTE_T count)
{
	ESCX_BYTE_T n; if ((m == (ESCX_StateMachine*)0) || (count > ESCX_MAX_INVOKES) || (m->running != ESCX_FALSE))return ESCX_STATUS_INVALID_ARGUMENT; if ((count != 0U) && ((d == (const ESCX_InvokeDescriptor*)0) || (i == (ESCX_InvokeInstance*)0)))return ESCX_STATUS_INVALID_ARGUMENT; m->invokes = d; m->invoke_instances = i; m->invoke_count = count; for (n = 0U; n < ESCX_MAX_INVOKES; ++n)m->pending_invokes[n] = ESCX_FALSE; for (n = 0U; n < count; ++n)i[n].active = ESCX_FALSE; return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureInvokeMap(ESCX_StateMachine* m, const ESCX_InvokeMap* map)
{
	ESCX_BYTE_T state, count, i; ESCX_UINT16_T first, index; ESCX_InvokeDescriptor d; if ((m == (ESCX_StateMachine*)0) || (map == (const ESCX_InvokeMap*)0) || (map->state_first == (const ESCX_UINT16_T*)0) || (map->state_count == (const ESCX_BYTE_T*)0) || (m->running != ESCX_FALSE))return ESCX_STATUS_INVALID_ARGUMENT; if ((map->state_count_total < m->definition->state_count) || (map->invoke_count_total != m->invoke_count))return ESCX_STATUS_INVALID_ARGUMENT; for (state = 0U; (ESCX_UINT16_T)state < m->definition->state_count; ++state) {
		first = ESCX_ROM_READ_U16(&map->state_first[state]); count = ESCX_ROM_READ_U8(&map->state_count[state]); if ((ESCX_UINT16_T)(first + count) > m->invoke_count)return ESCX_STATUS_INVALID_ARGUMENT; for (i = 0U; i < count; ++i) {
			index = (ESCX_UINT16_T)(first + i); ReadInvokeDescriptor(m, (ESCX_BYTE_T)index, &d); if (d.owner_state != state)return ESCX_STATUS_INVALID_ARGUMENT;
		}
	}m->invoke_map = map; return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureExitOrder(ESCX_StateMachine* m, const ESCX_ExitOrder* order)
{
	ESCX_UINT16_T i; ESCX_BYTE_T state, previous, seen[ESCX_ACTIVE_BYTES(ESCX_MAX_STATES)];
	if ((m == (ESCX_StateMachine*)0) || (order == (const ESCX_ExitOrder*)0) || (order->states == (const ESCX_BYTE_T*)0) || (m->running != ESCX_FALSE))return ESCX_STATUS_INVALID_ARGUMENT;
	if (order->state_count != (ESCX_UINT16_T)(m->definition->state_count - 1U))return ESCX_STATUS_INVALID_ARGUMENT;
	memset(seen, 0, sizeof(seen)); previous = ESCX_NO_STATE;
	for (i = 0U; i < order->state_count; ++i) {
		state = ESCX_ROM_READ_U8(&order->states[i]); if ((state == ESCX_NO_STATE) || ((ESCX_UINT16_T)state >= m->definition->state_count) || ((seen[state >> 3] & (ESCX_BYTE_T)(1U << (state & 7U))) != 0U))return ESCX_STATUS_INVALID_ARGUMENT; if (previous != ESCX_NO_STATE) {
			if (StateDepth(m, previous) < StateDepth(m, state))return ESCX_STATUS_INVALID_ARGUMENT; if ((StateDepth(m, previous) == StateDepth(m, state)) && (StateOrder(m, previous) < StateOrder(m, state)))return ESCX_STATUS_INVALID_ARGUMENT;
		}seen[state >> 3] = (ESCX_BYTE_T)(seen[state >> 3] | (ESCX_BYTE_T)(1U << (state & 7U))); previous = state;
	}
	m->exit_order = order; return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus ExecuteInvokeFinalizeEvent(ESCX_StateMachine* m, const ESCX_InvokeDescriptor* d, const ESCX_Event* event)
{
	ESCX_InvokeFinalizeStatusFunction checked; int status;
	SetCurrentEvent(m, event);
	checked = ReadInvokeFinalizeStatus(&d->finalize_status); if (checked != (ESCX_InvokeFinalizeStatusFunction)0) {
		status = checked(m, d->child_storage, event); if (status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, (ESCX_ProcessStatus)status);
	}
	else if (d->finalize != (ESCX_InvokeFinalizeFunction)0)d->finalize(m, d->child_storage, event->name_id, event->data);
	return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineCompleteInvokeEvent(ESCX_StateMachine* m, ESCX_BYTE_T invoke_id, const ESCX_Event* event)
{
	ESCX_InvokeDescriptor d; 
	ESCX_DoneDataFunction done_data; 
	ESCX_ProcessStatus fs;
	ESCX_Event done;
	void* payload; 
	int status;
	if ((m == (ESCX_StateMachine*)0) || (event == (const ESCX_Event*)0) || (invoke_id >= m->invoke_count) || (m->invoke_instances[invoke_id].active == ESCX_FALSE))
		return ESCX_STATUS_INVALID_ARGUMENT;
	ReadInvokeDescriptor(m, invoke_id, &d); 
	m->invoke_instances[invoke_id].active = ESCX_FALSE; 
	done = *event; done.origin_type = ESCX_EVENT_ORIGIN_INVOKE;
	done.invoke_id = invoke_id; 
	payload = done.data;
	done_data = d.done_data;
	if ((payload == (void*)0) && (done_data != (ESCX_DoneDataFunction)0)) {
		status = done_data(m, &payload); if (status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, (ESCX_ProcessStatus)status); done.data = payload;
	}
	fs = ExecuteInvokeFinalizeEvent(m, &d, &done); if (fs != ESCX_STATUS_PROCESSED) {
		SetNeutralCurrentEvent(m); return fs;
	}
	if (done.name_id == ESCX_NO_EVENT) {
		SetNeutralCurrentEvent(m); return ESCX_STATUS_PROCESSED;
	}
	if (done.name_id >= m->definition->event_count) {
		SetNeutralCurrentEvent(m); return ESCX_STATUS_INVALID_EVENT;
	}
	if (ESCX_EventQueuePushEx(&m->internal_queue, done.name_id, done.type, done.origin_type, done.invoke_id, done.send_id, done.data) == ESCX_FALSE) {
		SetNeutralCurrentEvent(m); return ESCX_STATUS_QUEUE_FULL;
	}
	if (m->processing == ESCX_FALSE) {
		ESCX_ProcessStatus result; m->processing = ESCX_TRUE; result = Stabilize(m); m->processing = ESCX_FALSE; if ((result == ESCX_STATUS_PROCESSED) && (m->finish_pending != ESCX_FALSE))FinishInterpreter(m); return result;
	}
	/* Finalize ran synchronously, but the queued done.invoke event will be consumed later. */
	SetNeutralCurrentEvent(m);
	return ESCX_STATUS_PROCESSED;
}

ESCX_ProcessStatus ESCX_StateMachineCompleteInvoke(ESCX_StateMachine* m, ESCX_BYTE_T invoke_id, void* data)
{
	ESCX_InvokeDescriptor d; ESCX_Event event; if ((m == (ESCX_StateMachine*)0) || (invoke_id >= m->invoke_count))return ESCX_STATUS_INVALID_ARGUMENT; ReadInvokeDescriptor(m, invoke_id, &d); event.name_id = d.done_event; event.type = ESCX_EVENT_TYPE_INTERNAL; event.origin_type = ESCX_EVENT_ORIGIN_INVOKE; event.invoke_id = invoke_id; event.send_id = ESCX_NO_SEND_ID; event.data = data; return ESCX_StateMachineCompleteInvokeEvent(m, invoke_id, &event);
}

ESCX_ProcessStatus ESCX_StateMachineReceiveInvokeEventDescriptor(ESCX_StateMachine* m, ESCX_BYTE_T invoke_id, const ESCX_Event* event)
{
	ESCX_InvokeDescriptor d; ESCX_ProcessStatus fs; ESCX_Event normalized; if ((m == (ESCX_StateMachine*)0) || (event == (const ESCX_Event*)0) || (invoke_id >= m->invoke_count) || (m->invoke_instances[invoke_id].active == ESCX_FALSE))return ESCX_STATUS_INVALID_ARGUMENT; if (event->name_id >= m->definition->event_count)return ESCX_STATUS_INVALID_EVENT; normalized = *event; normalized.origin_type = ESCX_EVENT_ORIGIN_INVOKE; normalized.invoke_id = invoke_id; ReadInvokeDescriptor(m, invoke_id, &d); fs = ExecuteInvokeFinalizeEvent(m, &d, &normalized); if (fs != ESCX_STATUS_PROCESSED) {
		SetNeutralCurrentEvent(m); return fs;
	} if (ESCX_EventQueuePushEx(&m->internal_queue, normalized.name_id, normalized.type, normalized.origin_type, normalized.invoke_id, normalized.send_id, normalized.data) == ESCX_FALSE) {
		SetNeutralCurrentEvent(m); return ESCX_STATUS_QUEUE_FULL;
	} if (m->processing == ESCX_FALSE) {
		ESCX_ProcessStatus result; m->processing = ESCX_TRUE; result = Stabilize(m); m->processing = ESCX_FALSE; if ((result == ESCX_STATUS_PROCESSED) && (m->finish_pending != ESCX_FALSE))FinishInterpreter(m); return result;
	}SetNeutralCurrentEvent(m); return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineReceiveInvokeEvent(ESCX_StateMachine* m, ESCX_BYTE_T invoke_id, ESCX_BYTE_T event_id, void* data)
{
	ESCX_Event event; event.name_id = event_id; event.type = ESCX_EVENT_TYPE_EXTERNAL; event.origin_type = ESCX_EVENT_ORIGIN_INVOKE; event.invoke_id = invoke_id; event.send_id = ESCX_NO_SEND_ID; event.data = data; return ESCX_StateMachineReceiveInvokeEventDescriptor(m, invoke_id, &event);
}
static ESCX_ProcessStatus ForwardExternalToInvokes(ESCX_StateMachine* m, ESCX_BYTE_T event_id, void* data)
{
	ESCX_BYTE_T i; ESCX_InvokeDescriptor d; int status; const ESCX_Event* event; event = ESCX_StateMachineGetCurrentEvent(m); for (i = 0U; i < m->invoke_count; ++i) {
		if (m->invoke_instances[i].active != ESCX_FALSE) {
			ReadInvokeDescriptor(m, i, &d); if (d.autoforward != ESCX_FALSE) {
				if (d.forward_event != (ESCX_InvokeForwardEventFunction)0) {
					status = d.forward_event(m, d.child_storage, event); if (status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, (ESCX_ProcessStatus)status);
				}
				else if (d.forward != (ESCX_InvokeForwardFunction)0) {
					status = d.forward(m, d.child_storage, event_id, data); if (status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, (ESCX_ProcessStatus)status);
				}
			}
		}
	}return ESCX_STATUS_PROCESSED;
}
static ESCX_UINT16_T FindSelector(const ESCX_StateMachine* m, ESCX_BYTE_T state, ESCX_BYTE_T event)
{
	ESCX_UINT16_T base, slot; if ((state == ESCX_NO_STATE) || ((ESCX_UINT16_T)state >= m->definition->state_count))return ESCX_NO_SELECTOR; base = ESCX_ROM_READ_U16(&m->definition->state_base[state]); if (base > (ESCX_UINT16_T)(65535U - event))return ESCX_NO_SELECTOR; slot = (ESCX_UINT16_T)(base + event); if (slot >= m->definition->slot_count)return ESCX_NO_SELECTOR; if (ESCX_ROM_READ_U8(&m->definition->slot_owner[slot]) != state)return ESCX_NO_SELECTOR; return ESCX_ROM_READ_U16(&m->definition->slot_selector[slot]);
}
static ESCX_UINT16_T FindFastSelector(const ESCX_StateMachine* m, ESCX_BYTE_T state, ESCX_BYTE_T event)
{
	const ESCX_FastDispatch* f; ESCX_UINT16_T base, slot; if (m->fast_dispatch == (const ESCX_FastDispatch*)0)return ESCX_NO_SELECTOR; f = m->fast_dispatch;
	base = ESCX_ROM_READ_U16(&f->state_base[state]); if (base > (ESCX_UINT16_T)(65535U - event))return ESCX_NO_SELECTOR; slot = (ESCX_UINT16_T)(base + event); if (slot >= f->slot_count)return ESCX_NO_SELECTOR; if (ESCX_ROM_READ_U8(&f->slot_owner[slot]) != state)return ESCX_NO_SELECTOR; return ESCX_ROM_READ_U16(&f->slot_selector[slot]);
}
static ESCX_UINT16_T FindMatchingSelector(const ESCX_StateMachine* m, ESCX_BYTE_T state, ESCX_BYTE_T event)
{
	ESCX_UINT16_T selector; ESCX_BYTE_T current, probes, parent;
	current = event; probes = 0U;
	for (;;) {
		selector = FindSelector(m, state, current); if (selector != ESCX_NO_SELECTOR)return selector;
		if ((m->event_parent == (const ESCX_BYTE_T*)0) || (current >= m->event_parent_count))break;
		parent = ESCX_ROM_READ_U8(&m->event_parent[current]); if ((parent == ESCX_NO_EVENT) || (parent == current))break; current = parent;
		if (++probes > ESCX_MAX_HIERARCHY_DEPTH)break;
	}
	if ((m->wildcard_event != ESCX_NO_EVENT) && (m->wildcard_event != event))return FindSelector(m, state, m->wildcard_event);
	return ESCX_NO_SELECTOR;
}
static ESCX_BOOL_T HasActiveFinalChild(const ESCX_StateMachine* m, ESCX_BYTE_T parent)
{
	ESCX_BYTE_T i, state; for (i = 0U; i < m->active_atomic_count; ++i) {
		state = m->active_atomic_states[i]; if ((Type(m, state) == ESCX_STATE_FINAL) && (Parent(m, state) == parent))return ESCX_TRUE;
	}return ESCX_FALSE;
}
static ESCX_BOOL_T IsComplete(const ESCX_StateMachine* m, ESCX_BYTE_T state)
{
	ESCX_BYTE_T type, count, child; ESCX_UINT16_T first, i; type = Type(m, state); if (type == ESCX_STATE_COMPOUND)return HasActiveFinalChild(m, state); if (type != ESCX_STATE_PARALLEL)return ESCX_FALSE; first = FirstChild(m, state); count = ChildCount(m, state); if (count == 0U)return ESCX_FALSE; if ((ESCX_UINT16_T)(first + count) > m->definition->child_count)return ESCX_FALSE; for (i = 0U; i < count; ++i) {
		child = ESCX_ROM_READ_U8(&m->definition->children[first + i]); if (IsComplete(m, child) == ESCX_FALSE)return ESCX_FALSE;
	}return ESCX_TRUE;
}
static ESCX_ProcessStatus NotifyFinalEntered(ESCX_StateMachine* m, ESCX_BYTE_T final_state)
{
	ESCX_BYTE_T state, immediate_parent, event_id; ESCX_DoneDataFunction done_data; void* payload = (void*)0; void* completion_payload; int status; done_data = ReadDoneData(&m->definition->states[final_state].done_data); if (done_data != (ESCX_DoneDataFunction)0) {
		status = done_data(m, &payload); if (status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, (ESCX_ProcessStatus)status);
	}immediate_parent = Parent(m, final_state); state = immediate_parent; while (state != ESCX_NO_STATE) {
		if (IsComplete(m, state) == ESCX_FALSE)break; if (CompletionWasEmitted(m, state) != ESCX_FALSE) {
			state = Parent(m, state); continue;
		}SetCompletionEmitted(m, state, ESCX_TRUE); if (Parent(m, state) == ESCX_NO_STATE) {
			m->finished = ESCX_TRUE; m->finish_pending = ESCX_TRUE; return ESCX_STATUS_PROCESSED;
		}event_id = DoneEvent(m, state); if (event_id != ESCX_NO_EVENT) {
			/* Final-state donedata belongs only to done.state of its immediate compound parent. */
			completion_payload = (state == immediate_parent) ? payload : (void*)0;
			if (ESCX_EventQueuePushEx(&m->internal_queue, event_id, ESCX_EVENT_TYPE_INTERNAL, ESCX_EVENT_ORIGIN_SCXML, ESCX_NO_INVOKE, ESCX_NO_SEND_ID, completion_payload) == ESCX_FALSE) {
				SetCompletionEmitted(m, state, ESCX_FALSE); return ESCX_STATUS_QUEUE_FULL;
			}
		}state = Parent(m, state);
	}if (immediate_parent == ESCX_NO_STATE) {
		m->finished = ESCX_TRUE; m->finish_pending = ESCX_TRUE;
	}return ESCX_STATUS_PROCESSED;
}
static ESCX_BOOL_T IsDescendantOf(const ESCX_StateMachine* m, ESCX_BYTE_T state, ESCX_BYTE_T ancestor)
{
	ESCX_BYTE_T depth = 0U; while (state != ESCX_NO_STATE) {
		if (state == ancestor)return ESCX_TRUE; state = Parent(m, state); if (++depth > ESCX_MAX_HIERARCHY_DEPTH)break;
	}return ESCX_FALSE;
}
static ESCX_BYTE_T* HistoryBits(ESCX_StateMachine* m, ESCX_BYTE_T slot)
{
	return m->history->bits + ((unsigned int)slot * (unsigned int)m->history->bytes_per_slot);
}
static void HistorySetBit(ESCX_BYTE_T* bits, ESCX_BYTE_T state)
{
	bits[state >> 3] = (ESCX_BYTE_T)(bits[state >> 3] | (ESCX_BYTE_T)(1U << (state & 7U)));
}
static ESCX_BOOL_T HistoryHasBit(const ESCX_BYTE_T* bits, ESCX_BYTE_T state)
{
	return((bits[state >> 3] & (ESCX_BYTE_T)(1U << (state & 7U))) != 0U) ? ESCX_TRUE : ESCX_FALSE;
}
static ESCX_ProcessStatus CaptureHistoryState(ESCX_StateMachine* m, ESCX_BYTE_T history_state)
{
	ESCX_BYTE_T slot, type, owner, i, state, count; ESCX_UINT16_T first, index; ESCX_BYTE_T* bits; if (m->history == (ESCX_HistoryStorage*)0)return ESCX_STATUS_INVALID_ARGUMENT; slot = HistorySlot(m, history_state); if (slot >= m->history->slot_count)return ESCX_STATUS_INVALID_ARGUMENT; bits = HistoryBits(m, slot); memset(bits, 0, m->history->bytes_per_slot); type = Type(m, history_state); owner = Parent(m, history_state); if (m->history_map != (const ESCX_HistoryMap*)0) {
		first = ESCX_ROM_READ_U16(&m->history_map->slot_first[slot]); count = ESCX_ROM_READ_U8(&m->history_map->slot_count[slot]); for (index = 0U; index < count; ++index) {
			state = ESCX_ROM_READ_U8(&m->history_map->states[first + index]); if (IsActive(m, state) != ESCX_FALSE)HistorySetBit(bits, state);
		}
	}
	else if (type == ESCX_STATE_HISTORY_SHALLOW) {
		for (i = 0U; i < m->active_atomic_count; ++i) {
			state = m->active_atomic_states[i]; while ((state != ESCX_NO_STATE) && (Parent(m, state) != owner))state = Parent(m, state); if (state != ESCX_NO_STATE)HistorySetBit(bits, state);
		}
	}
	else {
		for (state = 1U; (ESCX_UINT16_T)state < m->definition->state_count; ++state)if ((IsActive(m, state) != ESCX_FALSE) && (IsDescendantOf(m, state, owner) != ESCX_FALSE))HistorySetBit(bits, state);
	}m->history->valid[slot] = 1U; return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus CaptureHistoriesForOwner(ESCX_StateMachine* m, ESCX_BYTE_T owner)
{
	ESCX_UINT16_T first, i; ESCX_BYTE_T count, child, type; first = FirstChild(m, owner); count = ChildCount(m, owner); if ((ESCX_UINT16_T)(first + count) > m->definition->child_count)return ESCX_STATUS_INVALID_ARGUMENT; for (i = 0U; i < count; ++i) {
		child = ESCX_ROM_READ_U8(&m->definition->children[first + i]); type = Type(m, child); if ((type == ESCX_STATE_HISTORY_SHALLOW) || (type == ESCX_STATE_HISTORY_DEEP)) {
			ESCX_ProcessStatus r; r = CaptureHistoryState(m, child); if (r != ESCX_STATUS_PROCESSED)return r;
		}
	}return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus RestoreRecordedState(ESCX_StateMachine* m, ESCX_BYTE_T state, void* d)
{
	ESCX_BYTE_T type; ESCX_ProcessStatus r; if (IsActive(m, state) == ESCX_FALSE) {
		if (m->data_binding == ESCX_DATA_BINDING_LATE) {
			r = InitializeStateData(m, state, d); if (r != ESCX_STATUS_PROCESSED)return r;
		}SetActive(m, state, ESCX_TRUE); r = CallState(m, state, d, 0); if (r != ESCX_STATUS_PROCESSED)return r; r = CallState(m, state, d, 2); if (r != ESCX_STATUS_PROCESSED)return r;
	}type = Type(m, state); if ((type == ESCX_STATE_ATOMIC) || (type == ESCX_STATE_FINAL)) {
		if (AddAtomic(m, state) == ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT; if (type == ESCX_STATE_FINAL) {
			r = NotifyFinalEntered(m, state); if (r != ESCX_STATUS_PROCESSED)return r;
		}
	}return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus RestoreDeep(ESCX_StateMachine* m, ESCX_BYTE_T owner, const ESCX_BYTE_T* bits, void* d)
{
	ESCX_BYTE_T depth, state; ESCX_ProcessStatus r; for (depth = 1U; depth <= ESCX_MAX_HIERARCHY_DEPTH; ++depth) {
		for (state = 1U; (ESCX_UINT16_T)state < m->definition->state_count; ++state) {
			if ((HistoryHasBit(bits, state) != ESCX_FALSE) && (IsDescendantOf(m, state, owner) != ESCX_FALSE) && (StateDepth(m, state) == depth)) {
				r = RestoreRecordedState(m, state, d); if (r != ESCX_STATUS_PROCESSED)return r;
			}
		}
	}return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus RestoreHistory(ESCX_StateMachine* m, ESCX_BYTE_T history_state, void* d)
{
	ESCX_BYTE_T slot, owner, state, count; ESCX_UINT16_T default_plan, first, index; ESCX_BYTE_T* bits; if (m->history == (ESCX_HistoryStorage*)0)return ESCX_STATUS_INVALID_ARGUMENT; slot = HistorySlot(m, history_state); if (slot >= m->history->slot_count)return ESCX_STATUS_INVALID_ARGUMENT; default_plan = HistoryDefaultPlan(m, history_state); if (m->history->valid[slot] == 0U) {
		ESCX_ProcessStatus ar; ar = ExecuteGeneratedAction(m, &m->definition->states[history_state].history_default_action, d); if (ar != ESCX_STATUS_PROCESSED)return ar; return RunPlan(m, default_plan, d, ESCX_TRUE);
	}bits = HistoryBits(m, slot); owner = Parent(m, history_state); if (m->history_map != (const ESCX_HistoryMap*)0) {
		first = ESCX_ROM_READ_U16(&m->history_map->slot_first[slot]); count = ESCX_ROM_READ_U8(&m->history_map->slot_count[slot]); for (index = 0U; index < count; ++index) {
			state = ESCX_ROM_READ_U8(&m->history_map->states[first + index]); if (HistoryHasBit(bits, state) != ESCX_FALSE) {
				ESCX_ProcessStatus r; if (Type(m, history_state) == ESCX_STATE_HISTORY_SHALLOW)r = EnterStateTree(m, state, d, 0U); else r = RestoreRecordedState(m, state, d); if (r != ESCX_STATUS_PROCESSED)return r;
			}
		}return ESCX_STATUS_PROCESSED;
	}if (Type(m, history_state) == ESCX_STATE_HISTORY_SHALLOW) {
		for (state = 1U; (ESCX_UINT16_T)state < m->definition->state_count; ++state)if (HistoryHasBit(bits, state) != ESCX_FALSE) {
			ESCX_ProcessStatus r; r = EnterStateTree(m, state, d, 0U); if (r != ESCX_STATUS_PROCESSED)return r;
		}return ESCX_STATUS_PROCESSED;
	}return RestoreDeep(m, owner, bits, d);
}
static ESCX_ProcessStatus CaptureHistoriesForExitPlan(ESCX_StateMachine* m, ESCX_UINT16_T pid)
{
	const ESCX_StatePlan* p; ESCX_UINT16_T first, i; ESCX_BYTE_T count, state, type; if (pid == ESCX_NO_PLAN)return ESCX_STATUS_PROCESSED;
	if (pid >= m->definition->plan_count)return ESCX_STATUS_INVALID_ARGUMENT;
	p = &m->definition->plans[pid]; first = ESCX_ROM_READ_U16(&p->first_state); count = ESCX_ROM_READ_U8(&p->state_count); for (i = 0U; i < count; ++i) {
		state = ESCX_ROM_READ_U8(&m->definition->plan_states[first + i]); type = Type(m, state); if ((type == ESCX_STATE_COMPOUND) || (type == ESCX_STATE_PARALLEL)) {
			ESCX_ProcessStatus r; r = CaptureHistoriesForOwner(m, state); if (r != ESCX_STATUS_PROCESSED)return r;
		}
	}return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus RunPlan(ESCX_StateMachine* m, ESCX_UINT16_T pid, void* d, ESCX_BOOL_T entering)
{
	const ESCX_StatePlan* p; ESCX_UINT16_T first, i; ESCX_BYTE_T count, s; if (pid == ESCX_NO_PLAN)return ESCX_STATUS_PROCESSED;
	if (pid >= m->definition->plan_count)return ESCX_STATUS_INVALID_ARGUMENT;
	p = &m->definition->plans[pid]; first = ESCX_ROM_READ_U16(&p->first_state); count = ESCX_ROM_READ_U8(&p->state_count); if ((ESCX_UINT16_T)(first + count) > m->definition->plan_state_count)return ESCX_STATUS_INVALID_ARGUMENT; for (i = 0U; i < count; ++i) {
		s = ESCX_ROM_READ_U8(&m->definition->plan_states[first + i]); if (entering != ESCX_FALSE) {
			if ((Type(m, s) == ESCX_STATE_HISTORY_SHALLOW) || (Type(m, s) == ESCX_STATE_HISTORY_DEEP)) {
				ESCX_ProcessStatus hr; hr = RestoreHistory(m, s, d); if (hr != ESCX_STATUS_PROCESSED)return hr; continue;
			}if (m->data_binding == ESCX_DATA_BINDING_LATE) {
				ESCX_ProcessStatus dr; dr = InitializeStateData(m, s, d); if (dr != ESCX_STATUS_PROCESSED)return dr;
			}SetActive(m, s, ESCX_TRUE);
			{
				ESCX_ProcessStatus sr; sr = CallState(m, s, d, 0); if (sr != ESCX_STATUS_PROCESSED)return sr; sr = CallState(m, s, d, 2); if (sr != ESCX_STATUS_PROCESSED)return sr;
			}if ((Type(m, s) == ESCX_STATE_ATOMIC) || (Type(m, s) == ESCX_STATE_FINAL)) {
				if (AddAtomic(m, s) == ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT; if (Type(m, s) == ESCX_STATE_FINAL) {
					ESCX_ProcessStatus cr; cr = NotifyFinalEntered(m, s); if (cr != ESCX_STATUS_PROCESSED)return cr;
				}
			}
		}
		else {
			ESCX_ProcessStatus sr; sr = CallState(m, s, d, 1); if (sr != ESCX_STATUS_PROCESSED)return sr; SetCompletionEmitted(m, s, ESCX_FALSE); SetActive(m, s, ESCX_FALSE); RemoveAtomic(m, s);
		}
	}return ESCX_STATUS_PROCESSED;
}
static ESCX_BOOL_T PlanHasState(const ESCX_StateMachine* m, ESCX_UINT16_T pid, ESCX_BYTE_T state)
{
	const ESCX_StatePlan* p; ESCX_UINT16_T first, i; ESCX_BYTE_T count; if (pid == ESCX_NO_PLAN)return ESCX_FALSE; if (pid >= m->definition->plan_count)return ESCX_FALSE; p = &m->definition->plans[pid]; first = ESCX_ROM_READ_U16(&p->first_state); count = ESCX_ROM_READ_U8(&p->state_count); for (i = 0U; i < count; ++i)if (ESCX_ROM_READ_U8(&m->definition->plan_states[first + i]) == state)return ESCX_TRUE; return ESCX_FALSE;
}
static ESCX_BOOL_T PlanConflict(const ESCX_StateMachine* m, ESCX_UINT16_T a, ESCX_UINT16_T b)
{
	const ESCX_Transition* ta; const ESCX_Transition* tb; ESCX_UINT16_T pa, pb, i; const ESCX_StatePlan* p; ESCX_BYTE_T count, state; if (a == b)return ESCX_TRUE; ta = &m->definition->transitions[a]; tb = &m->definition->transitions[b]; pa = ESCX_ROM_READ_U16(&ta->exit_plan); pb = ESCX_ROM_READ_U16(&tb->exit_plan); if ((pa == ESCX_NO_PLAN) || (pb == ESCX_NO_PLAN))return ESCX_FALSE; p = &m->definition->plans[pa]; count = ESCX_ROM_READ_U8(&p->state_count); i = ESCX_ROM_READ_U16(&p->first_state); while (count-- != 0U) {
		state = ESCX_ROM_READ_U8(&m->definition->plan_states[i++]); if (PlanHasState(m, pb, state))return ESCX_TRUE;
	}return ESCX_FALSE;
}
static ESCX_BOOL_T Conflict(const ESCX_StateMachine* m, ESCX_UINT16_T a, ESCX_UINT16_T b)
{
	const ESCX_ConflictMap* map; unsigned long index; ESCX_BYTE_T value, mask;
	if (a == b)return ESCX_TRUE;
	map = m->conflict_map;
	if (map == (const ESCX_ConflictMap*)0)return PlanConflict(m, a, b);
	if ((a >= map->transition_count) || (b >= map->transition_count))return ESCX_FALSE;
	index = ((unsigned long)a * (unsigned long)map->bytes_per_row) + (unsigned long)(b >> 3);
	value = ESCX_ROM_READ_U8(&map->bits[index]); mask = (ESCX_BYTE_T)(1U << (b & 7U));
	return((value & mask) != 0U) ? ESCX_TRUE : ESCX_FALSE;
}
static ESCX_BYTE_T OrderOfTransition(const ESCX_StateMachine* m, ESCX_UINT16_T t)
{
	return ESCX_ROM_READ_U8(&m->definition->transitions[t].document_order);
}
static ESCX_BOOL_T SeenHas(const ESCX_BYTE_T* bits, ESCX_BYTE_T state)
{
	return((bits[state >> 3] & (ESCX_BYTE_T)(1U << (state & 7U))) != 0U) ? ESCX_TRUE : ESCX_FALSE;
}
static void SeenAdd(ESCX_BYTE_T* bits, ESCX_BYTE_T state)
{
	bits[state >> 3] = (ESCX_BYTE_T)(bits[state >> 3] | (ESCX_BYTE_T)(1U << (state & 7U)));
}
static ESCX_ProcessStatus InsertOrderedState(ESCX_StateMachine* m, ESCX_BYTE_T state, ESCX_BOOL_T entering)
{
	ESCX_MicrostepWorkspace* w; ESCX_BYTE_T* states; ESCX_BYTE_T* count; ESCX_BYTE_T capacity, i, depth, order, other_depth, other_order; ESCX_BYTE_T* seen;
	w = m->workspace; states = (entering != ESCX_FALSE) ? w->entry_states : w->exit_states; count = (entering != ESCX_FALSE) ? &w->entry_count : &w->exit_count; capacity = (entering != ESCX_FALSE) ? w->entry_capacity : w->exit_capacity; seen = (entering != ESCX_FALSE) ? w->entry_seen : w->exit_seen;
	if (SeenHas(seen, state) != ESCX_FALSE)return ESCX_STATUS_PROCESSED;
	if (*count >= capacity)return ESCX_STATUS_INVALID_ARGUMENT;
	depth = StateDepth(m, state); order = StateOrder(m, state); i = *count;
	while (i != 0U) {
		other_depth = StateDepth(m, states[i - 1U]); other_order = StateOrder(m, states[i - 1U]); if (entering != ESCX_FALSE) {
			if ((other_depth < depth) || ((other_depth == depth) && (other_order <= order)))break;
		}
		else {
			if ((other_depth > depth) || ((other_depth == depth) && (other_order >= order)))break;
		}states[i] = states[i - 1U]; --i;
	}
	states[i] = state; *count = (ESCX_BYTE_T)(*count + 1U); SeenAdd(seen, state); return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus MergePlan(ESCX_StateMachine* m, ESCX_UINT16_T pid, ESCX_BOOL_T entering)
{
	const ESCX_StatePlan* p; ESCX_UINT16_T first, i; ESCX_BYTE_T count, state; ESCX_ProcessStatus r; if (pid == ESCX_NO_PLAN)return ESCX_STATUS_PROCESSED; if (pid >= m->definition->plan_count)return ESCX_STATUS_INVALID_ARGUMENT; p = &m->definition->plans[pid]; first = ESCX_ROM_READ_U16(&p->first_state); count = ESCX_ROM_READ_U8(&p->state_count); if ((ESCX_UINT16_T)(first + count) > m->definition->plan_state_count)return ESCX_STATUS_INVALID_ARGUMENT;
	for (i = 0U; i < count; ++i) {
		state = ESCX_ROM_READ_U8(&m->definition->plan_states[first + i]); if (state != ESCX_NO_STATE) {
			r = InsertOrderedState(m, state, entering); if (r != ESCX_STATUS_PROCESSED)return r;
		}
	}return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus ExecuteWorkspaceExits(ESCX_StateMachine* m, void* d)
{
	ESCX_BYTE_T i, state; ESCX_ProcessStatus r; for (i = 0U; i < m->workspace->exit_count; ++i) {
		state = m->workspace->exit_states[i]; if (IsActive(m, state) != ESCX_FALSE) {
			r = CallState(m, state, d, 1); if (r != ESCX_STATUS_PROCESSED)return r; SetCompletionEmitted(m, state, ESCX_FALSE); SetActive(m, state, ESCX_FALSE); RemoveAtomic(m, state);
		}
	}return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus ExecuteWorkspaceEntries(ESCX_StateMachine* m, void* d)
{
	ESCX_BYTE_T i, state, type; ESCX_ProcessStatus r; for (i = 0U; i < m->workspace->entry_count; ++i) {
		state = m->workspace->entry_states[i]; type = Type(m, state); if ((type == ESCX_STATE_HISTORY_SHALLOW) || (type == ESCX_STATE_HISTORY_DEEP)) {
			r = RestoreHistory(m, state, d); if (r != ESCX_STATUS_PROCESSED)return r; continue;
		}if (IsActive(m, state) == ESCX_FALSE) {
			if (m->data_binding == ESCX_DATA_BINDING_LATE) {
				r = InitializeStateData(m, state, d); if (r != ESCX_STATUS_PROCESSED)return r;
			}SetActive(m, state, ESCX_TRUE); r = CallState(m, state, d, 0); if (r != ESCX_STATUS_PROCESSED)return r; r = CallState(m, state, d, 2); if (r != ESCX_STATUS_PROCESSED)return r;
		}if ((type == ESCX_STATE_ATOMIC) || (type == ESCX_STATE_FINAL)) {
			if (AddAtomic(m, state) == ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT; if (type == ESCX_STATE_FINAL) {
				r = NotifyFinalEntered(m, state); if (r != ESCX_STATUS_PROCESSED)return r;
			}
		}
	}return ESCX_STATUS_PROCESSED;
}
static void SortTransitionsByDocumentOrder(const ESCX_StateMachine* m, ESCX_UINT16_T* ids, ESCX_BYTE_T count)
{
	ESCX_BYTE_T i, j; ESCX_UINT16_T value; for (i = 1U; i < count; ++i) {
		value = ids[i]; j = i; while ((j != 0U) && (OrderOfTransition(m, ids[j - 1U]) > OrderOfTransition(m, value))) {
			ids[j] = ids[j - 1U]; --j;
		}ids[j] = value;
	}
}
static ESCX_ProcessStatus ExecuteSet(ESCX_StateMachine* m, ESCX_UINT16_T* ids, ESCX_BYTE_T count, void* d)
{
	ESCX_BYTE_T i, type; ESCX_UINT16_T xp, ep; ESCX_ActionFunction action; ESCX_ProcessStatus r; const ESCX_Transition* t; ESCX_MicrostepWorkspace* w;
	ESCX_BYTE_T fallback_exit[ESCX_MAX_STATES]; ESCX_BYTE_T fallback_entry[ESCX_MAX_STATES];
	ESCX_BYTE_T fallback_exit_seen[ESCX_ACTIVE_BYTES(ESCX_MAX_STATES)]; ESCX_BYTE_T fallback_entry_seen[ESCX_ACTIVE_BYTES(ESCX_MAX_STATES)]; ESCX_MicrostepWorkspace fallback; ESCX_MicrostepWorkspace* saved;
	if (count == 0U)return ESCX_STATUS_IGNORED;
	m->last_macrostep_result.transition_taken = ESCX_TRUE;
	if (m->workspace == (ESCX_MicrostepWorkspace*)0) {
		fallback.exit_states = fallback_exit; fallback.exit_capacity = (ESCX_BYTE_T)ESCX_MAX_STATES; fallback.exit_count = 0U; fallback.entry_states = fallback_entry; fallback.entry_capacity = (ESCX_BYTE_T)ESCX_MAX_STATES; fallback.entry_count = 0U; fallback.exit_seen = fallback_exit_seen; fallback.entry_seen = fallback_entry_seen; fallback.seen_byte_count = (ESCX_BYTE_T)ESCX_ACTIVE_BYTES(ESCX_MAX_STATES); saved = m->workspace; m->workspace = &fallback; r = ExecuteSet(m, ids, count, d); m->workspace = saved; return r;
	}
	w = m->workspace; w->exit_count = 0U; w->entry_count = 0U; memset(w->exit_seen, 0, w->seen_byte_count); memset(w->entry_seen, 0, w->seen_byte_count);
	for (i = 0U; i < count; ++i) {
		if (ids[i] >= m->definition->transition_count)return ESCX_STATUS_INVALID_ARGUMENT; t = &m->definition->transitions[ids[i]]; type = ESCX_ROM_READ_U8(&t->type); if (type != ESCX_TRANSITION_TARGETLESS) {
			xp = ESCX_ROM_READ_U16(&t->exit_plan); ep = ESCX_ROM_READ_U16(&t->entry_plan); r = CaptureHistoriesForExitPlan(m, xp); if (r != ESCX_STATUS_PROCESSED)return r; r = MergePlan(m, xp, ESCX_FALSE); if (r != ESCX_STATUS_PROCESSED)return r; r = MergePlan(m, ep, ESCX_TRUE); if (r != ESCX_STATUS_PROCESSED)return r;
		}
	}
	r = ExecuteWorkspaceExits(m, d); if (r != ESCX_STATUS_PROCESSED)return r; SortTransitionsByDocumentOrder(m, ids, count);
	for (i = 0U; i < count; ++i) {
		ESCX_StatusActionFunction checked; int action_status; t = &m->definition->transitions[ids[i]]; checked = ReadStatusAction(&t->status_action); if (checked != (ESCX_StatusActionFunction)0) {
			action_status = checked(m, d); if (action_status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, (ESCX_ProcessStatus)action_status);
		}
		else {
			action = ReadAction(&t->action); if (action != (ESCX_ActionFunction)0)action(m, d);
		}
	}
	r = ExecuteWorkspaceEntries(m, d); if (r == ESCX_STATUS_PROCESSED)++m->last_macrostep_result.microstep_count; return r;
}
static ESCX_BOOL_T TransitionPreempts(const ESCX_StateMachine* m, ESCX_UINT16_T candidate, ESCX_UINT16_T existing)
{
	if ((m->preemption_map != (const ESCX_PreemptionMap*)0) && (candidate < m->preemption_map->transition_count) && (existing < m->preemption_map->transition_count))return PreemptionMapBit(m->preemption_map, candidate, existing); return NormativeTransitionPreempts(m, candidate, existing);
}
static ESCX_ProcessStatus AddEnabledTransition(const ESCX_StateMachine* m, ESCX_UINT16_T* selected, ESCX_BYTE_T* count, ESCX_UINT16_T candidate)
{
	ESCX_BYTE_T i, write; ESCX_BOOL_T rejected;
	for (i = 0U; i < *count; ++i)if (selected[i] == candidate)return ESCX_STATUS_PROCESSED;
	rejected = ESCX_FALSE;
	for (i = 0U; i < *count; ++i) {
		if ((Conflict(m, candidate, selected[i]) != ESCX_FALSE) && (TransitionPreempts(m, candidate, selected[i]) == ESCX_FALSE)) {
			rejected = ESCX_TRUE; break;
		}
	}
	if (rejected != ESCX_FALSE)return ESCX_STATUS_PROCESSED;
	write = 0U;
	for (i = 0U; i < *count; ++i) {
		if ((Conflict(m, candidate, selected[i]) != ESCX_FALSE) && (TransitionPreempts(m, candidate, selected[i]) != ESCX_FALSE))continue;
		selected[write++] = selected[i];
	}
	if (write >= ESCX_MAX_ENABLED_TRANSITIONS)return ESCX_STATUS_INVALID_ARGUMENT;
	selected[write++] = candidate; *count = write; return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureEventMatching(ESCX_StateMachine* m, const ESCX_BYTE_T* parents, ESCX_BYTE_T count, ESCX_BYTE_T wildcard_event)
{
	ESCX_BYTE_T i, parent, current, steps;
	if ((m == (ESCX_StateMachine*)0) || (m->definition == (const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	if (m->running != ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;
	if ((count != 0U) && (parents == (const ESCX_BYTE_T*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	if (count < m->definition->event_count)return ESCX_STATUS_INVALID_ARGUMENT;
	if ((wildcard_event != ESCX_NO_EVENT) && (wildcard_event >= m->definition->event_count))return ESCX_STATUS_INVALID_EVENT;
	for (i = 0U; i < m->definition->event_count; ++i) {
		parent = ESCX_ROM_READ_U8(&parents[i]);
		if ((parent != ESCX_NO_EVENT) && (parent >= m->definition->event_count))return ESCX_STATUS_INVALID_EVENT;
		current = i; steps = 0U;
		while (current != ESCX_NO_EVENT) {
			if (current >= m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;
			parent = ESCX_ROM_READ_U8(&parents[current]);
			if (parent == ESCX_NO_EVENT)break;
			if ((parent == current) || (parent == i))return ESCX_STATUS_INVALID_ARGUMENT;
			current = parent;
			++steps;
			if (steps >= m->definition->event_count)return ESCX_STATUS_INVALID_ARGUMENT;
		}
	}
	m->event_parent = parents; m->event_parent_count = count; m->wildcard_event = wildcard_event; return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureSystemContext(ESCX_StateMachine* m, const ESCX_SystemContext* context)
{
	if ((m == (ESCX_StateMachine*)0) || (context == (const ESCX_SystemContext*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	if (m->running != ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT;
	m->system_context = *context; return ESCX_STATUS_PROCESSED;
}
const ESCX_SystemContext* ESCX_StateMachineGetSystemContext(const ESCX_StateMachine* m)
{
	if (m == (const ESCX_StateMachine*)0)return (const ESCX_SystemContext*)0;
	return &m->system_context;
}
static void SetCurrentEvent(ESCX_StateMachine* m, const ESCX_Event* event)
{
	m->current_event = event->name_id; m->current_event_descriptor = *event;
}
static void SetNeutralCurrentEvent(ESCX_StateMachine* m)
{
	m->current_event = ESCX_NO_EVENT; m->current_event_descriptor.name_id = ESCX_NO_EVENT;
	m->current_event_descriptor.type = ESCX_EVENT_TYPE_INTERNAL; m->current_event_descriptor.origin_type = ESCX_EVENT_ORIGIN_NONE;
	m->current_event_descriptor.invoke_id = ESCX_NO_INVOKE; m->current_event_descriptor.send_id = ESCX_NO_SEND_ID; m->current_event_descriptor.data = (void*)0;
}
static void SetTriggeringEvent(ESCX_StateMachine* m, const ESCX_Event* event)
{
	m->triggering_event_descriptor = *event; m->has_triggering_event = ESCX_TRUE; SetCurrentEvent(m, event);
}
const ESCX_Event* ESCX_StateMachineGetCurrentEvent(const ESCX_StateMachine* m)
{
	if (m == (const ESCX_StateMachine*)0)return (const ESCX_Event*)0;
	return &m->current_event_descriptor;
}
static ESCX_ProcessStatus ProcessEvent(ESCX_StateMachine* m, ESCX_BYTE_T e, void* d)
{
	ESCX_UINT16_T selected[ESCX_MAX_ENABLED_TRANSITIONS]; ESCX_BYTE_T selected_count = 0U, leaf, state, depth, i;
	ESCX_UINT16_T selector_id, transition_id; ESCX_SelectorFunction selector; ESCX_ProcessStatus status;
	if (e >= m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;
	m->current_event = e; m->current_event_descriptor.name_id = e; m->current_event_descriptor.data = d;
	for (i = 0U; i < m->active_atomic_count; ++i) {
		leaf = m->active_atomic_states[i]; state = leaf; depth = 0U;
		if (m->fast_dispatch != (const ESCX_FastDispatch*)0) {
			selector_id = FindFastSelector(m, leaf, e); if ((selector_id != ESCX_NO_SELECTOR) && (selector_id < m->definition->selector_count)) {
				selector = ReadSelector(&m->definition->selectors[selector_id]); if (selector == (ESCX_SelectorFunction)0)return ESCX_STATUS_INVALID_ARGUMENT; transition_id = selector(m, d); if (transition_id != ESCX_NO_TRANSITION) {
					status = AddEnabledTransition(m, selected, &selected_count, transition_id); if (status != ESCX_STATUS_PROCESSED)return status;
				}
			}continue;
		}
		while (state != ESCX_NO_STATE) {
			selector_id = FindMatchingSelector(m, state, e);
			if ((selector_id != ESCX_NO_SELECTOR) && (selector_id < m->definition->selector_count)) {
				selector = ReadSelector(&m->definition->selectors[selector_id]); if (selector == (ESCX_SelectorFunction)0)return ESCX_STATUS_INVALID_ARGUMENT; transition_id = selector(m, d);
				if (transition_id != ESCX_NO_TRANSITION) {
					status = AddEnabledTransition(m, selected, &selected_count, transition_id); if (status != ESCX_STATUS_PROCESSED)return status; break;
				}
			}
			state = Parent(m, state); if (++depth > ESCX_MAX_HIERARCHY_DEPTH)return ESCX_STATUS_HIERARCHY_LIMIT;
		}
	}
	return ExecuteSet(m, selected, selected_count, d);
}
static ESCX_UINT16_T FindEffectiveEventlessSelector(const ESCX_StateMachine* m, ESCX_BYTE_T leaf)
{
	const ESCX_FastDispatch* fast; ESCX_BYTE_T state, depth; ESCX_UINT16_T selector;
	fast = m->fast_dispatch;
	if ((fast != (const ESCX_FastDispatch*)0) && (fast->eventless_selector != (const ESCX_UINT16_T*)0) && (leaf < fast->eventless_count))return ESCX_ROM_READ_U16(&fast->eventless_selector[leaf]);
	state = leaf; depth = 0U;
	while (state != ESCX_NO_STATE) {
		selector = EventlessSelector(m, state); if (selector != ESCX_NO_SELECTOR)return selector; state = Parent(m, state); if (++depth > ESCX_MAX_HIERARCHY_DEPTH)return ESCX_NO_SELECTOR;
	}
	return ESCX_NO_SELECTOR;
}
static ESCX_ProcessStatus ProcessEventless(ESCX_StateMachine* m)
{
	ESCX_UINT16_T selected[ESCX_MAX_ENABLED_TRANSITIONS]; ESCX_BYTE_T selected_count, i, leaf; ESCX_UINT16_T selector_id, transition_id; ESCX_SelectorFunction selector; ESCX_ProcessStatus status;
	selected_count = 0U; SetNeutralCurrentEvent(m);
	for (i = 0U; i < m->active_atomic_count; ++i) {
		leaf = m->active_atomic_states[i]; selector_id = FindEffectiveEventlessSelector(m, leaf);
		if (selector_id == ESCX_NO_SELECTOR)continue;
		if (selector_id >= m->definition->selector_count)return ESCX_STATUS_INVALID_ARGUMENT;
		selector = ReadSelector(&m->definition->selectors[selector_id]); if (selector == (ESCX_SelectorFunction)0)return ESCX_STATUS_INVALID_ARGUMENT;
		transition_id = selector(m, (void*)0); if (transition_id == ESCX_NO_TRANSITION)continue;
		status = AddEnabledTransition(m, selected, &selected_count, transition_id); if (status != ESCX_STATUS_PROCESSED)return status;
	}
	return ExecuteSet(m, selected, selected_count, (void*)0);
}
static ESCX_BOOL_T IsPendingErrorEvent(const ESCX_StateMachine* m, const ESCX_QueuedEvent* q)
{
#if ESCX_ENABLE_EVENT_DATA
	return ((m->reporting_error != ESCX_FALSE) && (q->event_type == ESCX_EVENT_TYPE_PLATFORM) && (q->origin_type == ESCX_EVENT_ORIGIN_PLATFORM) && (q->event_data == (void*)&m->pending_error_event_data)) ? ESCX_TRUE : ESCX_FALSE;
#else
	(void)m; (void)q; return ESCX_FALSE;
#endif
}
static ESCX_ProcessStatus Stabilize(ESCX_StateMachine* m)
{
	ESCX_UINT16_T steps = 0U; ESCX_ProcessStatus r; ESCX_QueuedEvent q; ESCX_BOOL_T pending_error; for (;;) {
		r = ProcessEventless(m); if (r == ESCX_STATUS_PROCESSED) {
			if (++steps > ESCX_MAX_MICROSTEPS)return ESCX_STATUS_MICROSTEP_LIMIT; continue;
		}if (r != ESCX_STATUS_IGNORED)return r; if (ESCX_EventQueuePop(&m->internal_queue, &q) != ESCX_FALSE) {
			pending_error = IsPendingErrorEvent(m, &q);
#if ESCX_ENABLE_EVENT_DATA
			{
				ESCX_Event event; event.name_id = q.event_id; event.type = q.event_type; event.origin_type = q.origin_type; event.invoke_id = q.invoke_id; event.send_id = q.send_id; event.data = q.event_data; SetTriggeringEvent(m, &event);
			}r = ProcessEvent(m, q.event_id, q.event_data);
#else
			r = ProcessEvent(m, q.event_id, (void*)0);
#endif
			if (pending_error != ESCX_FALSE)m->reporting_error = ESCX_FALSE;
			if ((r != ESCX_STATUS_PROCESSED) && (r != ESCX_STATUS_IGNORED)) {
				return r;
			}
			if (++steps > ESCX_MAX_MICROSTEPS) {
				return ESCX_STATUS_MICROSTEP_LIMIT;
			}
			continue;
		}r = StartPendingInvokes(m); if (r != ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, r); return ESCX_STATUS_PROCESSED;
	}
}
static ESCX_ProcessStatus Macrostep(ESCX_StateMachine* m, ESCX_BYTE_T e, void* d)
{
	ESCX_ProcessStatus r; m->last_macrostep_result.event_accepted = ESCX_TRUE; m->last_macrostep_result.transition_taken = ESCX_FALSE; m->last_macrostep_result.microstep_count = 0U; r = ForwardExternalToInvokes(m, e, d); if (r != ESCX_STATUS_PROCESSED)return r; r = ProcessEvent(m, e, d); if ((r != ESCX_STATUS_PROCESSED) && (r != ESCX_STATUS_IGNORED))return r; r = Stabilize(m); return r;
}
static void CancelDelayedSends(ESCX_StateMachine* m)
{
#if ESCX_ENABLE_DELAYED_SENDS
	ESCX_BYTE_T i; for (i = 0U; i < ESCX_MAX_DELAYED_SENDS; ++i) {
		if ((m->delayed_sends[i].active != ESCX_FALSE) && (m->cancel_timer != (ESCX_CancelTimerFunction)0))(void)m->cancel_timer(m, m->delayed_sends[i].timer_id, m->timer_context); m->delayed_sends[i].active = ESCX_FALSE; m->delayed_sends[i].timer_id = ESCX_INVALID_TIMER_ID;
	}
#else
	(void)m;
#endif
}
static void FinishInterpreter(ESCX_StateMachine* m)
{
	if ((m == (ESCX_StateMachine*)0) || (m->finish_pending == ESCX_FALSE))return;
	ExitCompleteConfiguration(m, (void*)0); CancelDelayedSends(m); ESCX_EventQueueInit(&m->internal_queue); ESCX_EventQueueInit(&m->external_queue);
	memset(m->active_bits, 0, m->active_byte_count); if (m->completion_bits != (ESCX_BYTE_T*)0)memset(m->completion_bits, 0, m->completion_byte_count); m->active_atomic_count = 0U; m->running = ESCX_FALSE; m->processing = ESCX_FALSE; m->finished = ESCX_TRUE; m->finish_pending = ESCX_FALSE;
}
static void RollbackFailedStart(ESCX_StateMachine* m)
{
	ESCX_BYTE_T i;
	if (m == (ESCX_StateMachine*)0)return;
	ExitCompleteConfiguration(m, (void*)0);
	CancelDelayedSends(m);
	ESCX_EventQueueInit(&m->internal_queue); ESCX_EventQueueInit(&m->external_queue);
	memset(m->active_bits, 0, m->active_byte_count);
	if (m->completion_bits != (ESCX_BYTE_T*)0)memset(m->completion_bits, 0, m->completion_byte_count);
	m->active_atomic_count = 0U;
	for (i = 0U; i < ESCX_MAX_INVOKES; ++i)m->pending_invokes[i] = ESCX_FALSE;
	if (m->invoke_instances != (ESCX_InvokeInstance*)0)for (i = 0U; i < m->invoke_count; ++i)m->invoke_instances[i].active = ESCX_FALSE;
	m->running = ESCX_FALSE; m->processing = ESCX_FALSE; m->finished = ESCX_FALSE; m->finish_pending = ESCX_FALSE; m->reporting_error = ESCX_FALSE;
	SetNeutralCurrentEvent(m); m->triggering_event_descriptor = m->current_event_descriptor; m->has_triggering_event = ESCX_FALSE;
}
ESCX_ProcessStatus ESCX_StateMachineStart(ESCX_StateMachine* m)
{
	ESCX_ProcessStatus r; if ((m == (ESCX_StateMachine*)0) || (m->definition == (const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT; if (m->running != ESCX_FALSE)return ESCX_STATUS_IGNORED; memset(m->active_bits, 0, m->active_byte_count); if (m->completion_bits != (ESCX_BYTE_T*)0)memset(m->completion_bits, 0, m->completion_byte_count); m->active_atomic_count = 0U;
	{
		ESCX_BYTE_T pi; for (pi = 0U; pi < ESCX_MAX_INVOKES; ++pi)m->pending_invokes[pi] = ESCX_FALSE;
	}ESCX_EventQueueInit(&m->internal_queue); ESCX_EventQueueInit(&m->external_queue); SetNeutralCurrentEvent(m); m->triggering_event_descriptor = m->current_event_descriptor; m->has_triggering_event = ESCX_FALSE; m->last_macrostep_result.event_accepted = ESCX_FALSE; m->last_macrostep_result.transition_taken = ESCX_FALSE; m->last_macrostep_result.microstep_count = 0U; m->finished = ESCX_FALSE; m->finish_pending = ESCX_FALSE; m->reporting_error = ESCX_FALSE; m->running = ESCX_TRUE; if (m->data_initialized_bits != (ESCX_BYTE_T*)0) {
		memset(m->data_initialized_bits, 0, m->data_initialized_byte_count); if (m->data_binding == ESCX_DATA_BINDING_EARLY) {
			r = InitializeAllData(m); if (r != ESCX_STATUS_PROCESSED) {
				RollbackFailedStart(m); return r;
			}
		}
	}r = EnterInitial(m, m->definition->initial_state, (void*)0); if (r != ESCX_STATUS_PROCESSED) {
		RollbackFailedStart(m); return r;
	}m->processing = ESCX_TRUE; r = Stabilize(m); m->processing = ESCX_FALSE; if (r != ESCX_STATUS_PROCESSED) {
		RollbackFailedStart(m); return r;
	}if (m->finish_pending != ESCX_FALSE)FinishInterpreter(m); return r;
}
ESCX_ProcessStatus ESCX_StateMachineRaiseEvent(ESCX_StateMachine* m, const ESCX_Event* event)
{
	if ((m == (ESCX_StateMachine*)0) || (event == (const ESCX_Event*)0) || (m->running == ESCX_FALSE))return ESCX_STATUS_NOT_RUNNING; if (event->name_id >= m->definition->event_count)return ESCX_STATUS_INVALID_EVENT; return ESCX_EventQueuePushEx(&m->internal_queue, event->name_id, event->type, event->origin_type, event->invoke_id, event->send_id, event->data) ? ESCX_STATUS_PROCESSED : ESCX_STATUS_QUEUE_FULL;
}
ESCX_ProcessStatus ESCX_StateMachineRaiseInternal(ESCX_StateMachine* m, ESCX_BYTE_T e, void* d)
{
	ESCX_Event event; event.name_id = e; event.type = ESCX_EVENT_TYPE_INTERNAL; event.origin_type = ESCX_EVENT_ORIGIN_SCXML; event.invoke_id = ESCX_NO_INVOKE; event.send_id = ESCX_NO_SEND_ID; event.data = d; return ESCX_StateMachineRaiseEvent(m, &event);
}
const ESCX_MacrostepResult* ESCX_StateMachineGetLastMacrostepResult(const ESCX_StateMachine* m)
{
	if (m == (const ESCX_StateMachine*)0)return (const ESCX_MacrostepResult*)0; return &m->last_macrostep_result;
}
ESCX_ProcessStatus ESCX_StateMachineSendEvent(ESCX_StateMachine* m, const ESCX_Event* event)
{
	ESCX_ProcessStatus r; ESCX_QueuedEvent q; if ((m == (ESCX_StateMachine*)0) || (event == (const ESCX_Event*)0) || (m->definition == (const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT; if (m->running == ESCX_FALSE)return ESCX_STATUS_NOT_RUNNING; if (event->name_id >= m->definition->event_count)return ESCX_STATUS_INVALID_EVENT; if (m->processing != ESCX_FALSE)return ESCX_EventQueuePushEx(&m->external_queue, event->name_id, event->type, event->origin_type, event->invoke_id, event->send_id, event->data) ? ESCX_STATUS_PROCESSED : ESCX_STATUS_QUEUE_FULL; m->processing = ESCX_TRUE; SetTriggeringEvent(m, event); r = Macrostep(m, event->name_id, event->data); while ((r == ESCX_STATUS_PROCESSED) && (m->finish_pending == ESCX_FALSE) && (ESCX_EventQueuePop(&m->external_queue, &q) != ESCX_FALSE)) {
#if ESCX_ENABLE_EVENT_DATA
		{
			ESCX_Event queued; queued.name_id = q.event_id; queued.type = q.event_type; queued.origin_type = q.origin_type; queued.invoke_id = q.invoke_id; queued.send_id = q.send_id; queued.data = q.event_data; SetTriggeringEvent(m, &queued);
		}r = Macrostep(m, q.event_id, q.event_data);
#else
		r = Macrostep(m, q.event_id, (void*)0);
#endif
	}m->processing = ESCX_FALSE; if ((r == ESCX_STATUS_PROCESSED) && (m->finish_pending != ESCX_FALSE))FinishInterpreter(m); return r;
}
ESCX_ProcessStatus ESCX_StateMachineSend(ESCX_StateMachine* m, ESCX_BYTE_T e, void* d)
{
	ESCX_Event event; event.name_id = e; event.type = ESCX_EVENT_TYPE_EXTERNAL; event.origin_type = ESCX_EVENT_ORIGIN_PLATFORM; event.invoke_id = ESCX_NO_INVOKE; event.send_id = ESCX_NO_SEND_ID; event.data = d; return ESCX_StateMachineSendEvent(m, &event);
}
static void ExitCompleteConfiguration(ESCX_StateMachine* m, void* d)
{
	ESCX_BYTE_T depth, state, best, best_order; ESCX_UINT16_T index;
	if (m->exit_order != (const ESCX_ExitOrder*)0) {
		for (index = 0U; index < m->exit_order->state_count; ++index) {
			state = ESCX_ROM_READ_U8(&m->exit_order->states[index]); if (IsActive(m, state) != ESCX_FALSE) {
				(void)CallState(m, state, d, 1); SetCompletionEmitted(m, state, ESCX_FALSE); SetActive(m, state, ESCX_FALSE); RemoveAtomic(m, state);
			}
		}return;
	}
	for (depth = ESCX_MAX_HIERARCHY_DEPTH;; --depth) {
		for (;;) {
			best = ESCX_NO_STATE; best_order = 0U;
			for (state = 1U; (ESCX_UINT16_T)state < m->definition->state_count; ++state) {
				if ((IsActive(m, state) != ESCX_FALSE) && (StateDepth(m, state) == depth)) {
					if ((best == ESCX_NO_STATE) || (StateOrder(m, state) > best_order)) {
						best = state; best_order = StateOrder(m, state);
					}
				}
			}
			if (best == ESCX_NO_STATE)break;
			(void)CallState(m, best, d, 1); SetCompletionEmitted(m, best, ESCX_FALSE); SetActive(m, best, ESCX_FALSE); RemoveAtomic(m, best);
		}
		if (depth == 0U)break;
	}
}
void ESCX_StateMachineStop(ESCX_StateMachine* m)
{
	if ((m == (ESCX_StateMachine*)0) || (m->running == ESCX_FALSE))return;
	ExitCompleteConfiguration(m, (void*)0);
	memset(m->active_bits, 0, m->active_byte_count); if (m->completion_bits != (ESCX_BYTE_T*)0)memset(m->completion_bits, 0, m->completion_byte_count); m->active_atomic_count = 0U;
	m->running = ESCX_FALSE; m->processing = ESCX_FALSE; m->finished = ESCX_FALSE; m->finish_pending = ESCX_FALSE;
	m->current_event = ESCX_NO_EVENT; m->current_event_descriptor.name_id = ESCX_NO_EVENT;
	m->current_event_descriptor.type = ESCX_EVENT_TYPE_INTERNAL; m->current_event_descriptor.origin_type = ESCX_EVENT_ORIGIN_NONE;
	m->current_event_descriptor.invoke_id = ESCX_NO_INVOKE; m->current_event_descriptor.send_id = ESCX_NO_SEND_ID; m->current_event_descriptor.data = (void*)0; m->triggering_event_descriptor = m->current_event_descriptor; m->has_triggering_event = ESCX_FALSE; m->last_macrostep_result.event_accepted = ESCX_FALSE; m->last_macrostep_result.transition_taken = ESCX_FALSE; m->last_macrostep_result.microstep_count = 0U;
	ESCX_EventQueueInit(&m->internal_queue); ESCX_EventQueueInit(&m->external_queue);
	CancelDelayedSends(m);
}
static ESCX_ProcessStatus ReportErrorInternal(ESCX_StateMachine*, ESCX_ErrorCode, ESCX_ProcessStatus);
static ESCX_ProcessStatus DispatchSend(ESCX_StateMachine*, ESCX_BYTE_T, void*, ESCX_BYTE_T, ESCX_BYTE_T);
ESCX_ProcessStatus ESCX_StateMachineConfigureTimers(ESCX_StateMachine* m, ESCX_StartTimerFunction st, ESCX_CancelTimerFunction ct, void* ctx)
{
#if ESCX_ENABLE_TIMERS
	if (m == (ESCX_StateMachine*)0)return ESCX_STATUS_INVALID_ARGUMENT;
	m->start_timer = st; m->cancel_timer = ct; m->timer_context = ctx;
	return ESCX_STATUS_PROCESSED;
#else
	(void)m; (void)st; (void)ct; (void)ctx; return ESCX_STATUS_IGNORED;
#endif
}
ESCX_ProcessStatus ESCX_StateMachineTimerExpired(ESCX_StateMachine* m, ESCX_TimerId id)
{
#if ESCX_ENABLE_TIMERS && ESCX_ENABLE_DELAYED_SENDS
	ESCX_BYTE_T i; ESCX_DelayedSend pending; if (m == (ESCX_StateMachine*)0)return ESCX_STATUS_INVALID_ARGUMENT; if (m->running == ESCX_FALSE)return ESCX_STATUS_NOT_RUNNING; if (id == ESCX_INVALID_TIMER_ID)return ESCX_STATUS_IGNORED;
	for (i = 0U; i < ESCX_MAX_DELAYED_SENDS; ++i)if ((m->delayed_sends[i].active != ESCX_FALSE) && (m->delayed_sends[i].timer_id == id)) {
		pending = m->delayed_sends[i]; m->delayed_sends[i].active = ESCX_FALSE; m->delayed_sends[i].timer_id = ESCX_INVALID_TIMER_ID;
		if (pending.uses_target_descriptor != ESCX_FALSE) {
			ESCX_Event delayed_event; delayed_event.name_id = pending.event_id; delayed_event.type = pending.event_type; delayed_event.origin_type = pending.origin_type; delayed_event.invoke_id = pending.invoke_id; delayed_event.send_id = pending.send_id; delayed_event.data = pending.event_data; return ESCX_StateMachineSendToTarget(m, &pending.target_descriptor, &delayed_event);
		}return DispatchSend(m, pending.event_id, pending.event_data, pending.target, pending.target_id);
	}
	return ESCX_STATUS_IGNORED;
#else
	(void)m; (void)id; return ESCX_STATUS_IGNORED;
#endif
}
ESCX_ProcessStatus ESCX_StateMachineConfigureIOProcessor(ESCX_StateMachine* m, ESCX_IOProcessorSendFunction send, void* context)
{
	if (m == (ESCX_StateMachine*)0)return ESCX_STATUS_INVALID_ARGUMENT; if (m->running != ESCX_FALSE)return ESCX_STATUS_INVALID_ARGUMENT; m->io_processor_send = send; m->io_processor_context = context; return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineSendToTarget(ESCX_StateMachine* m, const ESCX_SendTargetDescriptor* t, const ESCX_Event* e)
{
	ESCX_InvokeDescriptor d; int status; if ((m == (ESCX_StateMachine*)0) || (t == (const ESCX_SendTargetDescriptor*)0) || (e == (const ESCX_Event*)0))return ESCX_STATUS_INVALID_ARGUMENT; if (m->running == ESCX_FALSE)return ESCX_STATUS_NOT_RUNNING; if (e->name_id >= m->definition->event_count)return ESCX_STATUS_INVALID_EVENT; if (t->processor == ESCX_IO_PROCESSOR_INTERNAL) {
		ESCX_Event internal_event;
		if (t->target_type != ESCX_IO_TARGET_INTERNAL)return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET);
		internal_event = *e; internal_event.type = ESCX_EVENT_TYPE_INTERNAL; internal_event.origin_type = ESCX_EVENT_ORIGIN_SCXML; internal_event.invoke_id = ESCX_NO_INVOKE;
		return ESCX_StateMachineRaiseEvent(m, &internal_event);
	}if (t->processor == ESCX_IO_PROCESSOR_SCXML) {
		if (t->target_type == ESCX_IO_TARGET_CURRENT_SESSION)return ESCX_StateMachineSendEvent(m, e); if (t->target_type == ESCX_IO_TARGET_PARENT_SESSION) {
			if ((m->parent_machine == (ESCX_StateMachine*)0) || (m->parent_invoke_id == ESCX_NO_INVOKE))return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_NO_PARENT); return ESCX_StateMachineReceiveInvokeEventDescriptor(m->parent_machine, m->parent_invoke_id, e);
		}if (t->target_type == ESCX_IO_TARGET_INVOKE) {
			if ((t->target_id >= m->invoke_count) || (m->invoke_instances == (ESCX_InvokeInstance*)0) || (m->invoke_instances[t->target_id].active == ESCX_FALSE))return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET); ReadInvokeDescriptor(m, (ESCX_BYTE_T)t->target_id, &d); if (d.forward_event != (ESCX_InvokeForwardEventFunction)0)status = d.forward_event(m, d.child_storage, e); else if (d.forward != (ESCX_InvokeForwardFunction)0)status = d.forward(m, d.child_storage, e->name_id, e->data); else return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET); if (status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, (ESCX_ProcessStatus)status); return ESCX_STATUS_PROCESSED;
		}if (t->target_type == ESCX_IO_TARGET_SESSION) {
			if ((t->target_id == m->system_context.session_id) || (t->target_id == 0U))return ESCX_StateMachineSendEvent(m, e); if (m->io_processor_send != (ESCX_IOProcessorSendFunction)0) {
				status = m->io_processor_send(m, t, e, m->io_processor_context); if (status == (int)ESCX_STATUS_PROCESSED)return ESCX_STATUS_PROCESSED; return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, (ESCX_ProcessStatus)status);
			}return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET);
		}return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET);
	}if (m->io_processor_send == (ESCX_IOProcessorSendFunction)0)return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET); status = m->io_processor_send(m, t, e, m->io_processor_context); if (status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, (ESCX_ProcessStatus)status); return ESCX_STATUS_PROCESSED;
}
ESCX_ProcessStatus ESCX_StateMachineConfigureParent(ESCX_StateMachine* m, ESCX_StateMachine* parent, ESCX_BYTE_T parent_invoke_id)
{
	if (m == (ESCX_StateMachine*)0)return ESCX_STATUS_INVALID_ARGUMENT;
	if ((parent == (ESCX_StateMachine*)0) && (parent_invoke_id != ESCX_NO_INVOKE))return ESCX_STATUS_INVALID_ARGUMENT;
	m->parent_machine = parent; m->parent_invoke_id = parent_invoke_id; return ESCX_STATUS_PROCESSED;
}

ESCX_ProcessStatus ESCX_StateMachineSendTargetedRequest(ESCX_StateMachine* m, const ESCX_TargetedSendRequest* request)
{
#if ESCX_ENABLE_DELAYED_SENDS
	ESCX_BYTE_T i;
#endif
	if ((m == (ESCX_StateMachine*)0) || (request == (const ESCX_TargetedSendRequest*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	if (m->running == ESCX_FALSE)return ESCX_STATUS_NOT_RUNNING;
	if (request->event.name_id >= m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;
	if ((request->delay_ticks != 0UL) && (request->target.processor == ESCX_IO_PROCESSOR_INTERNAL) && (request->target.target_type == ESCX_IO_TARGET_INTERNAL))return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_INVALID_TARGET);
	if (request->delay_ticks == 0UL)return ESCX_StateMachineSendToTarget(m, &request->target, &request->event);
#if ESCX_ENABLE_DELAYED_SENDS
	if (m->start_timer == (ESCX_StartTimerFunction)0)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_TIMER_START_FAILED);
	for (i = 0U; i < ESCX_MAX_DELAYED_SENDS; ++i)if (m->delayed_sends[i].active == ESCX_FALSE) {
		ESCX_TimerId id; m->delayed_sends[i].event_id = request->event.name_id; m->delayed_sends[i].target = 0U; m->delayed_sends[i].target_id = 0U; m->delayed_sends[i].send_id = request->event.send_id; m->delayed_sends[i].event_type = request->event.type; m->delayed_sends[i].origin_type = ESCX_EVENT_ORIGIN_SCXML; m->delayed_sends[i].invoke_id = request->event.invoke_id; m->delayed_sends[i].event_data = request->event.data; m->delayed_sends[i].target_descriptor = request->target; m->delayed_sends[i].uses_target_descriptor = ESCX_TRUE; m->delayed_sends[i].timer_id = ESCX_INVALID_TIMER_ID; id = m->start_timer(m, request->delay_ticks, m->timer_context); if (id == ESCX_INVALID_TIMER_ID) {
			m->delayed_sends[i].uses_target_descriptor = ESCX_FALSE; return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_TIMER_START_FAILED);
		}m->delayed_sends[i].timer_id = id; m->delayed_sends[i].active = ESCX_TRUE; return ESCX_STATUS_PROCESSED;
	}
	return ESCX_STATUS_QUEUE_FULL;
#else
	return ESCX_STATUS_INVALID_TARGET;
#endif
}
static ESCX_ProcessStatus DispatchSend(ESCX_StateMachine* m, ESCX_BYTE_T event_id, void* data, ESCX_BYTE_T target, ESCX_BYTE_T target_id)
{
	ESCX_InvokeDescriptor d; int status;
	if ((m == (ESCX_StateMachine*)0) || (event_id >= m->definition->event_count))return ESCX_STATUS_INVALID_EVENT;
	if (target == ESCX_SEND_TARGET_INTERNAL)return ESCX_StateMachineRaiseInternal(m, event_id, data);
	if (target == ESCX_SEND_TARGET_SELF)return ESCX_StateMachineSend(m, event_id, data);
	if (target == ESCX_SEND_TARGET_PARENT) {
		if ((m->parent_machine == (ESCX_StateMachine*)0) || (m->parent_invoke_id == ESCX_NO_INVOKE))return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_NO_PARENT);
		return ESCX_StateMachineReceiveInvokeEvent(m->parent_machine, m->parent_invoke_id, event_id, data);
	}
	if (target == ESCX_SEND_TARGET_INVOKE) {
		if ((target_id >= m->invoke_count) || (m->invoke_instances == (ESCX_InvokeInstance*)0) || (m->invoke_instances[target_id].active == ESCX_FALSE))return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET);
		ReadInvokeDescriptor(m, target_id, &d); if (d.forward == (ESCX_InvokeForwardFunction)0)return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, ESCX_STATUS_INVALID_TARGET);
		status = d.forward(m, d.child_storage, event_id, data); if (status != (int)ESCX_STATUS_PROCESSED)return ReportErrorInternal(m, ESCX_ERROR_COMMUNICATION, (ESCX_ProcessStatus)status); return ESCX_STATUS_PROCESSED;
	}
	return ESCX_STATUS_INVALID_TARGET;
}

ESCX_ProcessStatus ESCX_StateMachineSendRequest(ESCX_StateMachine* m, const ESCX_SendRequest* request)
{
#if ESCX_ENABLE_DELAYED_SENDS
	ESCX_BYTE_T i;
#endif
	if ((m == (ESCX_StateMachine*)0) || (request == (const ESCX_SendRequest*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	if (m->running == ESCX_FALSE)return ESCX_STATUS_NOT_RUNNING;
	if (request->event_id >= m->definition->event_count)return ESCX_STATUS_INVALID_EVENT;
	if ((request->delay_ticks != 0UL) && (request->target == ESCX_SEND_TARGET_INTERNAL))return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_INVALID_TARGET);
	if (request->delay_ticks == 0UL)return DispatchSend(m, request->event_id, request->event_data, request->target, request->target_id);
#if ESCX_ENABLE_DELAYED_SENDS
	if (m->start_timer == (ESCX_StartTimerFunction)0)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_TIMER_START_FAILED);
	for (i = 0U; i < ESCX_MAX_DELAYED_SENDS; ++i)if (m->delayed_sends[i].active == ESCX_FALSE) {
		ESCX_TimerId id; m->delayed_sends[i].event_id = request->event_id; m->delayed_sends[i].target = request->target; m->delayed_sends[i].target_id = request->target_id; m->delayed_sends[i].send_id = request->send_id; m->delayed_sends[i].event_type = ESCX_EVENT_TYPE_EXTERNAL; m->delayed_sends[i].origin_type = ESCX_EVENT_ORIGIN_SCXML; m->delayed_sends[i].invoke_id = ESCX_NO_INVOKE; m->delayed_sends[i].event_data = request->event_data; m->delayed_sends[i].uses_target_descriptor = ESCX_FALSE; m->delayed_sends[i].timer_id = ESCX_INVALID_TIMER_ID;
		id = m->start_timer(m, request->delay_ticks, m->timer_context); if (id == ESCX_INVALID_TIMER_ID)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_TIMER_START_FAILED); m->delayed_sends[i].timer_id = id; m->delayed_sends[i].active = ESCX_TRUE; return ESCX_STATUS_PROCESSED;
	}
	return ESCX_STATUS_QUEUE_FULL;
#else
	return ESCX_STATUS_INVALID_TARGET;
#endif
}

ESCX_ProcessStatus ESCX_StateMachineCancelSend(ESCX_StateMachine* m, ESCX_UINT16_T send_id)
{
#if ESCX_ENABLE_DELAYED_SENDS
	ESCX_BYTE_T i; if ((m == (ESCX_StateMachine*)0) || (send_id == ESCX_NO_SEND_ID))return ESCX_STATUS_INVALID_ARGUMENT;
	for (i = 0U; i < ESCX_MAX_DELAYED_SENDS; ++i)if ((m->delayed_sends[i].active != ESCX_FALSE) && (m->delayed_sends[i].send_id == send_id)) {
		ESCX_TimerId id = m->delayed_sends[i].timer_id; ESCX_BOOL_T ok = ESCX_TRUE; m->delayed_sends[i].active = ESCX_FALSE; m->delayed_sends[i].timer_id = ESCX_INVALID_TIMER_ID; if (m->cancel_timer != (ESCX_CancelTimerFunction)0)ok = m->cancel_timer(m, id, m->timer_context); return(ok != ESCX_FALSE) ? ESCX_STATUS_PROCESSED : ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_TIMER_CANCEL_FAILED);
	}
	return ESCX_STATUS_SEND_NOT_FOUND;
#else
	(void)m; (void)send_id; return ESCX_STATUS_SEND_NOT_FOUND;
#endif
}

ESCX_ProcessStatus ESCX_StateMachineExecuteContent(ESCX_StateMachine* m, const ESCX_ExecutableBlock* block)
{
	ESCX_UINT16_T i; ESCX_ExecutableStep step; ESCX_ProcessStatus result; int callback_status;
	if ((m == (ESCX_StateMachine*)0) || (block == (const ESCX_ExecutableBlock*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	if ((block->step_count != 0U) && (block->steps == (const ESCX_ExecutableStep*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	for (i = 0U; i < block->step_count; ++i) {
		ReadExecutableStep(&block->steps[i], &step); result = ESCX_STATUS_PROCESSED;
		if (step.kind == ESCX_EXECUTABLE_GENERATED) {
			if (step.function == (ESCX_ExecutableFunction)0)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_INVALID_ARGUMENT); callback_status = step.function(m, step.context); if (callback_status != (int)ESCX_STATUS_PROCESSED)result = ReportErrorInternal(m, ESCX_ERROR_EXECUTION, (ESCX_ProcessStatus)callback_status);
		}
		else if (step.kind == ESCX_EXECUTABLE_RAISE) {
			if (step.event == (const ESCX_Event*)0)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_INVALID_ARGUMENT); result = ESCX_StateMachineRaiseEvent(m, step.event); if (result != ESCX_STATUS_PROCESSED)result = ReportErrorInternal(m, ESCX_ERROR_EXECUTION, result);
		}
		else if (step.kind == ESCX_EXECUTABLE_SEND) {
			if (step.send_request == (const ESCX_SendRequest*)0)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_INVALID_ARGUMENT); result = ESCX_StateMachineSendRequest(m, step.send_request);
		}
		else if (step.kind == ESCX_EXECUTABLE_TARGETED_SEND) {
			if (step.targeted_send_request == (const ESCX_TargetedSendRequest*)0)return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_INVALID_ARGUMENT); result = ESCX_StateMachineSendTargetedRequest(m, step.targeted_send_request);
		}
		else if (step.kind == ESCX_EXECUTABLE_CANCEL) {
			result = ESCX_StateMachineCancelSend(m, step.send_id);
			/* W3C <cancel> is idempotent when no pending send has this ID. */
			if (result == ESCX_STATUS_SEND_NOT_FOUND)result = ESCX_STATUS_PROCESSED;
			else if (result != ESCX_STATUS_PROCESSED)result = ReportErrorInternal(m, ESCX_ERROR_EXECUTION, result);
		}
		else return ReportErrorInternal(m, ESCX_ERROR_EXECUTION, ESCX_STATUS_INVALID_ARGUMENT);
		if (result != ESCX_STATUS_PROCESSED)return result;
	}
	return ESCX_STATUS_PROCESSED;
}
static ESCX_ProcessStatus ReportErrorInternal(ESCX_StateMachine* m, ESCX_ErrorCode code, ESCX_ProcessStatus cause)
{
#if ESCX_ENABLE_ERROR_EVENTS
	ESCX_BYTE_T event_id; ESCX_ProcessStatus stabilization_status;
	if (m == (ESCX_StateMachine*)0)return ESCX_STATUS_INVALID_ARGUMENT;
	m->last_error = (ESCX_BYTE_T)code; m->last_error_status = cause;
	m->error_event_data.code = (ESCX_BYTE_T)code; m->error_event_data.cause = cause;
	m->error_event_data.triggering_event = (m->has_triggering_event != ESCX_FALSE) ? m->triggering_event_descriptor : m->current_event_descriptor;
	if (m->reporting_error != ESCX_FALSE)return cause;
	event_id = (code == ESCX_ERROR_COMMUNICATION) ? m->error_communication_event : m->error_execution_event;
	if ((event_id == ESCX_NO_EVENT) || (m->running == ESCX_FALSE))return cause;
	m->pending_error_event_data = m->error_event_data;
	if (ESCX_EventQueuePushEx(&m->internal_queue, event_id, ESCX_EVENT_TYPE_PLATFORM, ESCX_EVENT_ORIGIN_PLATFORM, ESCX_NO_INVOKE, ESCX_NO_SEND_ID, &m->pending_error_event_data) == ESCX_FALSE)return cause;
	m->reporting_error = ESCX_TRUE;
	if (m->processing == ESCX_FALSE) {
		m->processing = ESCX_TRUE; stabilization_status = Stabilize(m); m->processing = ESCX_FALSE; if ((stabilization_status != ESCX_STATUS_PROCESSED) && (stabilization_status != ESCX_STATUS_IGNORED))return stabilization_status;
	}
	return cause;
#else
	if (m != (ESCX_StateMachine*)0) {
		m->last_error = (ESCX_BYTE_T)code; m->last_error_status = cause;
	}
	return cause;
#endif
}
ESCX_ProcessStatus ESCX_StateMachineReportError(ESCX_StateMachine* m, ESCX_ErrorCode code, ESCX_ProcessStatus cause)
{
	if ((code == ESCX_ERROR_NONE) || (code > ESCX_ERROR_PLATFORM))return ESCX_STATUS_INVALID_ARGUMENT;
	return ReportErrorInternal(m, code, cause);
}
ESCX_ProcessStatus ESCX_StateMachineConfigureErrorEvents(ESCX_StateMachine* m, ESCX_BYTE_T execution_event, ESCX_BYTE_T communication_event)
{
	if ((m == (ESCX_StateMachine*)0) || (m->definition == (const ESCX_StateMachineDefinition*)0))return ESCX_STATUS_INVALID_ARGUMENT;
	if ((execution_event != ESCX_NO_EVENT) && (execution_event >= m->definition->event_count))return ESCX_STATUS_INVALID_EVENT;
	if ((communication_event != ESCX_NO_EVENT) && (communication_event >= m->definition->event_count))return ESCX_STATUS_INVALID_EVENT;
	m->error_execution_event = execution_event; m->error_communication_event = communication_event; return ESCX_STATUS_PROCESSED;
}

ESCX_BYTE_T ESCX_StateMachineGetLastError(const ESCX_StateMachine* m)
{
	return (m == (const ESCX_StateMachine*)0) ? (ESCX_BYTE_T)ESCX_ERROR_PLATFORM : m->last_error;
}

ESCX_ProcessStatus ESCX_StateMachineGetLastErrorStatus(const ESCX_StateMachine* m)
{
	return (m == (const ESCX_StateMachine*)0) ? ESCX_STATUS_INVALID_ARGUMENT : m->last_error_status;
}
const ESCX_ErrorEventData* ESCX_StateMachineGetLastErrorData(const ESCX_StateMachine* m)
{
	return (m == (const ESCX_StateMachine*)0) ? (const ESCX_ErrorEventData*)0 : &m->error_event_data;
}

void ESCX_StateMachineClearError(ESCX_StateMachine* m)
{
	if (m != (ESCX_StateMachine*)0) {
		m->last_error = ESCX_ERROR_NONE; m->last_error_status = ESCX_STATUS_PROCESSED; m->error_event_data.code = ESCX_ERROR_NONE; m->error_event_data.cause = ESCX_STATUS_PROCESSED; m->error_event_data.triggering_event.name_id = ESCX_NO_EVENT; m->error_event_data.triggering_event.type = ESCX_EVENT_TYPE_INTERNAL; m->error_event_data.triggering_event.origin_type = ESCX_EVENT_ORIGIN_NONE; m->error_event_data.triggering_event.invoke_id = ESCX_NO_INVOKE; m->error_event_data.triggering_event.send_id = ESCX_NO_SEND_ID; m->error_event_data.triggering_event.data = (void*)0; m->pending_error_event_data = m->error_event_data; m->reporting_error = ESCX_FALSE;
	}
}
