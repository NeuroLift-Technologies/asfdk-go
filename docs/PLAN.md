# ASFDK C++ Implementation Plan

**Repository:** `NeuroLift-Technologies/asfdk-cplus`
**Created:** 2026-09-06
**Agent:** Hermes / desktop
**Governed by:** ORG-DEV-OTOI-1.0.3
**Status:** Draft

---

## 1. Purpose

This document establishes the implementation plan for the ASFDK C/C++ port based on the architecture handoff from GPT-5.6 Luna (`docs/ASFDK-C++-HANDOFF.md`).

The plan is organized into phases that follow the recommended porting strategy:

1. Reverse-engineer reference packages
2. Port each pillar independently (TOI, OTOI, RRT Advocate, Sleepwalker)
3. Build ASFDK-C++ umbrella
4. Integrate with Unreal Engine

---

## 2. Phase 0 — Repository Setup (Complete)

### Deliverables
- [x] Governance scaffolding (22/22 checks passing)
- [x] Architecture handoff document
- [x] This implementation plan

### Validation
```bash
bash .nltotoi/scripts/validate-governance.sh
```

---

## 3. Phase 1 — Discovery & Reference Analysis

### 3.1 Inspect Reference Packages

| Package | Location | Key Files | Status |
|---------|----------|-----------|--------|
| TOI | `asfdk/src/toi/` | `TermsOfInteraction.ts`, `TOIManager.ts` | ⬜ Not analyzed |
| OTOI | `asfdk/src/otoi/` | `OTOIManager.ts`, `OTOIValidator.ts` | ⬜ Not analyzed |
| RRT Advocate | `rrt-advocate/src/` | `RRTAdvocate.ts`, `CrisisIntervention.ts` | ⬜ Not analyzed |
| Sleepwalker | `sleepwalker/src/` | `SleepwalkerProtocol.ts`, `continuity.ts` | ⬜ Not analyzed |

### 3.2 Produce API/Behavior Mapping

For each package, document:

- Public API surface
- Data structures and types
- Lifecycle and state transitions
- Validation rules and error behavior
- Serialization formats
- Security/governance behavior
- Dependencies and invariants
- Test coverage and patterns

### 3.3 Inspect Existing Ports

| Repo | Location | Status |
|------|----------|--------|
| asfdk-kotlin | `/home/joshd/Desktop/nlt-repos/asfdk-kotlin/` | ⬜ Not inspected |
| asfdk-csharp | `/home/joshd/Desktop/nlt-repos/asfdk-csharp/` | ⬜ Not inspected |

### 3.4 Inspect WorldEngine Integration Point

| Item | Location | Status |
|------|----------|--------|
| NLTGovernanceSubsystem (planned) | `nlt-world-engine/WorldEngine/Source/` | ⬜ Not found |
| Mass Entity fragments | `nlt-world-engine/WorldEngine/Source/` | ⬜ Not found |
| AvatarCharacter | `nlt-world-engine/WorldEngine/Source/` | ⬜ Not found |
| AvatarAIController | `nlt-world-engine/WorldEngine/Source/` | ⬜ Not found |

### 3.5 Deliverable

**Phase 1 Output:** `docs/phase-1-reference-analysis.md`

Contains:
- API surface for each package
- Data structure mappings
- Behavior contracts
- Integration point analysis

---

## 4. Phase 2 — TOI-C++ Port

### 4.1 Scope

Port the Terms of Interaction (TOI) package to idiomatic C++.

### 4.2 Proposed Structure

```
packages/toi/
├── include/
│   └── toi/
│       ├── TermsOfInteraction.h
│       ├── TOIManager.h
│       └── TOITypes.h
├── src/
│   ├── TermsOfInteraction.cpp
│   └── TOIManager.cpp
├── tests/
│   ├── test_toi_manager.cpp
│   └── test_terms.cpp
├── CMakeLists.txt
└── README.md
```

### 4.3 Key C++ Design Decisions

