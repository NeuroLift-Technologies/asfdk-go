# Repository Governance Setup

## Adding Governance to a New NLT Repository

When creating a new NLT repository (like `asfdk-csharp`), follow these steps to establish minimum required governance artifacts:

### Step 1: Create the Governance Foundation

These files are **mandatory** for every NLT repo:

| File | Purpose | Location |
|---|---|---|
| `NLT-DEV-OTOI.md` | Org-level agent contract (mirror) | Repository root |
| `AGENTS.md` | Agent registry | Repository root |
| `CLAUDE.md` | Project-specific context | Repository root |
| `nltotoi.json` | Discovery manifest | Repository root |
| `.nltotoi/README.md` | Namespace overview | `.nltotoi/` |
| `.nltotoi/index/governance-files.md` | File registry | `.nltotoi/index/` |
| `.nltotoi/agent-registration.json` | Registration schema | `.nltotoi/` |
| `.nltotoi/scripts/validate-governance.sh` | Validation script | `.nltotoi/scripts/` |
| `templates/agent-registration.json` | OTOI Section 3 format | `templates/` |
| `templates/handoff-record.json` | OTOI Section 5 format | `templates/` |
| `templates/escalation.md` | OTOI Section 4.3 format | `templates/` |
| `templates/intent-log.md` | Intent logging | `templates/` |
| `ISSUE_TEMPLATE/agent-escalation.md` | GitHub issue form | `ISSUE_TEMPLATE/` |
| `ISSUE_TEMPLATE/governance-proposal.md` | OTOI amendment form | `ISSUE_TEMPLATE/` |
| `PULL_REQUEST_TEMPLATE/agent-contribution.md` | PR checklist | `PULL_REQUEST_TEMPLATE/` |
| `.github/workflows/validate-governance.yml` | CI validation | `.github/workflows/` |
| `SOPs/new-agent-onboarding.md` | Onboarding procedure | `SOPs/` |
| `SOPs/repo-governance-setup.md` | This procedure | `SOPs/` |
| `SOPs/incident-response.md` | Off-rails response | `SOPs/` |

### Step 2: Configure the Validation Script

Edit `.nltotoi/scripts/validate-governance.sh` to match your repo's specific:
- OTOI version (should match `ORG-DEV-OTOI-1.0.3`)
- File paths specific to your repo structure
- Additional checks relevant to your project type

### Step 3: Set Up Branch Protection

On GitHub, configure your default branch (main/master) with:
- ✅ Require pull request reviews before merging: 1 approving review
- ✅ Dismiss stale pull request approvals when new commits are pushed: true
- ✅ Require status checks to pass before merging: all required checks
- ✅ Require branches to be up to date before merging: true
- ✅ Do not allow bypassing the above settings: true
- ✅ Restrict who can push to matching branches: maintainers only

### Step 4: Add PR Review Hermes Bot

Ensure `.github/workflows/pr-review-hermes.yml` exists with the required configuration:

```yaml
name: PR Review Hermes Bot
on:
  pull_request:
    types: [opened, synchronize, reopened, ready_for_review]

permissions:
  contents: read
  pull-requests: write
  checks: read
  statuses: read

jobs:
  hermes-review:
    name: PR Review Hermes
    runs-on: ubuntu-latest
    steps:
      - name: Check status
        uses: actions/github-script@v7
        with:
          script: |
            // Verify all required checks pass
            // Post review comments
            // Block merge if checks incomplete
```

### Step 5: Test the Setup

Run the validation script:
```bash
bash .nltotoi/scripts/validate-governance.sh
```

Expected output: `✅ Governance validation PASSED — all 25 checks OK`

### Step 6: Onboard New Agents

When a new agent joins:
1. Add their entry to `AGENTS.md`
2. Have them read `NLT-DEV-OTOI.md`, `AGENTS.md`, `CLAUDE.md` in order
3. Confirm task scope before beginning work
4. Have them self-register per OTOI Section 3
5. Verify `validate-governance.sh` passes

### Common Pitfalls to Avoid

| Pitfall | Consequence | Fix |
|---|---|---|
| Missing `NLT-DEV-OTOI.md` mirror | Governance non-compliance | Add the org-level contract mirror |
| Outdated agent registry | Invisible agent activity | Update `AGENTS.md` on every new agent |
| Missing commit format enforcement | Inconsistent commit messages | Ensure `CLAUDE.md` is read and followed |
| Validation script failures | Blocked PRs | Fix failing checks before merging |
| No branch protection | Uncontrolled merges | Configure GitHub branch protection rules |