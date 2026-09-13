---
id: governance-files-index
title: "ASFDK C++ Governance File Registry"
author: "NeuroLift Technologies"
date: 2026-09-06
version: 1.0.0
metadata:
  oroi: ORG-DEV-OTOI-1.0.3
---
# Governance File Index

This file registry tracks all governance-related files in the `asfdk-cplus` repository, ensuring compliance with `ORG-DEV-OTOI-1.0.3`.

| File Path | Category | Status | Description |
|---|---|---|---|
| `.nltotoi/README.md` | Discovery | ✅ Implemented | Namespace overview and file registry entry point |
| `.nltotoi/agent-registration.json` | Discovery | ✅ Implemented | Agent registration schema (OTOI Section 3) |
| `.nltotoi/index/governance-files.md` | Discovery | ✅ Implemented | This file — governance file registry |
| `.nltotoi/scripts/validate-governance.sh` | Governance | ✅ Implemented | Validation script (22 checks) |
| `AGENTS.md` | Governance | ✅ Implemented | Agent registry with roles and authority |
| `CLAUDE.md` | Governance | ✅ Implemented | Agent collaboration protocols and commit format |
| `NLT-DEV-OTOI.md` | Governance | ✅ Implemented | Org-level coding agent contract (mirror) |
| `templates/agent-registration.json` | Templates | ✅ Implemented | OTOI Section 3 registration format |
| `templates/handoff-record.json` | Templates | ✅ Implemented | OTOI Section 5 handoff format |
| `templates/escalation.md` | Templates | ✅ Implemented | OTOI Section 4.3 escalation format |
| `templates/intent-log.md` | Templates | ✅ Implemented | Intent logging template |
| `ISSUE_TEMPLATE/agent-escalation.md` | Issues | ✅ Implemented | GitHub escalation issue form |
| `ISSUE_TEMPLATE/governance-proposal.md` | Issues | ✅ Implemented | OTOI amendment proposal form |
| `PULL_REQUEST_TEMPLATE/agent-contribution.md` | PR | ✅ Implemented | Agent PR checklist |
| `.github/workflows/validate-governance.yml` | CI | ✅ Implemented | CI workflow for governance validation |
| `SOPs/new-agent-onboarding.md` | Procedures | ✅ Implemented | New agent onboarding procedure |
| `SOPs/repo-governance-setup.md` | Procedures | ✅ Implemented | How to add governance to a new NLT repo |
| `SOPs/incident-response.md` | Procedures | ✅ Implemented | What to do when an agent goes off-rails |

---

## Package Deliverables

| Package | Phase | Files | Status |
|---|---|---|---|
| `packages/toi/` | Phase 2 | `TOITypes.h`, `TermsOfInteraction.h/.cpp`, `TOIManager.cpp`, `test_toi_manager.cpp`, `CMakeLists.txt`, `README.md` | ✅ Complete |
| `packages/otoi/` | Phase 3 | `OTOITypes.h`, `OTOIManager.h/.cpp`, `test_otoi_manager.cpp`, `CMakeLists.txt`, `README.md` | ✅ Complete |
| `packages/sleepwalker/` | Phase 5 | `SleepwalkerTypes.h`, `StateDetector.h/.cpp`, `ConsentManager.h/.cpp`, `ContinuityManager.h/.cpp`, `SleepwalkerProtocol.h/.cpp`, `test_state_detection.cpp`, `test_consent.cpp`, `test_continuity.cpp`, `test_sleepwalker.cpp`, `CMakeLists.txt`, `README.md` | ✅ Complete |
| `packages/rrt-advocate/` | Phase 4 | `RRTTypes.h/.cpp`, `KeywordLayer.h/.cpp`, `SentimentLayer.h/.cpp`, `BehavioralLayer.h/.cpp`, `CrisisDetector.h/.cpp`, `CrisisAssessor.h/.cpp`, `CrisisEngine.h/.cpp`, `BurnoutDetector.h/.cpp`, `RRTAdvocate.h/.cpp`, `test_rrt_advocate.cpp`, `test_crisis.cpp`, `test_burnout.cpp`, `CMakeLists.txt`, `README.md` | ✅ Complete |
| `packages/asfdk/` | Phase 6 | — | ⬜ Pending |
| `unreal/NLTGovernanceSubsystem/` | Phase 7 | — | ⬜ Pending |

**Legend:** ✅ = Present and validated, ⬜ = Planned, ❌ = Missing

---

## Agent Registrations

| Agent | Session | Date | Status |
|---|---|---|---|
| pool / desktop (Codex CLI) | phase5-sleepwalker-cplus | 2026-09-06 | ✅ Active |

*Last updated: 2026-09-06*