| TypeScript Concept | C++ Equivalent | Rationale |
|-------------------|----------------|-----------|
| `interface` | `struct` + `virtual` base | Idiomatic C++ polymorphism |
| `enum` | `enum class` | Scoped, type-safe enums |
| `Promise<T>` | `std::future<T>` or callbacks | Async without TS runtime |
| `optional T` | `std::optional<T>` | Standard optional type |
| `Record<K,V>` | `std::unordered_map<K,V>` | Standard hash map |
| GC-managed objects | RAII / smart pointers | Deterministic lifecycle |

### 4.4 Tasks

- [ ] Create package directory structure
- [ ] Define core types (`TOITypes.h`)
- [ ] Implement `TermsOfInteraction` class
- [ ] Implement `TOIManager` class
- [ ] Write unit tests
- [ ] Create CMake build configuration
- [ ] Update governance file registry

### 4.5 Deliverable

**Phase 2 Output:** Working TOI-C++ library with passing tests

---

## 5. Phase 3 — OTOI-C++ Port

### 5.1 Scope

Port the OTOI (Operations Terms of Interaction) enforcement/governance layer.

### 5.2 Proposed Structure

```
packages/otoi/
├── include/
│   └── otoi/
│       ├── OTOIManager.h
│       ├── OTOIValidator.h
│       ├── OTOITypes.h
│       └── GovernanceContext.h
├── src/
│   ├── OTOIManager.cpp
│   └── OTOIValidator.cpp
├── tests/
│   ├── test_otoi_manager.cpp
│   └── test_otoi_validator.cpp
├── CMakeLists.txt
└── README.md
```

### 5.3 Key Focus Areas

- Interaction boundaries
- Agent identity and authorization
- Enforcement and validation
- Escalation protocols
- Trusted/untrusted interaction state
- Governance events
- Audit trail/logging

### 5.4 Tasks

- [ ] Create package directory structure
- [ ] Define governance types (`OTOITypes.h`)
- [ ] Implement `GovernanceContext`
- [ ] Implement `OTOIValidator`
- [ ] Implement `OTOIManager`
- [ ] Write unit tests
- [ ] Create CMake build configuration
- [ ] Update governance file registry

### 5.5 Deliverable

**Phase 3 Output:** Working OTOI-C++ library with passing tests

---

## 6. Phase 4 — RRT Advocate-C++ Port

### 6.1 Scope

Port the Rapid Response Team Advocate package.

### 6.2 Proposed Structure

```
packages/rrt-advocate/
├── include/
│   └── rrt/
│       ├── RRTAdvocate.h
│       ├── CrisisIntervention.h
│       ├── BurnoutDetector.h
│       └── RRTTypes.h
├── src/
│   ├── RRTAdvocate.cpp
│   ├── CrisisIntervention.cpp
│   └── BurnoutDetector.cpp
├── tests/
│   ├── test_rrt_advocate.cpp
│   ├── test_crisis.cpp
│   └── test_burnout.cpp
├── CMakeLists.txt
└── README.md
```

### 6.3 Key Focus Areas

- Crisis assessment and intervention
- Burnout detection
- Escalation to human authority
- Recovery protocols

### 6.4 Tasks

- [ ] Create package directory structure
- [ ] Define RRT types (`RRTTypes.h`)
- [ ] Implement `BurnoutDetector`
- [ ] Implement `CrisisIntervention`
- [ ] Implement `RRTAdvocate`
- [ ] Write unit tests
- [ ] Create CMake build configuration
- [ ] Update governance file registry

### 6.5 Deliverable

**Phase 4 Output:** Working RRT Advocate-C++ library with passing tests

---

## 7. Phase 5 — Sleepwalker-C++ Port

### 7.1 Scope

Port the Sleepwalker Protocol package (continuity/persistence).

### 7.2 Proposed Structure

