# Review: PR #29 — Hermetic Builds + CodeRabbit Re-review

## Review Metadata
**Reviewer:** Hermes / desktop
**Review Date:** 2026-09-07
**Review Type:** Code
**Scope:** `packages/include/` (new), `packages/toi/src/*.cpp` (4 files modified), `packages/otoi/include/otoi/OTOIManager.h` (modified), `packages/otoi/src/OTOIManager.cpp` (modified), `packages/otoi/CMakeLists.txt` (modified), `packages/sleepwalker/src/*.cpp` (4 files modified), `packages/toi/tests/standalone_test.cpp` (new), `packages/otoi/tests/standalone_test.cpp` (new), `packages/sleepwalker/tests/standalone_test.cpp` (modified), `docs/agent-log/handoffs/`, `docs/agent-log/registrations/`, `docs/active-threads.md`
**OTOI Version:** ORG-DEV-OTOI-1.0.3
**Phases:** Phases 2-3, 5 (TOI, OTOI, Sleepwalker) + hermetic build infrastructure

## Pre-Review Checklist
- [x] Read NLT-DEV-OTOI.md and understood governance requirements
- [x] Identified the review scope and what is being evaluated
- [x] Checked active threads in `docs/active-threads.md` (thread `hermetic-build-fixes` resolved)
- [x] Verified the review format matches REVIEW.md template

## Review Findings

### Summary
This PR (merged) was a follow-up to PR #15 (merged), addressing four CodeRabbit findings plus earlier PR #15 issues. It fixes package include paths, OTOI C++23 issues (missing `<expected>`, CMakeLists C++23, missing `<set>`, typed-to-JSON validation bridge, PolicyConflict tier mapping, public resolveEnforcement), adds vendored hermetic headers to `packages/include/`, and adds dependency-free standalone test runners for all three pillar packages. The PR scope touches Phases 2 (TOI), 3 (OTOI), and 5 (Sleepwalker), plus the `packages/include/` hermetic build layer used by Phase 6.

### Strengths
- `packages/include/` vendored headers: nlohmann/json.hpp (3.11.3, MIT), tl/expected.hpp (1.3.1, CC0), spdlog compatibility shim — all with provenance documentation in `packages/include/README.md`.
- OTOI fixes address real latent bugs: missing `#include <expected>` in header, CMake C++20→23 mismatch (std::expected is C++23), OTOIValidator::validate passing typed members to JSON-shaped validators, PolicyConflict tier string→enum mapping, resolveEnforcement made public.
- All package `.cpp` files now use namespaced includes (`<sleepwalker/X.h>`, `<toi/X.h>`, `<otoi/X.h>`) — consistent consumer-facing convention.
- Standalone test runners written against actual implemented APIs, exit non-zero on failure (CI-able smoke tests).
- Comprehensive CodeRabbit-generated summary included in PR description.

### Issues
#### Critical
No critical issues on the original PR. However, **two CodeRabbit comments were flagged as "Major" and initially unaddressed**:

1. **Unknown-tier consistency (OTOIManager.cpp line 303):** `detectConflicts` skips unknown `$tier` values via `tier_from_string` returning `std::nullopt`, but `resolveDocuments` still processes those documents with priority 0 — silently resolving conflicts under `ConflictStrategy::Reject` without surfacing a `PolicyConflict`.

2. **Non-POSIX `localProcessId()` (standalone_test.cpp line 27):** `std::rand()` fallback can produce identical values in separate processes, causing temp-directory collisions and potential `remove_all` race conditions.

**Resolution:** Both issues were subsequently addressed in commits `fba306d` and `7a49530` (merged into main after this PR). On the current main state:
- `detectConflicts` now surfaces unknown-tier documents as `PolicyConflict` (path `"$tier"`)
- `resolveDocuments` drops documents with unresolvable `$tier` (priority -1, filtered)
- `localProcessId()` uses `_getpid()` on Windows, `::getpid()` on POSIX, `std::random_device`-seeded atomic counter on other non-POSIX platforms

#### High Priority
No high-priority issues remaining — both CodeRabbit Major findings resolved.

#### Low Priority / Observations
- The spdlog shim in `packages/include/` is labeled as a compatibility shim for replacement under a full toolchain — documented appropriately.
- Standalone runners could be wired into CI as required smoke checks (noted as a decision pending in the handoff record).
- DECISIONS.md §1 specifies C++20 as the standard, but OTOI uses C++23 for `std::expected`. This is a documented intentional deviation (OTOI legitimately requires C++23 per dual error-handling pattern). The root CMakeLists.txt sets C++23 to accommodate this.

### Cross-Pillar Integration Check
- Follows DECISIONS.md: C++20 for TOI/Sleepwalker/RRT, C++23 for OTOI (std::expected), nlohmann::json, Catch2, vcpkg
- API surface compatible: OTOI's `resolveEnforcement` made public for `safeHonor` reuse; dual API pattern (exceptions + std::expected) preserved
- No duplicate logic in Phase 6 umbrella — `packages/include/` is designed as a fallback for Phase 6
- Error handling follows dual API pattern (throwing + std::expected)

### Factual Accuracy
- All external claims verified against authoritative sources
- No hallucinated organizations, numbers, or URLs
- Sources cited: nlohmann/json 3.11.3 official single-header (MIT), tl/expected 1.3.1 (CC0), g++ 15.2.0

### Governance Compliance
- No credentials or secrets exposed
- No external integrations without approval
- No architecture decisions without approval
- Commit format follows `[AGENT_NAME] type(scope): description`
- docs/active-threads.md updated
- Handoff records written to docs/agent-log/handoffs/

## Verification (on merged main state)
```
=== TOI Results: 25 passed, 0 failed ===          (g++ 15.2.0, C++20)
=== OTOI Results: 8 passed, 0 failed ===          (g++ 15.2.0, C++23)
=== Sleepwalker Results: 84 passed, 0 failed ===  (g++ 15.2.0, C++20)
=== ASFDK Umbrella: 37 passed, 0 failed ===       (g++ 15.2.0, C++23)
Governance validation PASSED — all 22 checks OK
```

## Verdict
**Status:** APPROVED
**Rationale:** PR #29 comprehensively addresses hermetic build issues across Phases 2-3 and 5. The two CodeRabbit Major findings were subsequently resolved in commits fba306d and 7a49530 (merged into main). On the current main state, all unknown-tier handling is consistent between detectConflicts and resolveDocuments, and localProcessId() is cross-platform safe. All 154+ tests pass across all pillar packages and the umbrella. Governance 22/22.

## Handoff Notes
- Phases 2 (TOI), 3 (OTOI), 5 (Sleepwalker) now compile hermetically and pass standalone suites
- packages/include/ is available as fallback for Phase 6 ASFDK umbrella integration
- RRT Advocate (Phase 4) already compiles on main (its standalone runner landed with PR #14)
- Consider wiring standalone runners into CI as required smoke checks (decision pending per handoff record)

*This review conducted following REVIEW.md under ORG-DEV-OTOI-1.0.3.*
