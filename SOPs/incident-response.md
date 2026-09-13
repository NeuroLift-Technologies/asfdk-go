# Incident Response

## What to Do When an Agent Goes Off-Rails

### Recognition Signals

An agent may be off-rails if you observe:

| Signal | Description |
|---|---|
| **Scope creep** | Agent begins working outside agreed-upon task boundaries without escalation |
| **Architecture decisions** | Agent making framework/database/deployment decisions without human approval |
| **Provider lock-in** | Agent hardcoding a specific LLM provider or external service |
| **Credential exposure** | Agent committing secrets, tokens, or credentials to code/VCS |
| **Guardrail violations** | Any of the 6 non-negotiable guardrails being violated |
| **OTOI self-amendment** | Agent attempting to change `NLT-DEV-OTOI.md` without formal process |
| **Silent failures** | Agent not reporting errors or continuing after failures |

### Immediate Actions

1. **Stop the agent** — If running in a live session, intervene to stop further work
2. **Escalate to Joshua** — Use the escalation format:
   ```markdown
   ## Escalation Record
   
   **Date:** [ISO 8601]
   **Agent:** [Agent name]
   **Session:** [Session/branch ID]
   **Trigger:** [What caused the escalation]
   
   ### Situation
   [Describe the situation]
   
   ### Decision Required
   [What specific decision or input is needed from Joshua]
   
   ### Options Considered
   1. [Option A] — [trade-offs]
   2. [Option B] — [trade-offs]
   
   ### Recommendation
   [Agent's recommendation, if any]
   
   ### Blockers
   [What cannot proceed until this is resolved]
   ```
3. **Handoff** — If the agent has done work, write a handoff record using `templates/handoff-record.json`
4. **Audit** — Review what the agent did, what went wrong, and how to prevent recurrence

### Prevention

| Prevention Strategy | Implementation |
|---|---|
| **Clear scope definition** | Before starting, confirm task boundaries with human principal |
| **Guardrail education** | Ensure agent understands all 6 non-negotiable guardrails |
| **Regular check-ins** | Session leads should verify scope alignment every 30-60 min |
| **Escalation culture** | Make it clear that escalation is correct protocol, not failure |
| **Validation gates** | Require `validate-governance.sh` passing before PR merge |
| **Branch protection** | Enforce required checks and approvals on all PRs |

### Post-Incident Review

After the incident is resolved:

1. **Document** — What happened, root cause, and resolution
2. **Prevent** — What changes will prevent recurrence (process, documentation, training)
3. **Socialize** — Share lessons learned with other agents
4. **Update** — Revise docs/procedures if needed to close gaps

### Contact Escalation

**Primary:** Joshua W. Dorsey, Sr. — Final authority on all architectural, deployment, and strategic decisions

**Secondary:** Any NLT team lead or senior agent who can assess the situation and determine if Joshua's direct intervention is required

**Emergency:** If the agent has committed credentials, exposed vulnerabilities, or taken actions with security implications — escalate immediately and do not wait for regular channels