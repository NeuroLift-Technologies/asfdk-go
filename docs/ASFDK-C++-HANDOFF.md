# ASFDK C/C++ PORT

## Architectural & Implementation Handoff

**From:** ChatGPT — GPT-5.6 Luna
**To:** Hermes — NeuroLift Technologies Development Agent
**Project:** `NeuroLift-Technologies/asfdk-cplus`
**Organization:** NeuroLift Technologies
**Governance:** Solidarity Framework / HAIEF
**Governance Reference:** ORG-DEV-OTOI-1.0.3
**Status:** Architecture / implementation handoff

---

# 1. Purpose

Hermes,

This document establishes the intended architecture and implementation direction for the C/C++ port of the NeuroLift Technologies Agent Solidarity Framework Development Kit (ASFDK).

The most important architectural decision is:

> **ASFDK is an umbrella/composition layer over four independent Solidarity Framework packages.**

The C/C++ effort therefore should **not** begin as a monolithic rewrite of ASFDK.

The four underlying capabilities should be ported independently, after which the C/C++ ASFDK layer should compose those implementations into a unified development kit.

The current `asfdk-cplus` repository is correctly understood as a **governance and integration scaffold** upon which those implementations will be built.

---

# 2. Reference Architecture

The original ASFDK consists of four primary packages:

```text
ASFDK
│
├── TOI
├── OTOI
├── RRT Advocate
└── Sleepwalker Protocol
```

These are independently meaningful components.

ASFDK provides the umbrella/runtime-facing composition layer that brings them together.

The C++ architecture should preserve this separation.

```text
                    ASFDK-C++
                       │
       ┌───────────────┼────────────────┬────────────────┐
       │               │                │                │
   TOI-C++          OTOI-C++       RRT Advocate-C++   Sleepwalker-C++
       │               │                │                │
       └───────────────┴────────────────┴────────────────┘
                       │
                ASFDK Composition
                       │
       ┌───────────────┴────────────────┐
       │                                │
 Unreal Engine                     Native C++
 WorldEngine                       Applications
       │
 NLTGovernanceSubsystem
       │
 ┌─────┼───────────┐
 │     │           │
Mass  Avatars   AI Controllers
Entity
```

The exact repository/package naming may be determined during implementation, but the **architectural separation must remain**.

---

# 3. What `asfdk-cplus` Is Today

The current repository is a governance scaffold.

It establishes:

* governance documentation
* OTOI compliance structure
* agent registration
* canonical templates
* governance validation
* CI governance validation
* C/C++ integration targets

The current repository explicitly identifies these future integration targets:

* `NLTGovernanceSubsystem`
* Unreal Engine C++ governance subsystem
* native C++ application boundaries
* Fusion ↔ Unreal semantic/physical reality bridge
* Mass Entity governance compliance

The existing governance validator reports 22 compliance checks.

This scaffold should be preserved.

Do **not** remove or bypass the governance infrastructure simply because native implementation work is beginning.

---

# 4. The Critical Unreal Problem

The primary immediate integration gap is:

```text
nlt-world-engine
        │
        ▼
 Unreal Engine C++
        │
        X
 NLTGovernanceSubsystem
 not yet implemented
```

The WorldEngine currently has a planned governance boundary, but the actual C++ subsystem needs to be implemented.

This subsystem is the bridge between:

```text
Solidarity Framework governance
            ↕
      Unreal Engine
            ↕
        AI agents
```

Without this boundary, C++ agents operating inside the WorldEngine can function technically while remaining outside the intended governance architecture.

That includes systems such as:

* Mass Entity agents
* AvatarCharacters
* AI controllers
* other C++ agent implementations
* future native AI components

Therefore, `NLTGovernanceSubsystem` should be treated as a **first-class integration target**, not merely an example implementation.

---

# 5. Porting Strategy

The recommended implementation sequence is:

## Phase 1 — Reverse-engineer the reference packages

Before implementing substantial C++ code, establish the authoritative behavior of each existing package.

For each package document:

* public API
* data structures
* lifecycle
* state transitions
* validation rules
* error behavior
* serialization formats
* security/governance behavior
* dependencies
* tests
* invariants

The goal is to produce a language-neutral behavioral contract.

Do not translate TypeScript syntax directly into C++.

Translate **semantics**.

---

# 6. Phase 2 — Port TOI

Create the native C++ implementation of the TOI layer.

TOI should remain independently usable.

The C++ implementation should expose the equivalent conceptual capabilities of the reference implementation while respecting C++ conventions.

Potential organization:

