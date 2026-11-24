# Stage 20: fast table-driven dispatch and ordered workspace

Fast dispatch uses a generated effective state/event row-displacement table. Each active atomic state performs one O(1) lookup that already includes state ancestry, hierarchical event descriptors, and wildcard precedence. The compact fallback remains available when no fast table is configured.

Microsteps now merge only the selected transitions' generated exit and entry plans into caller-owned fixed workspace. States are deduplicated with bitsets and inserted in normative depth/document order. The running path no longer scans all generated states to construct exit or entry sets. Runtime cost is proportional to selected plan lengths and affected states, not total machine state count.
