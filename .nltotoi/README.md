# Namespace Overview

This repository `asfdk-go` is the Go port of the NeuroLift Technologies ASFDK (Solidarity Framework Development Kit).

**Purpose:** Provide TOI/OTOI/ASFDK governance compliance for Go ecosystems, enabling:
- Agent registration and governance boundary definition
- Terms of Interaction (TOI) enforcement
- Terms of Operation (OTOI) compliance checking
- ASFDK (Solidarity Framework Development Kit) integration points for Go services and CLI tools

**Related repositories:**
- `NeuroLift-Technologies/asfdk` — Original ASFDK (Python/TypeScript reference)
- `NeuroLift-Technologies/asfdk-kotlin` — Kotlin port
- `NeuroLift-Technologies/asfdk-csharp` — C#/.NET port
- `NeuroLift-Technologies/asfdk-cplus` — C++ port
- `NeuroLift-Technologies/nlt-world-engine` — Unreal Engine C++ simulation world
- `NeuroLift-Technologies/neurolift-ai-fusion` — Python intelligence/Avatar-Aide-Advocate layer
- `NeuroLift-Technologies/.github-private` — Org-level governance

**Current status:** Foundation files scaffolded. Go port in progress.

**Key directories:**
- `.nltotoi/` — Discovery manifest and governance file registry
- `templates/` — OTOI Section 3 registration format, handoff records, escalation format, intent logging
- Root package — Go implementation (`go.mod`, `types.go`, `dto.go`, `foundation.go`, `promptdefense.go`, `sleepwalker.go`, `rrt.go`)
- `docs/` — ASFDK integration documentation and agent logs