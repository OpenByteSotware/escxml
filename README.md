# ESCXML embedded runtime 0.4.0

ANSI C90 embedded state-machine runtime with generated row-displacement state/event lookup and generated selector functions.

## Dispatch model

1. The row-displacement arrays locate one selector ID using a fixed number of indexed ROM reads. This portion is O(1) and never scans states, events, or transitions.
2. The generated selector uses a C `switch` or a fixed sequence of conditions to return one transition ID.
3. The runtime executes that transition directly. It does not loop over guard candidates.

A C compiler is free to lower `switch` into a jump table, comparison tree, or comparison sequence. Selector work is bounded by generated code and does not grow with the total state-machine size, but it is not claimed to be strict O(1) for arbitrary guards.

## Memory behavior

* No guard-key tables.
* No cached guard state.
* No dynamic allocation.
* No runtime table construction.
* No guard function pointer in each transition descriptor.
* State, transition, selector and row-displacement arrays are `static const` and use `PROGMEM` on classic AVR.

## Generated selector example

    ESCX_SELECTOR_BEGIN(Machine_SelectMode)
        MachineData *data;
        (void)event_data;
        data = (MachineData *)machine->instance;
        ESCX_SELECTOR_SWITCH(data->mode)
            ESCX_SELECTOR_CASE(0, MACHINE_TRANSITION_MODE0)
            ESCX_SELECTOR_CASE(1, MACHINE_TRANSITION_MODE1)
            ESCX_SELECTOR_DEFAULT(ESCX_NO_TRANSITION)
        ESCX_SELECTOR_SWITCH_END()
    ESCX_SELECTOR_END()

Unconditional transitions use a selector that immediately returns one transition ID. This costs one selector call but keeps one compact and portable runtime representation.

## Platform support

Normal ARM, x86, Windows, Linux and Green Hills builds use ordinary const reads. Define `ESCX_PLATFORM_AVR=1` for classic AVR targets to enable `PROGMEM` and typed `memcpy_P` reads for function pointers.

