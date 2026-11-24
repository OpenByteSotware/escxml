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
