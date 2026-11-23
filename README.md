# ESCXML ANSI C state-machine runtime

This is the first simple-state-machine stage of the embedded SCXML runtime. It is generated-code friendly and intentionally keeps expression/ECMAScript translation outside the engine.

## Portability target

- ANSI C90 source (also acceptable to C89-oriented compilers)
- Green Hills strict C mode
- Microsoft Visual C compiler
- GCC and Clang on Linux
- No C99 declarations, `//` comments, variadic macros, designated initializers, VLAs, `stdint.h`, or compiler extensions
- No heap allocation in the runtime

## Main corrections from ESCXML.c

- Guards and actions are attached to transitions, not destination states.
- Ordered transition descriptors replace the event/state target matrix, allowing multiple guarded transitions for the same event.
- Caller-owned machine storage replaces `malloc`.
- The generated state and transition tables are constant data.
- The queue stores both event ID and payload pointer.
- Queue processing is iterative rather than recursive.
- Event IDs use unsigned `ESCX_BYTE_T`, avoiding signed-char truncation.
- Assertions can be redirected to a platform callback and do not require `stdio`.
- State entry, exit, transition action, and state callback order is explicit.

## Build

Linux:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ctest --test-dir build --output-on-failure

Visual Studio:

    cmake -S . -B build-vs -G "Visual Studio 18 2026" -A x64
    cmake --build build-vs --config Release
    ctest --test-dir build-vs -C Release --output-on-failure

If the installed Visual Studio generator name differs, run `cmake --help` and select the listed generator.

Strict GCC check used for this package:

    cc -std=c89 -pedantic-errors -Wall -Wextra -Werror -Iinclude -Iexample src/*.c example/SimpleMachine.c tests/test_state_machine.c -o scxml_test

## Generator contract for the first stage

The generator emits:

1. State and event enums.
2. State callback functions.
3. Transition guard and action functions.
4. `ESCX_STATES_BEGIN` / `ESCX_STATE` table.
5. Ordered `ESCX_TRANSITIONS_BEGIN` / `ESCX_TRANSITION` table.
6. `ESCX_DEFINE_MACHINE` and `ESCX_IMPLEMENT_MACHINE`.

The engine does not parse SCXML and does not translate ECMAScript. Those belong to the generator.

## Current deliberate scope

This stage supports flat, single-active-state machines with exact numeric events, ordered guarded transitions, entry/exit handlers, transition actions, timers, and queued internal/reentrant events.

Later runtime stages can add compound states, eventless stabilization, parallel configurations, history, final states, internal transition domains, and SCXML conflict resolution without changing the expression generator boundary.

## Payload lifetime

The queue stores `void *` event payloads. The caller must keep queued payload storage alive until processing completes. A later target profile can generate copied tagged-union payloads when pointer lifetime is not acceptable.

## Compile-time memory profile

Configuration is in `include/ESCXConfig.h` and can be overridden with compiler definitions:

    -DESCX_MAX_STATES=16 -DESCX_MAX_EVENTS=16 -DESCX_MAX_TRANSITIONS=24
    -DESCX_QUEUE_SIZE=2 -DESCX_ENABLE_EVENT_DATA=0

`ESCX_MAX_STATES`, `ESCX_MAX_EVENTS`, and `ESCX_MAX_TRANSITIONS` are generator/table limits checked at compile time. They do not allocate arrays in the runtime. `ESCX_QUEUE_SIZE` directly controls RAM. Setting `ESCX_ENABLE_EVENT_DATA=0` removes the payload pointer from every queue entry.

Suggested profiles:

* AVR/Arduino Uno: 16 states, 16 events, 24 transitions, queue 2, event data disabled if possible.
* Cortex-M/Renesas/ESP32: 32 states, 32 events, 64 transitions, queue 4 or 8.
* Linux/Windows: project-specific values; queue 10 or larger if needed.

## Arduino

The runtime can run on Arduino because it uses fixed storage, no heap, no exceptions, and C linkage. Arduino builds sketches as C++, but `.c` library sources remain C and the headers provide `extern "C"` guards. Copy this directory into the Arduino libraries directory or install the ZIP. The `library.properties` file declares cross-architecture support.

For small AVR boards, keep generated state/transition tables `static const`, use a shallow queue, disable payload pointers when unused, and avoid the default `assert()` failure path by installing `ESCX_SetAssertCallback()` or compiling with `ESCX_ENABLE_ASSERTS=0` after testing.
