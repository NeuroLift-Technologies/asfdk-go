---
id: governance-files-index
title: "ASFDK Go Governance File Registry"
author: "NeuroLift Technologies"
date: 2026-09-12
version: 1.0.0
metadata:
  oroi: ORG-DEV-OTOI-1.0.3
---
# Governance File Index

This file registry tracks all governance-related files in the `asfdk-go` repository, ensuring compliance with `ORG-DEV-OTOI-1.0.3`.

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

## Port Deliverables

| Deliverable | Files | Status |
|---|---|---|
| Module definition | `go.mod` | ✅ Complete |
| Shared types & enums | `types.go`, `dto.go` | ✅ Complete |
| TOI/OTOI validation, sanitization, output validation | `promptdefense.go` | ✅ Complete |
| Foundation orchestration (mode/component resolution) | `foundation.go` | ✅ Complete |
| Sleepwalker emotional-state analysis | `sleepwalker.go` | ✅ Complete |
| RRT crisis assessment & response | `rrt.go` | ✅ Complete |
| Test suite (29 tests) | `foundation_test.go` | ✅ Complete |

**Legend:** ✅ = Present and validated, ⬜ = Planned, ❌ = Missing

---

## Agent Registrations

| Agent | Session | Date | Status |
|---|---|---|---|
| go_governance_agent (Cline) | session-asfdk-go-001 (Go port) | 2026-09-12 | ✅ Complete |
| go_governance_agent (Cline) | session-asfdk-go-002 (review round 1) | 2026-09-12 | ✅ Complete |
| go_governance_agent (Cline) | session-asfdk-go-003 (Bugbot/canonical round 2) | 2026-09-12 | ✅ Complete |

*Last updated: 2026-09-13*