```
packages/sleepwalker/
├── include/
│   └── sleepwalker/
│       ├── SleepwalkerProtocol.h
│       ├── ContinuityManager.h
│       └── SleepwalkerTypes.h
├── src/
│   ├── SleepwalkerProtocol.cpp
│   └── ContinuityManager.cpp
├── tests/
│   ├── test_sleepwalker.cpp
│   └── test_continuity.cpp
├── CMakeLists.txt
└── README.md
```

### 7.3 Tasks

- [ ] Create package directory structure
- [ ] Define Sleepwalker types
- [ ] Implement `ContinuityManager`
- [ ] Implement `SleepwalkerProtocol`
- [ ] Write unit tests
- [ ] Create CMake build configuration
- [ ] Update governance file registry

### 7.5 Deliverable

**Phase 5 Output:** Working Sleepwalker-C++ library with passing tests

---

## 8. Phase 6 — ASFDK-C++ Umbrella

### 8.1 Scope

Build the ASFDK umbrella that composes the four pillar packages.

### 8.2 Proposed Structure

```
packages/asfdk/
├── include/
│   └── asfdk/
│       ├── ASFDK.h
│       └── ASFDKTypes.h
├── src/
│   └── ASFDK.cpp
├── tests/
│   └── test_asfdk_composition.cpp
├── CMakeLists.txt
└── README.md
```

### 8.3 Composition Pattern

```cpp
// ASFDK.h
#include <toi/TOIManager.h>
#include <otoi/OTOIManager.h>
#include <rrt/RRTAdvocate.h>
#include <sleepwalker/SleepwalkerProtocol.h>

namespace asfdk {

class ASFDK {
    toi::TOIManager m_toi;
    otoi::OTOIManager m_otoi;
    rrt::RRTAdvocate m_rrt;
    sleepwalker::SleepwalkerProtocol m_sleepwalker;
    
public:
    // Unified API surface
    // Composes all four pillars
};

} // namespace asfdk
```

### 8.4 Tasks

- [ ] Create umbrella package structure
- [ ] Implement `ASFDK` composition class
- [ ] Write integration tests
- [ ] Ensure no pillar logic duplication
- [ ] Create top-level CMakeLists.txt
- [ ] Update governance file registry

### 8.5 Deliverable

**Phase 6 Output:** Working ASFDK-C++ umbrella with integration tests

---

## 9. Phase 7 — Unreal Integration

### 9.1 Scope

Build the NLTGovernanceSubsystem for Unreal Engine integration.

### 9.2 Proposed Structure

```
unreal/
└── NLTGovernanceSubsystem/
    ├── Source/
    │   ├── NLTGovernanceSubsystem.h
    │   ├── NLTGovernanceSubsystem.cpp
    │   ├── NLTAgentComponent.h
    │   ├── NLTAgentComponent.cpp
    │   └── NLTGovernanceModule.h
    ├── Tests/
    │   └── test_governance_subsystem.cpp
    └── NLTGovernanceSubsystem.Build.cs
```

### 9.3 Integration Points

| Unreal System | Governance Boundary |
|---------------|---------------------|
| `AActor` | Agent identity, governance context |
| `UActorComponent` | Per-component governance state |
| `UWorldSubsystem` | World-level governance manager |
| `UMassEntity` | Fragment-based governance |
| `UAIController` | Decision governance |

### 9.4 Tasks

- [ ] Create Unreal module structure
- [ ] Implement `NLTGovernanceSubsystem` (UWorldSubsystem)
- [ ] Implement `NLTAgentComponent` (UActorComponent)
- [ ] Wire ASFDK-C++ into Unreal types
- [ ] Write environment tests
- [ ] Document integration API
- [ ] Update governance file registry

### 9.5 Deliverable

**Phase 7 Output:** Working NLTGovernanceSubsystem with environment tests

---

## 10. Build System

### 10.1 Top-Level CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(asfdk-cplus VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core packages
add_subdirectory(packages/toi)
add_subdirectory(packages/otoi)
add_subdirectory(packages/rrt-advocate)
add_subdirectory(packages/sleepwalker)
add_subdirectory(packages/asfdk)