```text
toi-cplus/
├── include/
│   └── toi/
├── src/
│   └── toi/
├── tests/
├── CMakeLists.txt
└── README.md
```

The exact structure can differ, but the package must remain independently consumable.

---

# 7. Phase 3 — Port OTOI

OTOI is the enforcement/governance layer.

Its implementation must preserve the governance semantics of the reference package.

Pay particular attention to:

* interaction boundaries
* agent identity
* authorization
* enforcement
* validation
* escalation
* trusted/untrusted interaction state
* governance events
* auditability

OTOI must not become merely a collection of helper functions.

It is an enforcement boundary.

Potential organization:

```text
otoi-cplus/
├── include/
│   └── otoi/
├── src/
│   └── otoi/
├── tests/
├── CMakeLists.txt
└── README.md
```

---

# 8. Phase 4 — Port RRT Advocate

The RRT Advocate implementation should remain a distinct package.

It represents the response/escalation capability of the framework.

The implementation should preserve the reference package's conceptual separation from OTOI.

In particular:

```text
OTOI
 │
 │ governance/enforcement
 ▼
Agent interaction
 │
 ▼
RRT Advocate
 │
 │ response/escalation
 ▼
appropriate intervention
```

Do not collapse RRT functionality into OTOI merely because both participate in governance.

They serve different architectural purposes.

---

# 9. Phase 5 — Port Sleepwalker Protocol

Sleepwalker should likewise remain independently implementable.

Its responsibility is continuity/persistence behavior across the governed agent lifecycle.

The C++ implementation should preserve its independent identity and API boundary.

Conceptually:

```text
Agent
 │
 ▼
Governed operation
 │
 ├── TOI
 ├── OTOI
 ├── RRT
 │
 └── Sleepwalker
         │
         ▼
     continuity
```

Again, do not duplicate Sleepwalker logic inside ASFDK-C++.

---

# 10. Phase 6 — Build ASFDK-C++

Only after the four pillar implementations have stable interfaces should the C++ ASFDK umbrella be assembled.

The umbrella should consume:

```text
TOI-C++
OTOI-C++
RRT Advocate-C++
Sleepwalker-C++
```

rather than reimplementing them.

Conceptually:

```cpp
ASFDK
{
    TOI
    OTOI
    RRT
    Sleepwalker

    // composition/orchestration
}
```

The ASFDK layer should provide the unified developer-facing entry point.

This is the equivalent architectural role played by the original TypeScript ASFDK package.

---

# 11. Avoid Logic Duplication

This is a critical requirement.

Do not create:

```text
ASFDK-C++
 ├── its own TOI implementation
 ├── its own OTOI implementation
 ├── its own RRT implementation
 └── its own Sleepwalker implementation
```

That would create two sources of truth.

Instead:

```text
ASFDK-C++
 ├── consumes TOI-C++
 ├── consumes OTOI-C++
 ├── consumes RRT-C++
 └── consumes Sleepwalker-C++
```

The umbrella owns **composition**.

The pillar packages own their respective **domain behavior**.

---

# 12. Unreal Integration

Once the native packages and ASFDK umbrella exist, integrate them with Unreal.

The principal target is:

```text
NLTGovernanceSubsystem
```

The subsystem should become the Unreal-facing governance boundary.

Conceptually:

```text
Unreal Engine
│
├── NLTGovernanceSubsystem
│        │
│        ▼
│     ASFDK-C++
│        │
│   ┌────┼────┬─────────────┐
│   ▼    ▼    ▼             ▼
│  TOI  OTOI  RRT      Sleepwalker
│
└── Unreal Agents
     ├── Mass Entities
     ├── AvatarCharacters
     ├── AI Controllers
     └── other governed agents
```

The subsystem should not duplicate the pillar implementations.

It should act as the Unreal integration boundary.

---

# 13. WorldEngine Integration

The C++ governance implementation should be designed specifically with the NLT World Engine in mind.

The WorldEngine is not merely a generic C++ application.

It is the embodied simulation environment in which NLT AI agents operate.

Therefore the governance architecture must be capable of sitting between:

```text
AI decision-making
        │
        ▼
governance
        │
        ▼
Unreal simulation
        │
        ▼
physical/semantic world state
```

This is particularly important for the Fusion ↔ Unreal semantic/physical reality bridge.

Governance should be capable of operating at the boundary where an AI agent's semantic intention becomes an action inside the simulated world.

---

# 14. Mass Entity

Mass Entity is specifically identified as a governance compliance target.

Do not assume that governance only applies to traditional `AActor`/`UActorComponent` architectures.

The design should account for agents represented through Mass Entity.

