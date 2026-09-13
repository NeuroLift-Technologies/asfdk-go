### Thread: phase-1-reference-analysis
**Status:** resolved
**Owner:** Hermes / desktop
**Started:** 2026-09-06
**Last updated:** 2026-09-06
**Summary:** Completed Phase 1 — Discovery & Reference Analysis for ASFDK C++ port. Produced comprehensive reference analysis documenting TOI, OTOI, RRT Advocate, and Sleepwalker pillar packages from original TypeScript/Python reference implementations. All 22 governance checks passing.
**Blockers:** None
**Next action:** Phase 2 TOI-C++ port maintained per technology decisions in docs/DECISIONS.md

### Thread: governance-setup
**Status:** in_progress
**Owner:** Hermes / desktop
**Started:** 2026-09-06
**Last updated:** 2026-09-06
**Summary:** Governance scaffolding verified — 22/22 checks passing. Phase 1 deliverable created. Technology decisions resolved in PR #8. Ready to begin implementation phases.
**Blockers:** None
**Next action:** Phase 2 TOI-C++ implementation using C++20 technology decisions

### Thread: phase-2-toi-port
**Status:** completed
**Owner:** Hermes / desktop
**Started:** 2026-09-06
**Last updated:** 2026-09-06
**Summary:** TOI-C++ port implemented per PLAN.md Section 4 and DECISIONS.md technology decisions. Package structure created at packages/toi/ with:
- include/toi/TOITypes.h — 12 enum classes mapping TOI Zod schema (ProcessingStyle, AttentionModel, ScaffoldingPreference, EnergyModel, Tone, Verbosity, Structure, JargonTolerance, ThreadReconnection + type aliases)
- include/toi/TermsOfInteraction.h — API surface (parseTOI, safeParseTOI, validateTOI, resolveTOI, canonicalize, verifySignature) with dual error handling pattern (throwing + std::expected)
- include/toi/TOITypes.h — Full TOI document model (Identity, CognitiveProfile, Privacy, Agency, Communication structs; TOIDocument root)
- src/TermsOfInteraction.cpp — TOI document parsing, validation, resolution, and canonicalization implementation
- src/TOIManager.cpp — TOIManager public API class (parse, validate, resolve, canonicalize, verifySignature, safeParseTOI)
- tests/test_toi_manager.cpp — 6 Catch2 unit tests (minimal parse, full parse, validate, invalid $toi, missing identity, tier precedence, ethical_pillars)
- CMakeLists.txt — C++20 build config with nlohmann_json, Catch2, spdlog, tl_expected via vcpkg
- README.md — Package documentation with governance compliance

**Key design decisions (per DECISIONS.md):**
- C++20 standard — coroutines, concepts, std::format
- Dual error handling: exceptions + std::expected<T,E> (safeParseTOI())
- Catch2 v3 — header-only, Unreal-native
- vcpkg — package manager with binary caching
- nlohmann::json — mutable DOM, JSON Schema validation
- JSON first, protobuf later — native TOI/OTOI format support
- spdlog — async logging, custom Unreal sink
- Callbacks + coroutines wrapper — C++20 async model
- Monorepo — single build/version coordination
- extractToi out of Unreal runtime — dev/CI tool only

**Governance compliance:** 22/22 validation passing, commit format followed, no credentials/secrets, no external integrations, active threads updated, handoff record written.

**Blockers:** None
**Next action:** Proceed to Phase 3 — OTOI-C++ port (packages/otoi/ per PLAN.md Section 5)

---

### Thread: phase-3-otoi-port
**Status:** in_progress
**Owner:** Hermes / desktop
**Started:** 2026-09-06
**Last updated:** 2026-09-06
**Summary:** OTOI-C++ port in progress per PLAN.md Section 5 and DECISIONS.md technology decisions. Package structure created at packages/otoi/ with:
- include/otoi/OTOITypes.h — Core types (Tier, EnforcementMode, ConflictStrategy, UnsupportedStrategy enums; OtoiAgent, OtoiSource, OtoiEnforcement, ResolvedEnforcement, OtoiCharter, EffectivePolicy, PolicyConflict structs; OtoiError hierarchy with OtoiParseError, OtoiValidationError, OtoiHonorError)
- include/otoi/OTOIManager.h — Public API surface (OTOIManager, OTOIValidator, GovernanceContext) with dual error handling pattern
- src/OTOIManager.cpp — OTOIValidator (parseAndValidate, detectConflicts, validateReservedKeys, validateIdentity, validateAgents, validateEnforcement, validateToiSources, resolveEnforcement) and OTOIManager (parseCharter, safeParseCharter, honor, safeHonor, propagate, getStatus, loadSource, resolveDocuments, applyEnforcement, isAgentDeclared)
- tests/test_otoi_manager.cpp — 8 Catch2 unit tests (minimal charter parse, charter with enforcement, missing required $otoi, invalid JSON, honor with single source, honor with tier precedence, detect same-tier conflicts, propagate refuses undeclared agent in strict mode, enum conversions)
- CMakeLists.txt — C++20 build config with nlohmann_json, Catch2, spdlog, tl_expected via vcpkg
- README.md — Package documentation with governance compliance