# Unreal integration (optional, requires UE5)
if(BUILD_UNREAL)
    add_subdirectory(unreal/NLTGovernanceSubsystem)
endif()

# Testing
enable_testing()
```

### 10.2 Build Targets

| Target | Type | Dependencies |
|--------|------|--------------|
| `toi-cplus` | Static lib | None |
| `otoi-cplus` | Static lib | None |
| `rrt-cplus` | Static lib | None |
| `sleepwalker-cplus` | Static lib | None |
| `asfdk-cplus` | Static lib | All four pillars |
| `NLTGovernanceSubsystem` | UE Module | asfdk-cplus |

---

## 11. Testing Strategy

### 11.1 Test Levels

| Level | Scope | Tool |
|-------|-------|------|
| Unit | Individual classes/funcs | Catch2/GoogleTest |
| Integration | Pillar composition | Catch2/GoogleTest |
| Environment | Unreal integration | UE Automation |

### 11.2 Test Coverage Requirements

| Package | Unit | Integration | Environment |
|---------|------|-------------|-------------|
| TOI | ✅ Required | N/A | N/A |
| OTOI | ✅ Required | N/A | N/A |
| RRT | ✅ Required | N/A | N/A |
| Sleepwalker | ✅ Required | N/A | N/A |
| ASFDK | ✅ Required | ✅ Required | N/A |
| NLTGovernanceSubsystem | ✅ Required | ✅ Required | ✅ Required |

---

## 12. Timeline & Milestones

| Phase | Deliverable | Estimated Effort | Dependencies |
|-------|-------------|------------------|--------------|
| 0 | Repo setup | ✅ Complete | — |
| 1 | Reference analysis | 1-2 sessions | — |
| 2 | TOI-C++ | 2-3 sessions | Phase 1 |
| 3 | OTOI-C++ | 3-4 sessions | Phase 1 |
| 4 | RRT-C++ | 2-3 sessions | Phase 1 |
| 5 | Sleepwalker-C++ | 2-3 sessions | Phase 1 |
| 6 | ASFDK-C++ | 1-2 sessions | Phases 2-5 |
| 7 | Unreal integration | 3-4 sessions | Phase 6 |

---

## 13. Governance Compliance

Throughout all phases:

- All commits follow `[AGENT_NAME] type(scope): description`
- `validate-governance.sh` passes (22/22 checks)
- Agent registration maintained
- Handoff records written at session end
- No OTOI self-amendment without formal process
- CI governance validation passes

---

## 14. Open Questions

| # | Question | Impact | Status |
|---|----------|--------|--------|
| 1 | Should pillar packages be separate git submodules? | Build complexity | ⬜ Open |
| 2 | C++ standard: C++17 or C++20? | Feature availability | ⬜ Open |
| 3 | Test framework: Catch2 or GoogleTest? | Dependencies | ⬜ Open |
| 4 | Package manager: vcpkg, Conan, or none? | Distribution | ⬜ Open |
| 5 | How to handle async: coroutines or callbacks? | API design | ⬜ Open |
| 6 | Serialization format: JSON, protobuf, or both? | Interop | ⬜ Open |
| 7 | Logging: spdlog, custom, or pluggable? | Observability | ⬜ Open |

---

## 15. References

- **Architecture Handoff:** `docs/ASFDK-C++-HANDOFF.md`
- **Original ASFDK:** `/home/joshd/Desktop/nlt-repos/asfdk/`
- **Kotlin Port:** `/home/joshd/Desktop/nlt-repos/asfdk-kotlin/`
- **C# Port:** `/home/joshd/Desktop/nlt-repos/asfdk-csharp/`
- **WorldEngine:** `/home/joshd/Desktop/nlt-repos/nlt-world-engine/`
- **Org Governance:** `/home/joshd/Desktop/nlt-repos/.github-private/`

---

*This plan is a living document. Update as implementation progresses.*