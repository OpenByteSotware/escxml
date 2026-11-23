# ESCXML embedded runtime 0.3.0

ANSI C90 embedded state-machine runtime with generated O(1) state/event dispatch.

## Runtime guarantee

The generator emits a compressed row-displacement index. Lookup performs a fixed number of ROM reads, one addition, one bounds check and one owner check. It never scans states, events, or the complete transition table. For one candidate per state/event pair, transition selection is worst-case O(1). If SCXML defines multiple ordered guards for the same pair, locating their range is O(1), while guard evaluation is O(k) for only those k candidates.

All generated lookup arrays and descriptors use `static const` storage. No heap or runtime table construction is used.

## Cross-platform ROM access

Normal ARM, x86, Windows, Linux and Green Hills builds use ordinary const reads. Classic AVR builds define:

    ESCX_PLATFORM_AVR=1

This enables `PROGMEM`, `pgm_read_byte`, `pgm_read_word`, and `pgm_read_ptr` through `ESCXRom.h`. Generated source remains the same across targets.

## Strict validation

    gcc -std=c89 -pedantic-errors -Wall -Wextra -Werror \
      -Iinclude -Iexample src/*.c example/SimpleMachine.c \
      tests/test_state_machine.c -o escxml_test

## Generator output contract

The desktop generator emits exact-size arrays using:

* `ESCX_STATES_BEGIN`
* `ESCX_TRANSITIONS_BEGIN`
* `ESCX_RANGES_BEGIN`
* `ESCX_STATE_BASE_BEGIN`
* `ESCX_SLOT_OWNER_BEGIN`
* `ESCX_SLOT_RANGE_BEGIN`
* `ESCX_DEFINE_MACHINE`

The generator may spend additional time optimizing row placement. Runtime memory and execution speed take priority over generation and compile time.
