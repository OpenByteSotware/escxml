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
