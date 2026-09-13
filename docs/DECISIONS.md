# ASFDK C++ Technology Decisions

**Repository:** `NeuroLift-Technologies/asfdk-cplus`
**Date:** 2026-09-06
**Governed by:** ORG-DEV-OTOI-1.0.3
**Status:** Final

---

## Purpose

This document records the technology decisions made for the ASFDK C++ port (Phases 2-7). All implementation work in `packages/`, `unreal/`, and related build infrastructure must conform to these decisions.

These decisions resolve the open questions from `docs/PLAN.md` Section 14 and `docs/ref_PORT_MATRIX_TOI.md` Open Questions.

---

## Decisions

### 1. C++ Standard

**Decision:** C++20

**Rationale:**
- Unreal Engine 5.8 fully supports C++20
- Coroutines (`co_await`) needed for async governance model (Decision 8)
- `std::format` useful for logging and error messages
- Concepts provide better template error messages
- C++23 is too new for stable Unreal toolchain across all platforms

**Implications:**
- Minimum compiler: Clang 14+, GCC 11+, MSVC 19.30+
- No C++23-only features (e.g., `std::expected` from stdlib — use `tl::expected`)

---

### 2. Error Handling Strategy

**Decision:** Both — exceptions AND `std::expected<T, E>`

**Rationale:**
- Original TypeScript reference has a deliberate dual API: `parseToi()` throws, `safeParseToi()` returns a result object
- Behavioral parity across language ports is a hard requirement
- `std::expected` provides explicit error handling at API boundaries
- Exceptions provide ergonomic error propagation in internal code

**Implications:**
- Public APIs expose both throwing and non-throwing variants
- Internal implementation may use exceptions freely
- `std::expected` provided via `tl::expected` (C++17 backport) or equivalent
- Unreal-facing boundaries translate exceptions to `std::expected` or Unreal delegates

**API Pattern:**
```cpp
// Throwing variant
ToiDocument parseToi(const nlohmann::json& input);

// Non-throwing variant
std::expected<ToiDocument, ToiError> safeParseToi(const nlohmann::json& input);
```

---

### 3. Test Framework

**Decision:** Catch2 v3 (header-only)

**Rationale:**
- Unreal Engine has native Catch2 integration
- Header-only simplifies CMake setup
- Faster compile times than GoogleTest
- De facto standard for game-adjacent C++ codebases

**Implications:**
- Tests use `TESTCASE` / `SECTION` macros
- CI runs tests via `ctest`
- No separate Catch2 compilation step

---

### 4. Package Manager

**Decision:** vcpkg

**Rationale:**
- Microsoft-backed with strong CMake integration
- Works with Unreal's CMake build system
- Binary caching via GitHub Actions for CI speed
- Large existing package ecosystem

**Implications:**
- `vcpkg.json` manifest in repo root
- Dependencies: `nlohmann-json`, `catch2`, `spdlog`, `tl-expected`
- CI caches vcpkg artifacts between builds
- Unreal module builds against vcpkg-provided dependencies

---

### 5. JSON Library

**Decision:** `nlohmann::json`

**Rationale:**
- Mutable DOM required for OTOI tier-merge logic
- JSON Schema validation via `nlohmann-json-schema-validator`
- Excellent UTF-8 support (critical for JCS canonicalization per RFC 8785)
- Widely adopted with good error messages
- Available in vcpkg

**Implications:**
- All JSON parsing/serialization goes through `nlohmann::json`
- No mixing with Unreal's `FJsonObject` at library boundaries
- Conversion layer at Unreal integration boundary only

---

### 6. Serialization Format

**Decision:** JSON first, protobuf later

**Rationale:**
- TOI/OTOI specs are JSON — JSON must be natively supported
- Existing TS/Python interop uses JSON
- Protobuf can be added for wire efficiency after the port is stable

**Implications:**
- Phase 2-6 implementations use JSON exclusively
- Protobuf support can be added as an optional serialization layer later
- No protobuf dependency in initial builds

---

### 7. Logging

**Decision:** spdlog

**Rationale:**
- `std::format` syntax (C++20)
- Async logging (non-blocking for game thread)
- Available in vcpkg
- Can route to Unreal's `UE_LOG` via custom sink

**Implications:**
- Core libraries use `spdlog::logger` interfaces
- Unreal integration provides custom sink routing to `UE_LOG`
- No direct `UE_LOG` calls in core libraries

---

### 8. Async Model

**Decision:** Callbacks + coroutines wrapper

**Rationale:**
- C++20 coroutines provide sequential-looking async code
- Callbacks for low-level async (HTTP, file I/O)
- Coroutines wrap callbacks for ergonomic caller code
- Unreal's task graph can integrate with both

**Implications:**
- Low-level I/O uses callback-based APIs (`std::function<void(Result)>`)
- C++20 coroutines (`co_await`) wrap callbacks at higher levels
- No `std::future`/`std::async` in hot paths (thread overhead)
- Unreal integration uses `AsyncTask` or coroutine wrappers

---

### 9. Repository Structure

**Decision:** Monorepo (single build, single version)

**Rationale:**
- Porting effort requires tight coordination between pillars
- All pillars share the same C++ stdlib, nlohmann::json, Catch2, spdlog
- Version drift between TOI/OTOI/RRT/Sleepwalker is unacceptable during porting
- Single CMake build = faster CI

**Implications:**
- All pillar packages under `packages/`
- Single top-level `CMakeLists.txt`
- Single version number for all packages
- Future split possible if independent release cadences needed

---

### 10. `extractToi` Scope

**Decision:** Out for Unreal runtime (dev/CI tool only)

**Rationale:**
- `extractToi` is a pure regex/phrase-matching module (~452 lines)
- Explicitly documented as "no LLM, no network"
- Unreal WorldEngine doesn't need NL extraction at runtime
- Adds regex engine dependency (PCRE2/RE2) to runtime binary
- Useful for CI pipelines, dev tools, content pipelines

**Implications:**
- `extractToi` not included in `packages/toi/`
- May be packaged as separate CLI tool or CI utility in future
- TOI runtime port focuses on: schema, validation, canonicalization, signing, tier resolution

---

## Dependency Summary

| Category | Choice | vcpkg Package |
|----------|--------|---------------|
| C++ Standard | C++20 | (compiler) |
| JSON | nlohmann::json | `nlohmann-json` |
| JSON Schema | nlohmann-json-schema-validator | (may need custom port) |
| Testing | Catch2 v3 | `catch2` |
| Logging | spdlog | `spdlog` |
| Async | coroutines + callbacks | (C++20 stdlib) |
| Expected | `std::expected` backport | `tl-expected` |
| Package Manager | vcpkg | (toolchain) |

---

## Build Configuration

### CMake Minimum
```cmake
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

### vcpkg Manifest (`vcpkg.json`)
```json
{
  "name": "asfdk-cplus",
  "version": "1.0.0",
  "dependencies": [
    "nlohmann-json",
    "catch2",
    "spdlog",
    "tl-expected"
  ]
}
```

---

## References

- **Phase 1 Analysis:** `docs/phase-1-reference-analysis.md`
- **TOI Port Matrix:** `docs/ref_PORT_MATRIX_TOI.md`
- **Implementation Plan:** `docs/PLAN.md`
- **Architecture Handoff:** `docs/ASFDK-C++-HANDOFF.md`

---

*Decisions are final. Escalate to Joshua W. Dorsey, Sr. for amendments.*