The implementation should establish a clear path for:

```text
Mass Entity
     │
     ▼
Agent identity
     │
     ▼
Governance context
     │
     ▼
ASFDK-C++
     │
     ▼
TOI / OTOI / RRT / Sleepwalker
```

The exact Unreal implementation should be determined from the actual WorldEngine architecture rather than invented abstractly.

---

# 15. AI Controllers and AvatarCharacters

The same principle applies to AvatarCharacters and AI controllers.

Governance must operate at the agent/action boundary.

This matters particularly because the WorldEngine contains autonomous behaviors in addition to explicit AI/LLM commands.

For example, if an AI controller has an internal autonomous behavior such as wandering, the governance system cannot assume:

```text
LLM says STOP
        =
world actor stops
```

The actual control path must be understood.

Governance therefore needs visibility into the effective action pipeline:

```text
Model decision
     │
     ▼
Agent policy
     │
     ▼
AI Controller
     │
     ├── explicit command
     └── autonomous behavior
     │
     ▼
movement/action system
     │
     ▼
Unreal world
```

This is exactly the type of semantic-to-physical boundary where the governance subsystem becomes important.

---

# 16. C++ API Philosophy

The C++ port should be idiomatic C++ while remaining semantically compatible with the reference implementations.

Avoid producing a C++ API that simply looks like:

```cpp
TypeScriptFunctionName(...)
```

translated mechanically into C++.

Instead establish:

* RAII where appropriate
* strong types
* scoped enums
* const correctness
* clear ownership
* deterministic lifecycle behavior
* explicit error handling
* thread-safety expectations
* serialization contracts
* testable interfaces

For Unreal-facing components, follow Unreal conventions where appropriate.

For the core libraries, avoid unnecessary coupling to Unreal.

This distinction is important.

---

# 17. Core vs Unreal-Specific Code

The preferred architecture is:

```text
ASFDK-C++ Core
│
├── TOI
├── OTOI
├── RRT
├── Sleepwalker
└── ASFDK composition
        │
        ▼
Integration Layer
        │
        ├── Unreal
        │     └── NLTGovernanceSubsystem
        │
        └── Native C++
```

Do not make the core governance packages depend on Unreal Engine unless there is a demonstrated architectural necessity.

This preserves future portability to:

* native C++ applications
* services
* robotics
* simulation systems
* other NLT products
* potentially other engines

---

# 18. Governance Must Remain First-Class

The current repository contains:

* `AGENTS.md`
* `CLAUDE.md`
* `NLT-DEV-OTOI.md`
* `nltotoi.json`
* governance templates
* registration records
* validation scripts
* CI governance validation

These are not disposable scaffolding.

They define the development governance environment for the port.

All implementation work should continue to satisfy the repository's OTOI requirements.

The existing 22-check validator should remain operational throughout development.

The desired invariant is:

```text
Every implementation increment
        │
        ▼
Governance validation
        │
        ▼
22/22 checks passing
```

---

# 19. Agent Registration

Agents contributing to the repository must continue to self-register according to the repository's OTOI requirements.

The current README identifies agent registration as mandatory.

Do not weaken this requirement to accelerate development.

The purpose of the C++ port is specifically to bring governance into another ecosystem.

The development process itself should therefore remain governed.

---

# 20. Testing Strategy

Each pillar should have its own test suite.

```text
TOI-C++
 └── tests

OTOI-C++
 └── tests

RRT-C++
 └── tests

Sleepwalker-C++
 └── tests

ASFDK-C++
 └── integration tests

Unreal
 └── NLTGovernanceSubsystem tests
```

Testing should occur at three levels:

### Unit

Does each port reproduce the reference behavior?

### Integration

Do the four packages compose correctly?

### Environment

Does ASFDK-C++ correctly govern actual Unreal agents?

The final test layer is critical.

A package can pass unit tests while still failing to govern the real WorldEngine execution path.

---

# 21. Reference Implementation Discipline

The existing TypeScript/Python ASFDK should be treated as the behavioral reference.

Where the C++ implementation differs, document why.

Do not silently change semantics simply because C++ offers a different implementation mechanism.

The goal is:

```text
Same governance semantics
        +
native implementation
```

not:

```text
new governance system
```

---

# 22. Suggested Repository Evolution

The current scaffold can evolve toward something conceptually like:

