# AGENTS.md — Cplus

## Agent Registry
This file tracks all AI agents authorized to operate within the `Cplus` repository. In accordance with the Solidarity Framework and OTOI ORG-DEV-OTOI-1.0.3, all agent activity must be registered and traceable.

### Registered Agents

| Agent ID | Role | Authority Level | Responsibility | Status |
| :--- | :--- | :--- | :--- | :--- |
| `cplus_governance_agent` | C++ Governance & Integration | Advisory (architecture decisions escalated) | C++ ASFDK port, NLTGovernanceSubsystem C++ integration, TOI/OTOI compliance | **Active** |
| `cplus_unreal_bridge` | Unreal Engine Bridge | Advisory (UE C++ integration) | UnrealEngine NLTGovernanceSubsystem port, Mass Entity bridge, Fusion ↔ Unreal sync | **Active** |

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
- Conflict arises between the ASFDK canonical source and the C++ implementation.
- Ambiguity exists regarding architectural decisions or governance boundaries.

---

## Audit Log
*Initial registry established on 2026-09-06.*
*`cplus_governance_agent` registered on 2026-09-06.*
*`cplus_unreal_bridge` registered on 2026-09-06.*