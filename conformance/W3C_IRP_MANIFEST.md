# W3C SCXML IRP manifest

This CSV is the machine-readable source of truth for ESCXML conformance work.

Disposition meanings:

- `MAPPED`: an ESCXML regression test covers the same semantic area, but the official W3C test has not yet been imported and executed end to end.
- `PASS`: the named official W3C test has been transformed for the embedded profile and passes the deterministic ESCXML harness.
- `GAP`: a known normative behavior is not yet implemented or cannot yet be proven.
- `PROFILE`: outside the declared generated embedded processor profile; the exact processor/generator/platform responsibility must be recorded.
- `MANUAL`: the official test requires human observation or an approved equivalent deterministic assertion.

A row must never move from `MAPPED` to `PASS` merely because a similar local test passes. `PASS` requires an imported W3C assertion test with a success/failure verdict. Known gaps are exact entries, not a lower bound: an unexpected pass must force the row to be updated.
