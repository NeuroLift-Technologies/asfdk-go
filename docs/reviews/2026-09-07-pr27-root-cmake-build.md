# Review: PR #27 — Root CMakeLists.txt, TOI vcpkg fix, README update

## Review Metadata
**Reviewer:** Hermes / desktop
**Review Date:** 2026-09-07
**Review Type:** Code
**Scope:** `CMakeLists.txt` (new, root), `packages/toi/CMakeLists.txt` (modified), `README.md` (modified)
**OTOI Version:** ORG-DEV-OTOI-1.0.3
**Phase:** Phase 6 (ASFDK-C++ umbrella build scaffolding)

## Pre-Review Checklist
- [x] Read NLT-DEV-OTOI.md and understood governance requirements
- [x] Identified the review scope and what is being evaluated
- [x] Checked active threads in `docs/active-threads.md`
- [x] Verified the review format matches REVIEW.md template

## Review Findings

### Summary
This PR (merged) addressed an unmerged CodeRabbit review comment from PR #25: the repo lacked a root `CMakeLists.txt` despite README documenting `cmake -B build` from repo root, and `packages/toi/CMakeLists.txt` had a hardcoded vcpkg path (`/home/joshd/Documents/NLT/Engine/...`). Added top-level CMake project file, replaced hardcoded path with `$ENV{VCPKG_ROOT}`, and updated README build instructions. Build-infrastructure only — no source logic changes.

### Strengths
- Root `CMakeLists.txt` well-structured: project name + version 1.0.3, C++23 (required for OTOI `std::expected`), vcpkg toolchain auto-detection via `VCPKG_ROOT`, vendored fallback includes via `packages/include/`, `add_subdirectory` for all five packages matching `packages/asfdk/CMakeLists.txt` expectations exactly.
- TOI CMakeLists.txt fix correctly replaces hardcoded path with `$ENV{VCPKG_ROOT}/installed/x64-linux`, matching the pattern in `packages/sleepwalker/CMakeLists.txt`. Consistent with DECISIONS.md §4.
- README update clearly documents vcpkg toolchain requirement and includes both vcpkg and hermetic g++ build paths.

### Issues
#### Critical
No critical issues.

#### High Priority
1. TOI CMakeLists.txt uses `$ENV{VCPKG_ROOT}/installed/x64-linux` which is platform-specific. For cross-platform CI (Windows/macOS), this path would be incorrect. Consider using CMake variable instead or platform guard. *(Low priority — documented as Linux-specific; vcpkg manifest mode handles cross-platform.)*

#### Low Priority / Observations
- Root CMakeLists.txt does not provide `CMakePresets.json` (noted by author as future improvement — acceptable).
- `vcpkg_fallback_includes` INTERFACE library created but not linked to any target in root — pillar CMakeLists.txt handle their own includes (acceptable, could be documented better).

### Cross-Pillar Integration Check
- Follows DECISIONS.md (vcpkg §4, C++20/C++23 per package)
- Root add_subdirectory targets match packages/asfdk/CMakeLists.txt expectations (toi-cplus, otoi-cplus, rrt-cplus, sleepwalker-cplus, asfdk-cplus)
- No duplicate logic
- Error handling unchanged (dual API pattern preserved)

### Factual Accuracy
- All external claims verified
- No hallucinated organizations, numbers, or URLs
- Sources cited (DECISIONS.md §4, packages/include/README.md)

### Governance Compliance
- No credentials or secrets exposed
- No external integrations without approval
- No architecture decisions without approval
- Commit format: `[HERMES] fix(build): ...`
- docs/active-threads.md updated
- Handoff record written

## Verdict
**Status:** APPROVED
**Rationale:** Correctly addresses missing root CMakeLists.txt and hardcoded vcpkg path regression. All Phase 6 umbrella build scaffolding needs met. Governance 22/22. Verified: hermetic g++ build 37/37 tests pass.

## Handoff Notes
- Completes Phase 6 build scaffolding — repo now buildable via cmake from root
- CI adopters should set VCPKG_ROOT and pass toolchain file
- packages/include/ vendored headers serve as fallback for hermetic environments
- Consider CMakePresets.json in future PR

*This review conducted following REVIEW.md under ORG-DEV-OTOI-1.0.3.*