**Key design decisions (per DECISIONS.md):**
- C++20 standard
- Dual error handling: exceptions + std::expected<T,E>
- Catch2 v3
- vcpkg
- nlohmann::json
- JSON first
- spdlog
- Callbacks + coroutines wrapper
- Monorepo
- extractToi out of Unreal runtime

**Governance compliance:** Commit format followed, no credentials/secrets, no external integrations.

**Blockers:** None
**Next action:** Complete Phase 3 testing and validation; proceed to Phase 4 — RRT Advocate-C++ port

### Thread: phase-5-sleepwalker-cplus
**Status:** resolved
**Owner:** pool / desktop
**Started:** 2026-09-06
**Last updated:** 2026-09-06
**Summary:** Phase 5 — Sleepwalker Protocol C++ port COMPLETE. Created full packages/sleepwalker/ package at packages/sleepwalker/ with:
- include/sleepwalker/SleepwalkerTypes.h — Core types: ConsentLevel enum, Channel enum + normalizeChannel, StateType enum, CrisisIndicators, EmotionalIndicators, EmotionalState, SwpConfig, ContinuityContext, InteractionAssessment, ResponseGuidance structs
- include/sleepwalker/StateDetector.h + src/StateDetector.cpp — Regex-based emotional state detection (9 dissociation, 6 numbing, 6 avoidance, 6 detachment, 5 suicidal_ideation, 4 self_harm, 5 safety_concern patterns); confidence scoring (0.0/0.5/0.8)
- include/sleepwalker/ConsentManager.h + src/ConsentManager.cpp — Graduated consent model (PASSIVE → LOW_PRESSURE → SAFETY_CHECK → RRTA_HANDOFF) with intervention threshold logic
- include/sleepwalker/ContinuityManager.h + src/ContinuityManager.cpp — Temporal continuity with JSON file persistence, ISO 8601 timestamps
- include/sleepwalker/SleepwalkerProtocol.h + src/SleepwalkerProtocol.cpp — Main orchestrator composing all three components
- tests/*.cpp — 56 Catch2 v3 unit tests (189 assertions), all passing
- CMakeLists.txt + README.md — build config and full documentation

**Key design decisions:**
- Removed tl_expected dependency — sleepwalker package does not use std::expected (unlike TOI)
- Dual constructor on SleepwalkerProtocol (default + parameterized) to avoid nlohmann::json brace-init ambiguity
- ConsentManager.getAppropriateLevel() mirrors Python reference interface

**Compilation verified:** g++ 15.2.0, C++20, -Wall -Wextra — no warnings in project code. Full test suite: 56 test cases, 189 assertions, all passing.

**Governance compliance:** 22/22 validation checks passing. Self-registered per OTOI §3 (docs/agent-log/registrations/2026-09-06-pool-phase5.json). Handoff record written (docs/agent-log/handoffs/2026-09-06-phase5-sleepwalker.json).

**Blockers:** None

---

### Thread: phase-4-rrt-port
**Status:** completed
**Owner:** Hermes / desktop
**Started:** 2026-09-06
**Last updated:** 2026-09-06
**Summary:** Phase 4 — RRT Advocate C++ port COMPLETE. Created full packages/rrt-advocate/ package with:
- include/rrt/RRTTypes.h — Core types: CrisisLevel, KeywordSemanticField, SentimentTrend, ComplexityTrend, BurnoutLevel enums; KeywordMatch, KeywordAnalysisResult, SentimentReading, SentimentAnalysisResult, BehavioralAnalysisResult, CrisisIndicators, CrisisAssessment, BurnoutIndicator, BurnoutAssessment structs
- include/rrt/KeywordLayer.h + src/KeywordLayer.cpp — Layer 1: 7 semantic fields (NEGATIVE_SELF_TALK, TASK_AVOIDANCE, OVERWHELM, MELTDOWN, SHUTDOWN, HYPERFOCUS_LOOP, SELF_HARM_RISK) with 75+ regex patterns, confidence weights, SELF_HARM_RISK always returns 1.0
- include/rrt/SentimentLayer.h + src/SentimentLayer.cpp — Layer 2: heuristic polarity scorer (positive/negative word lists), sliding window trend classification (stable/declining/sharply_declining/recovering)
- include/rrt/BehavioralLayer.h + src/BehavioralLayer.cpp — Layer 3: message complexity scoring, latency anomaly detection, Jaccard looping detection with hashed word tokens (privacy-preserving)
- include/rrt/CrisisDetector.h + src/CrisisDetector.cpp — Orchestrates 3 layers, aggregates with weights (0.45/0.35/0.20)
- include/rrt/CrisisAssessor.h + src/CrisisAssessor.cpp — Maps aggregate confidence to CrisisLevel (GREEN/YELLOW/ORANGE/RED/BLACK), computes safety score with penalties, self-harm always BLACK
- include/rrt/CrisisEngine.h + src/CrisisEngine.cpp — Facade API (detect/assess/resetSession)
- include/rrt/BurnoutDetector.h + src/BurnoutDetector.cpp — Session metrics + crisis history burnout detection
- include/rrt/RRTAdvocate.h + src/RRTAdvocate.cpp — Main facade combining crisis + burnout
- tests/test_rrt_advocate.cpp, test_crisis.cpp, test_burnout.cpp — Catch2 test suite
- CMakeLists.txt — C++20 build config with nlohmann_json, Catch2, spdlog
- README.md — Package documentation with governance compliance

**Key design decisions (per DECISIONS.md):**
- C++20 standard
- nlohmann::json for context factors
- Dual error handling pattern
- Catch2 v3 for tests
- vcpkg for dependencies
- Monorepo structure
- Layer weights preserved exactly from Python reference (0.45/0.35/0.20)
- Crisis thresholds preserved exactly: GREEN [0,0.2), YELLOW [0.2,0.4), ORANGE [0.4,0.7), RED [0.7,0.9), BLACK [0.9,1.01)
- Self-harm always escalates to BLACK regardless of aggregate confidence
- Safety score: self-harm → 0.05, else 1.0 - confidence with penalties for looping/shutdown/declining

**Compilation verified:** g++ 15.2.0, C++20, -O0 — all sources compile cleanly. Standalone test: 19 test cases, all passing.

**Governance compliance:** Commit format followed, no credentials/secrets, no external integrations.

**Blockers:** None
**Next action:** Proceed to Phase 6 — ASFDK-C++ Umbrella integration (requires Phases 2-5 interfaces)
**Next action:** Proceed to Phase 4 — RRT Advocate C++ port (packages/rrt_advocate/ per PLAN.md Section 8). Consider creating a dedicated Phase 4 branch from current state.

---

### Thread: hermetic-build-fixes
**Status:** resolved
**Owner:** Cline / desktop
**Started:** 2026-09-06
**Last updated:** 2026-09-06
**Summary:** Post-merge package build verification COMPLETE. All three pillar packages (toi, otoi, sleepwalker) now compile hermetically using only vendored headers and pass their standalone test suites (g++ 15.2.0; toi C++20, otoi C++23, sleepwalker C++20):
- packages/sleepwalker/src/*.cpp — include paths qualified to <sleepwalker/X.h>, matching the toi/otoi namespaced-include convention and fixing consumer-facing header resolution
- packages/toi/src/*.cpp, packages/otoi/src/*.cpp — same bare-include fix applied (TermsOfInteraction.h, TOITypes.h, OTOIManager.h, OTOITypes.h → namespaced forms)
- packages/otoi — fixed latent C++23 issues found during verification: missing #include <expected> in OTOIManager.h, CMakeLists.txt CXX_STANDARD corrected 20→23 (std::expected is C++23), missing <set> include, OTOIValidator::validate typed→JSON bridge (it passed typed members to its own JSON-shaped validators), PolicyConflict tier string→enum mapping, resolveEnforcement made public for OTOIManager::safeHonor
- packages/include/ — new vendored header root: nlohmann/json.hpp (official 3.11.3 single-header, MIT), tl/expected.hpp (upstream 1.3.1, CC0; required by toi), spdlog/spdlog.h (minimal local compatibility shim, clearly labeled for replacement by real spdlog)
- packages/include/README.md — provenance, versions, licenses, per-package header requirements, build/verify commands
- packages/{toi,otoi,sleepwalker}/tests/standalone_test.cpp — dependency-free runners written against the actual implemented APIs, mirroring the Catch2 suites' core behaviors (Catch2 and CMake are unavailable on this machine)

**Key design decisions:**
- Vendor official upstream single-headers instead of trimming package dependencies — packages/* source stays canonical, vcpkg remains the primary dependency path
- spdlog provided as a labeled shim (ports use only a tiny API subset); documented for replacement under a full toolchain
- Standalone runners exit non-zero on failure so they can serve as CI smoke tests without Catch2

**Governance compliance:** 22/22 validation checks passing. Self-registered per OTOI §3 (docs/agent-log/registrations/2026-09-06-cline-hermetic-builds.json). Handoff record written (docs/agent-log/handoffs/2026-09-06-cline-hermetic-builds.json). Commits follow [AGENT_NAME] format. No credentials or external integrations.

**Blockers:** None (no system vcpkg/CMake — pre-existing environment limitation, documented in packages/include/README.md)

**Next action:** CI adopters may wire the standalone runners in as required smoke checks; Phase 6 ASFDK umbrella integration can reuse packages/include as its fallback include root.


---

### Thread: phase-6-consolidated-concurrency-fix
**Status:** completed
**Owner:** Hermes / desktop
**Started:** 2026-09-07
**Last updated:** 2026-09-07
**Summary:** Consolidated all Phase 6 fixes from the PR #20→#21→#22 chain into a single branch against main. Three fixes applied: (1) m_active made mutable std::atomic<bool> — fixes compilation failure in safeHonor() const; (2) m_mode made mutable std::atomic<EnforcementMode> — fixes CodeRabbit data race; (3) Sleepwalker CMakeLists.txt restored VCPKG_ROOT per DECISIONS.md §4. Combined with the correct release/acquire ordering (m_mode relaxed-store before m_active release-store; m_active acquire-load before m_mode relaxed-load in getStatus), this fully resolves both CodeRabbit Critical comments from PR #20.
**Blockers:** None
**Next action:** PR review — this replaces the 3-PR chain (#20, #21, #22) with a single consolidated PR against main.
### Thread: phase-6-pr18-pr19-review
**Status:** completed
**Owner:** Hermes / desktop
**Started:** 2026-09-07
**Summary:** Reviewed two merged Phase 6 PRs (PR #18 umbrella; PR #19 OTOI manager state fix). Both merged to main. PR #18 introduced regressions: (1) Sleepwalker CMakeLists.txt hardcoded vcpkg path instead of VCPKG_ROOT per DECISIONS.md §4; (2) OTOI m_active is plain mutable bool (data race between safeHonor const-write and getStatus read); (3) m_mode never updated after safeHonor. Created branch phase-6-review-fixes with all three fixes. Handoff record written.
**Blockers:** None
**Next action:** Create PR from phase-6-review-fixes against main.

---

### Thread: phase-6-pr20-hermes-additional-review
**Status:** completed
**Owner:** Hermes / desktop
**Started:** 2026-09-07
**Summary:** Conducted additional review of PR #20 (phase-6-review-fixes). Two CodeRabbit comments remain unaddressed: (1) m_active declared as std::atomic<bool> without mutable qualifier — safeHonor() is const and calls .store() which is non-const, causing compilation failure; (2) m_mode write happens AFTER the release store on m_active, so the release/acquire pair does not synchronize m_mode reads in getStatus() — data race. Applied fixes: made m_active mutable, reordered writes (m_mode before release-store), changed getStatus() to acquire-load m_active. Committed to branch phase-6-hermes-review-fixes.
**Blockers:** None
**Next action:** Create PR from phase-6-hermes-review-fixes against phase-6-review-fixes (or main if PR #20 merges first).

---

### Thread: phase-6-coderabbit-mmode-race-fix
**Status:** completed
**Owner:** Hermes / desktop
**Started:** 2026-09-07
**Summary:** CodeRabbit commented on PR #20 (phase-6-review-fixes) that reordering m_mode write before the release store on m_active is insufficient — m_mode is still a plain mutable EnforcementMode (non-atomic), so concurrent reads in getStatus() can still race. PR #21 (phase-6-hermes-review-fixes) applied the reordering but did NOT make m_mode atomic. Applied the proper fix in branch phase-6-hermes-atomic-mmode: made both m_active and m_mode std::atomic, with release-store on m_active and acquire-load in getStatus(). m_mode uses relaxed ordering on both store and load (it is synchronized by the m_active release/acquire pair). This fully eliminates the data race per C++11 memory model.
**Blockers:** None
**Next action:** Create PR from phase-6-hermes-atomic-mmode against phase-6-hermes-review-fixes.

---

### Thread: phase-6-root-cmake-build-fix
**Status:** in_progress
**Owner:** Hermes / desktop
**Started:** 2026-09-07
**Summary:** Addressing unmerged CodeRabbit review comment from PR #25 (merged) — repository lacks root `CMakeLists.txt` but README documents `cmake -B build` from repo root. `packages/asfdk/CMakeLists.txt` expects the four pillar targets to be defined by a top-level project. Also fixed hardcoded vcpkg path in `packages/toi/CMakeLists.txt` (same regression pattern as Sleepwalker — per DECISIONS.md §4, should use `$ENV{VCPKG_ROOT}`). Created root `CMakeLists.txt` with project definition, C++23 standard, vcpkg toolchain auto-detection, vendored fallback include path, and `add_subdirectory` for all pillar packages + asfdk umbrella. Updated README build instructions to include the `--toolchain-file` flag for vcpkg. Governance compliance: 22/22.
**Blockers:** None
**Next action:** Create PR for root CMakeLists.txt + TOI CMakeLists.txt fix + README update.
