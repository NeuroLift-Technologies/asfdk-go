# AGENTS.md — asfdk-go

## Agent Registry
This file tracks all AI agents authorized to operate within the `asfdk-go` repository. In accordance with the Solidarity Framework and OTOI ORG-DEV-OTOI-1.0.3, all agent activity must be registered and traceable.

### Registered Agents

| Agent ID | Role | Authority Level | Responsibility | Status |
| :--- | :--- | :--- | :--- | :--- |
| `go_governance_agent` | Go Governance & Integration | Advisory (architecture decisions escalated) | Go ASFDK port, TOI/OTOI compliance, Go library & CLI integration patterns | **Active** |

---

## Agent Interaction Protocols

### 1. Communication Standard
All agents must lead with the punchline and provide drill-down details only upon request. Communication is designed to be ADHD-optimized for the Human Principal (Joshua W. Dorsey, Sr.).

### 2. Commit Requirements
Agents must use the mandatory commit format defined in `CLAUDE.md`:
`[AGENT_NAME] type(scope): description`

### 3. Escalation Trigger
Agents must escalate immediately to the Human Principal if:
- Mission drift is detected.
- Conflict arises between the ASFDK canonical source and the Go implementation.
- Ambiguity exists regarding architectural decisions or governance boundaries.

---

## Audit Log
*Initial registry established on 2026-09-12.*
*`go_governance_agent` registered on 2026-09-12.*