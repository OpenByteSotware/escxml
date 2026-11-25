# ESCXML SCXML 1.0 conformance map

Baseline: ESCXML 0.48.0, Stage 44
Normative reference: W3C SCXML 1.0 Recommendation, 1 September 2015.
Test model reference: W3C SCXML 1.0 Implementation Report Plan (IRP).

## Profile

ESCXML is a generated, deterministic embedded SCXML execution runtime. The generator is responsible for XML parsing, static document validation, expression compilation, event-name internment, transition-domain/entry/exit plan construction, and generation of fast lookup tables. The runtime intentionally has no XML parser, heap allocator, or general scripting engine.

The conformance claim therefore applies to the observable behavior of valid generated SCXML documents within the supported processor profile. XML syntax/schema rejection and expression-language parsing belong to the generator, not the embedded runtime.

## Status vocabulary

- SUPPORTED: runtime behavior is represented directly and has focused regression coverage.
- GENERATED: semantics are supported through generator-produced callbacks or tables; generator and runtime together form the processor feature.
- PROFILE: intentionally constrained by the embedded profile and requires an adapter or generator facility.
- GAP: identified behavior requiring a later conformance stage.

## Requirement map

| SCXML area | Status | ESCXML evidence | Next action |
|---|---|---|---|
| Basic state hierarchy, compound states, initial configuration | SUPPORTED | hierarchical active configuration; initial child and generated initial-entry plans; Stage 22/24 tests | Map applicable IRP cases |
| Parallel states | SUPPORTED | simultaneous atomic configuration and global microsteps; Stage 11/12/23 tests | Map applicable IRP cases |
| Eventless transitions | SUPPORTED | stabilization loop and generated effective eventless selector table | Map applicable IRP cases |
| External/internal/targetless transitions | SUPPORTED | transition type metadata and generated exit/entry plans | Add focused transition-domain IRP mappings |
| Transition selection and conflict resolution | SUPPORTED | descendant preemption, document order, optional conflict bitmap; Stages 12/21 | Expand W3C conflict corpus |
| Entry/exit ordering | SUPPORTED | merged ordered workspace and generated termination exit order; Stages 11/38 | Expand ordering corpus |
| Internal vs external event queues | SUPPORTED | distinct queues; eventless/internal processing precedes next external macrostep | Add queue-order W3C mappings |
| Event descriptor / `_event` scope | SUPPORTED | structured ESCX_Event and neutral eventless scope; Stages 13/23/37 | Map metadata assertions |
| Event descriptor matching and wildcard matching | SUPPORTED | parent-prefix event map plus wildcard; Stage 14/43 | Map descriptor-prefix IRP tests |
| `raise` | SUPPORTED | executable block internal-event step; Stage 28 | Map IRP executable-content tests |
| `send`, delay, cancel | SUPPORTED | send requests, delayed timer integration, send IDs, cancel; Stages 7/31 | Map deterministic virtual-time equivalents |
| SCXML I/O processor targets | SUPPORTED/PROFILE | current session, internal, parent, invoke, session and custom transport descriptors; Stage 30/31 | Verify target URI/name mapping in generator |
| `onentry`, `onexit`, transition executable content | GENERATED | legacy and status callbacks plus ordered execution | W3C content tests should compile to generated callbacks |
| `assign`, script-like content | GENERATED | ESCX_EXECUTABLE_GENERATED callback | Generator/datamodel defines expression semantics |
| Conditional expressions | GENERATED | generated selector callback chooses transition | Datamodel-specific expression conformance remains generator responsibility |
| Datamodel early/late binding | SUPPORTED | ESCX_DATA_BINDING_EARLY/LATE; Stage 17 | Add state-local initialization mappings |
| System variables | SUPPORTED/PROFILE | system context and current event; Stage 29/37 | Generator maps SCXML names to fixed representation |
| `final` and `done.state` | SUPPORTED | completion detection/latch, done payload, interpreter termination; Stages 18/19/25 | Map completion corpus |
| `donedata` | GENERATED | ESCX_DoneDataFunction for final and invoked completion | Add complete payload W3C mappings |
| Shallow/deep history | SUPPORTED | generated history ranges, default content, deterministic restoration and validation; Stages 35/42 | Map history corpus |
| `invoke` lifecycle | SUPPORTED/PROFILE | initialization, deferred start, cancellation, completion, finalize, autoforward, event metadata; Stages 26/27/32/33/36 | External invoke types need adapters |
| `finalize` | SUPPORTED | finalize receives normalized child event before parent processing | Map invoke/finalize corpus |
| `autoforward` | SUPPORTED | external events forwarded to active invokes | Map invoke corpus |
| `error.execution` / `error.communication` | SUPPORTED | configurable platform error events and stable structured payload; Stages 34/40 | Map error corpus |
| Interpreter termination | SUPPORTED | top-level final exits complete configuration, cancels delayed work, stops session; Stage 25 | Map termination cases |
| XML parsing/schema validation | PROFILE | intentionally generator-side | Validate in generator conformance suite |
| Data model language (ECMAScript/XPath/minimal) | PROFILE | callbacks are compiled/generated outside runtime | Declare selected datamodel per generated application |
| Dynamic expression evaluation | PROFILE | no general runtime scripting engine | Generator must precompile supported expressions |
| Arbitrary remote I/O processor protocols | PROFILE | custom transport callback | Protocol adapters are platform integrations |

## W3C IRP mapping policy

The W3C IRP tests are self-checking state machines whose automated verdict is reaching a success or failure state. ESCXML should therefore map each applicable IRP test to a generated C machine plus a GoogleTest harness that starts it, drives required external/timer/invoke stimuli deterministically, and asserts successful final completion.

Do not treat an IRP test as covered merely because a unit test exercises a similar API. Each imported/mapped IRP case should record:

1. W3C test identifier.
2. Applicable processor/datamodel profile.
3. Generator transformation used.
4. Runtime regression test file.
5. PASS / PROFILE / GAP disposition.
6. If PROFILE or GAP, the exact unsupported normative behavior.

Manual IRP tests remain manual unless converted into an equivalent deterministic assertion without changing their observable requirement.

## Stage 44 gap register and priority

Stage 44 is deliberately documentation/test-mapping only. It does not alter macrostep code paths.

Priority for Stage 45+:

1. Build an explicit W3C IRP manifest and begin mapping core state/transition tests to GoogleTest.
2. Use failures from that manifest to drive runtime changes rather than adding speculative features.
3. First target core algorithm semantics: transition domain, configuration entry/exit ordering, event queue ordering, completion, and history.
4. Then cover executable content, errors, send/cancel, and invoke semantics.
5. Keep generator/datamodel-only requirements clearly separated from embedded-runtime gaps.

Performance rule: conformance metadata must remain build/test-time or generated ROM metadata. No Stage 44 facility introduces runtime string searches, XML parsing, heap use, or whole-machine scans into normal event dispatch.