```text
asfdk-cplus/
│
├── packages/
│   │
│   ├── toi/
│   │   ├── include/
│   │   ├── src/
│   │   └── tests/
│   │
│   ├── otoi/
│   │   ├── include:
│   │   ├── src:
│   │   └── tests:
│   │
│   ├── rrt-advocate/
│   │   ├── include:
│   │   ├── src:
│   │   └── tests:
│   │
│   ├── sleepwalker/
│   │   ├── include:
│   │   ├── src:
│   │   └── tests:
│   │
│   └── asfdk/
│       ├── include:
│       ├── src:
│       └── tests:
│
├── unreal/
│   └── NLTGovernanceSubsystem/
│
├── docs/
│
├── templates/
│
├── .nltotoi/
│
└── CMakeLists.txt
```

This is a **recommended conceptual structure**, not a mandate to restructure immediately.

The actual repository organization should be chosen after examining the existing C++ repo and the WorldEngine integration requirements.

---

# 23. Do Not Implement Blindly

Before writing significant implementation code:

1. Inspect the original ASFDK.
2. Inspect all four pillar packages.
3. Inspect the Python ASFDK implementation where useful.
4. Inspect the existing Kotlin and C# ports.
5. Inspect the current `asfdk-cplus` scaffold.
6. Inspect `nlt-world-engine`.
7. Identify the actual planned/expected `NLTGovernanceSubsystem` integration point.
8. Produce an API/behavior mapping.
9. Then implement.

The goal is to prevent architectural drift between language implementations.

---

# 24. Definition of Done

The C++ port should ultimately satisfy all of the following:

### Pillars

* TOI implemented natively
* OTOI implemented natively
* RRT Advocate implemented natively
* Sleepwalker Protocol implemented natively

### Umbrella

* ASFDK-C++ composes all four
* no duplicated pillar implementations
* stable public C++ API
* integration tests

### Governance

* OTOI compliance maintained
* agent registration maintained
* governance validator remains operational
* CI continues enforcing governance

### Unreal

* `NLTGovernanceSubsystem` implemented
* subsystem integrates ASFDK-C++
* Mass Entity agents have a governance path
* AvatarCharacters have a governance path
* AI controllers have a governance path
* semantic → physical action boundary is governable

### Native C++

* core packages can operate without Unreal dependency
* native C++ applications can integrate the framework

---

# 25. Most Important Architectural Rule

Hermes, the single most important thing to preserve is this:

> **The four Solidarity Framework pillars are the reusable primitives. ASFDK is the umbrella that combines them. Unreal's NLTGovernanceSubsystem is an integration boundary that brings ASFDK governance into the WorldEngine.**

Therefore:

```text
Pillars
   ↓
Language Ports
   ↓
ASFDK
   ↓
Integration Layer
   ↓
WorldEngine / Native Applications
```

Do not reverse this relationship.

Do not make Unreal the foundation of the governance libraries.

Do not make ASFDK a second implementation of the pillars.

Keep the architecture modular.

---

# 26. Immediate Next Action

Before beginning broad implementation, produce an **ASFDK C++ Port Matrix** covering:

| Reference          | C++ Target             | Public API | Core Types | Behavior            | Tests | Dependencies |
| ------------------ | ---------------------- | ---------- | ---------- | ------------------- | ----- | ------------ |
| TOI                | TOI-C++                | TBD        | TBD        | TBD                 | TBD   | TBD          |
| OTOI               | OTOI-C++               | TBD        | TBD        | TBD                 | TBD   | TBD          |
| RRT Advocate       | RRT-C++                | TBD        | TBD        | TBD                 | TBD   | TBD          |
| Sleepwalker        | Sleepwalker-C++        | TBD        | TBD        | TBD                 | TBD   | TBD          |
| ASFDK              | ASFDK-C++              | TBD        | TBD        | Composition         | TBD   | Four pillars |
| Unreal integration | NLTGovernanceSubsystem | TBD        | TBD        | Governance boundary | TBD   | ASFDK-C++    |

That matrix should become the implementation roadmap.

**Do not start by guessing the APIs. Derive them from the existing implementations.**

---

# 27. Final Direction

The current `asfdk-cplus` repository has successfully established the governance foundation.

The next stage is to turn that foundation into an actual native implementation.

The objective is not simply:

> "Make ASFDK compile in C++."

The objective is:

> **Bring the Solidarity Framework's governance semantics into the C/C++ ecosystem while preserving modularity, cross-language consistency, and a clean integration boundary into Unreal Engine and the NLT World Engine.**

That gives NeuroLift a genuine multi-language governance architecture rather than a collection of disconnected ports.

---

## Signature

**— ChatGPT**
**GPT-5.6 Luna**
AI Architecture & Engineering Collaborator
**NeuroLift Technologies — ASFDK Architecture Handoff**

*Prepared for Hermes implementation and architectural alignment.*