## Strict validation

    gcc -std=c89 -pedantic-errors -Wall -Wextra -Werror \
      -Iinclude -Iexample src/*.c example/SimpleMachine.c \
      tests/test_state_machine.c -o escxml_test

The package tests switch-selected transitions, an unconditional selector, and the no-match path.


## builds
cmake -S . -B build-vs -G "Visual Studio 18 2026"

cmake -S . -B build-vs -G "Visual Studio 18 2026" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DESCX_BUILD_TESTS=ON

### Stage 35: generated ordered history maps

Optional `ESCX_HistoryMap` tables let generated code enumerate only the states relevant to each history slot. Capture and restore avoid whole-machine state scans, and deep restoration follows generator-supplied entry order. The legacy scan path remains available when no map is configured.


#### Stage 36: generated invoke ownership and deferred startup

Optional `ESCX_InvokeMap` tables provide per-state contiguous invoke ranges. Entry schedules invokes, eventless stabilization completes, and only invokes whose owners remain active are initialized and started. Exit clears pending starts and cancels active invokes before exit content.


##### Stage 37: current-event scoping

Eventless selection and executable content now observe a neutral `_event`. The event that drives a macrostep is retained separately as structured triggering-event context, allowing platform errors raised during eventless stabilization to identify the correct external or internal event without leaking that event through `_event`.


####### Stage 38: generated complete-configuration exit order

An optional `ESCX_ExitOrder` gives termination and explicit stop a generated deepest-first, reverse-document-order state sequence. The stop path performs one bounded pass over this ROM table, exits only active states, and avoids repeated whole-definition scans. The legacy scan remains available when no exit order is configured.


### Stage 39: event acceptance semantics

A valid event whose macrostep completes returns `ESCX_STATUS_PROCESSED`, whether or not a transition was enabled. `ESCX_StateMachineGetLastMacrostepResult()` separately reports event acceptance, whether any transition was taken, and the number of transition microsteps executed.


### Stage 40: error-event hardening

A queued platform error owns a stable structured payload and keeps the recursion latch set until that exact event has been consumed. Additional failures while an error is pending update diagnostics without overwriting or recursively enqueueing the pending platform event. Queue-full delivery leaves the latch clear so a later report can retry.


### Stage 41: startup rollback and failure cleanup

Startup is now transactional from the runtime's perspective. Failures during early data initialization, initial entry, eventless stabilization, queued internal-event processing, or deferred invoke startup exit every state that was successfully entered, cancel active invokes and delayed sends, clear queues and pending invoke work, and return the machine to a stopped, restartable state. Parallel child-entry failures preserve their original status instead of being collapsed to `ESCX_STATUS_INVALID_ARGUMENT`.


### Stage 42: generated history validation and deep-history conformance

Generated history maps are now validated against their actual history pseudo-state and owner. Every slot must have exactly one history state, mapped states must be unique descendants of that owner, shallow history may contain only direct children, and deep-history entries must be in ancestor-before-descendant and document order. History pseudo-states cannot be recorded as configuration states. These guarantees make generated deep-history restoration deterministic for nested compound and parallel configurations.


#### Stage 43: generated event-hierarchy validation

Event-prefix parent tables are now validated as an acyclic forest before they are installed. Configuration rejects self-cycles, multi-event cycles, out-of-range parents, and attempts to replace matching tables while the interpreter is running. This keeps hierarchical event matching deterministic and prevents malformed generated metadata from being silently truncated by the runtime probe bound.


#### Stage 44: SCXML 1.0 conformance mapping

The runtime now ships `conformance/SCXML_1_0_CONFORMANCE.md`, which defines the embedded processor profile, maps major SCXML 1.0 semantic areas to existing ESCXML runtime facilities and regression stages, separates generator/datamodel responsibilities from runtime behavior, and defines the policy for mapping the W3C Implementation Report Plan state machines into deterministic GoogleTest coverage. Stage 44 intentionally changes no macrostep execution path; future stages should be driven by explicit W3C test dispositions and observed gaps.


##### Stage 45: complete generated effective-dispatch validation

Fast dispatch configuration now proves, before the interpreter starts, that every generated atomic/final-state and event lookup exactly matches the hierarchical SCXML selection represented by the baseline state tables, including event-prefix fallback, wildcard matching, and ancestor-state precedence. Generated eventless selectors are likewise checked against effective ancestor lookup. Validation can be O(states * events * hierarchy) at configuration time; normal running dispatch remains one row-displacement lookup per active atomic state with no added scans or heap use.


##### Stage 46: generated transition-domain validation

`ESCX_StateMachineValidateTransitionDomains()` validates generated transition metadata before startup. Targetless transitions must have no exit/entry plans; non-targetless transitions must identify a valid source with the correct generated depth and valid non-empty plans. Generated exit plans must be unique and deepest-first/reverse-document-order, entry plans unique and ancestor-first/document-order. External transitions must exit the source or one of its ancestors. Internal transitions are restricted to compound sources, must not exit that source, and their generated entry plan must remain within the source subtree. The validator is explicitly configuration/test-time so normal microstep execution continues to consume the generated plans without additional transition-domain searches.


####### Stage 47: generated conflict-map semantic validation

`ESCX_StateMachineConfigureConflictMap()` now validates every generated transition-pair bit against the conflict relation implied by the validated generated exit plans. The map must be symmetric, its diagonal must mark each transition as conflicting with itself, targetless transitions must not conflict with distinct transitions, and two targeted transitions conflict exactly when their exit-state sets intersect. Configuration rejects any missing or spurious bit before installing the map. Normal transition selection still uses one ROM bit lookup, preserving the O(1) pair-conflict hot path.

This implements the generated-metadata boundary for the W3C SCXML transition-selection algorithm: after enabled transitions are identified, conflicting transitions are those whose exit sets intersect. Descendant-source transitions preempt conflicting ancestor-source transitions; otherwise document order resolves conflicts. ESCXML already performs preemption in `AddEnabledTransition`; Stage 47 proves that its optional generated bitmap encodes the same exit-set conflict predicate used by the normative algorithm.


######## Stage 48: normative transition preemption validation

Transition conflict resolution now uses the W3C SCXML source-ancestry rule rather than treating greater source depth as proof of descent. A candidate preempts a conflicting existing transition only when its source is a proper descendant of the existing source; when the sources are equal or unrelated, document order decides. Optional generated `ESCX_PreemptionMap` ROM metadata provides an O(1) runtime relation. `ESCX_StateMachineConfigurePreemptionMap()` validates every generated bit against actual state ancestry and document order before startup. Without a map, the corrected bounded ancestry fallback is used.


######### Stage 49: exact fast-dispatch validation

`ESCX_StateMachineConfigureFastDispatch()` now requires exact equality between every authoritative generated fast event lookup and the effective hierarchical selector obtained from the baseline state/event tables. A generated entry is rejected both when a required selector is missing and when a spurious selector is present where hierarchical SCXML event matching finds no transition selector. Event-prefix, wildcard and ancestor-state flattening remain covered by the same comparison; eventless mappings were already exact and remain unchanged. Configuration-time validation may scan states, events and bounded ancestry, while normal event dispatch remains one row-displacement ROM lookup per active atomic state with no fallback scan or heap use.


########## Stage 50: W3C IRP manifest and exact disposition gate

The repository now includes `conformance/W3C_IRP_MANIFEST.csv` as a machine-readable mapping from official W3C SCXML assertion/test IDs to ESCXML responsibility, disposition, evidence and next action. `MAPPED` is intentionally distinct from `PASS`: a similar local regression test is not an official W3C pass until the W3C machine is imported and reaches its success verdict. A GoogleTest gate validates schema, unique assertion IDs, allowed dispositions and explicit follow-up for every gap. The initial core manifest identifies exact transition-domain gaps for assertions 504, 506 and 533, making future runtime work test-driven instead of speculative. No event-processing path changes in this stage.


########### Stage 51: exact generated transition-domain metadata

Optional `ESCX_TransitionDomainMap` metadata records each transition's generated target range, effective transition type and computed transition domain. Configuration validates target IDs, legal internal-transition eligibility, external normalization required by W3C assertions 506/533, and the least common compound ancestor required by assertion 504. Generated exit plans must contain only proper descendants of the validated domain. The runtime loop still executes precomputed exit and entry plans directly; target and LCCA work occurs only during generation and optional startup validation.


############ Stage 52: W3C cancel idempotence

Generated `<cancel>` executable content is now idempotent when its send ID has no pending delayed send. `ESCX_StateMachineCancelSend()` retains `ESCX_STATUS_SEND_NOT_FOUND` as a useful host API diagnostic, while `ESCX_StateMachineExecuteContent()` treats that result as successful completion of the SCXML `<cancel>` instruction and does not enqueue `error.execution`. Actual timer-cancellation failures still report `error.execution` and stop the executable block. The operation remains bounded by `ESCX_MAX_DELAYED_SENDS`, a compile-time fixed capacity independent of total state-machine size.


############# Stage 53: internal-target send constraints

SCXML internal-queue sends are now accepted only with zero delay. Both generated targeted sends using the internal I/O processor and the legacy internal send request reject a positive delay as `ESCX_STATUS_INVALID_TARGET`, record `ESCX_ERROR_EXECUTION`, and avoid allocating a delayed-send slot or starting a platform timer. Zero-delay internal sends retain FIFO internal-queue behavior. The new validation is a fixed set of field comparisons, so it adds O(1) work independent of state, transition, event, or hierarchy size.


############## Stage 54: done.state payload scope

A final state's generated `donedata` payload is now attached only to the `done.state.*` event for that final state's immediate compound parent. If the completion also causes an enclosing parallel state to complete, the parallel state's separate `done.state.*` event carries no payload instead of incorrectly reusing the payload from whichever region completed last. Completion latches and generated done-event IDs remain unchanged. The runtime adds only one parent comparison per emitted completion event and does not scan additional states.
