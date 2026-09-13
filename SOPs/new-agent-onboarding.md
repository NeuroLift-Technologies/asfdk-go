# New Agent Onboarding

## Welcome to NeuroLift Technologies

This guide will walk you through onboarding as a coding agent in any NLT repository, including `asfdk-csharp`.

### Step 1: Read the Governance Docs

Before doing any work, read these documents in order:

1. **`NLT-DEV-OTOI.md`** — Organization-wide coding agent contract (root of this repo)
2. **`AGENTS.md`** — Agent registry and interaction protocols (this repo)
3. **`CLAUDE.md`** — Project-specific context (this repo)
4. **`docs/active-threads.md`** — Current work state (check if present)

### Step 2: Self-Register

Add your agent registration to the registry. Create or update `AGENTS.md` with your entry following the format:

```markdown
### Agent ID [Agent ID]
**Role:** [Your role]
**Authority Level:** [Advisory/Escalation/etc.]
**Responsibility:** [Your responsibilities]
**Status:** Active
```

### Step 3: Confirm Task Scope

Before beginning significant work:
- Confirm your task scope with the human (Joshua W. Dorsey, Sr.)
- Ensure you understand the scope boundaries
- Identify any escalation triggers before they become blockers

### Step 4: Begin Work

When starting a task:
- Update `docs/active-threads.md` if present
- Log your intent for significant decisions (see `templates/intent-log.md`)
- Follow the commit format: `[AGENT_NAME] type(scope): description`
- Escalate immediately if you encounter:
  - Unclear scope or conflicting work
  - Architecture decisions required
  - Blockers you cannot resolve
  - Ethical concerns

### Step 5: Handoff

Before ending any significant session:
1. Update `docs/active-threads.md`
2. Write a handoff record using `templates/handoff-record.json` format
3. Document any open escalations in `docs/escalations/`
4. Summarize decisions made and pending

### Step 6: Validate

After pushing work:
- Run `bash .nltotoi/scripts/validate-governance.sh` to ensure all governance files are in order
- Ensure all checks pass before creating a PR

### Need Help?

- Escalate to Joshua W. Dorsey, Sr. for any architectural, deployment, or strategic decisions
- Ask in the active thread if you're uncertain about scope or boundaries
- Review the `asfdk` original repository for ASFDK pattern reference