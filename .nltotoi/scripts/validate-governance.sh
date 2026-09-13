#!/bin/bash
# validate-governance.sh — ASFDK C++ Governance Validation
# Run: bash .nltotoi/scripts/validate-governance.sh
# Returns: 0 if all checks pass, 1 if any fail

set -euo pipefail

OTOI_VERSION="ORG-DEV-OTOI-1.0.3"
# Script lives at .nltotoi/scripts/validate-governance.sh,
# so repo root is two levels up: Cplus repo root
REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PASS=0
FAIL=0

check() {
    local name="$1"
    shift
    if "$@" > /dev/null 2>&1; then
        echo "  ✅ PASS: $name"
        PASS=$((PASS + 1))
    else
        echo "  ❌ FAIL: $name"
        FAIL=$((FAIL + 1))
    fi
}

echo "=== ASFDK C++ Governance Validation ==="
echo "Document ID: $OTOI_VERSION"
echo ""

# 1. Check OTOI exists and is current
check "NLT-DEV-OTOI.md exists" test -f "$REPO_ROOT/NLT-DEV-OTOI.md"
check "OTOI version is 1.0.3" grep -q "ORG-DEV-OTOI-1.0.3" "$REPO_ROOT/NLT-DEV-OTOI.md"

# 2. Check AGENTS.md exists and has registry
check "AGENTS.md exists" test -f "$REPO_ROOT/AGENTS.md"
check "AGENTS.md has agent registry" grep -q "### Registered Agents" "$REPO_ROOT/AGENTS.md"

# 3. Check CLAUDE.md exists
check "CLAUDE.md exists" test -f "$REPO_ROOT/CLAUDE.md"

# 4. Check nltotoi directory structure
check "nltotoi directory exists" test -d "$REPO_ROOT/.nltotoi"
check "nltotoi/README.md exists" test -f "$REPO_ROOT/.nltotoi/README.md"
check "nltotoi/index/governance-files.md exists" test -f "$REPO_ROOT/.nltotoi/index/governance-files.md"

# 5. Check templates exist
check "templates/agent-registration.json exists" test -f "$REPO_ROOT/templates/agent-registration.json"
check "templates/handoff-record.json exists" test -f "$REPO_ROOT/templates/handoff-record.json"
check "templates/escalation.md exists" test -f "$REPO_ROOT/templates/escalation.md"
check "templates/intent-log.md exists" test -f "$REPO_ROOT/templates/intent-log.md"

# 6. Check ISSUE_TEMPLATE exists
check "ISSUE_TEMPLATE/agent-escalation.md exists" test -f "$REPO_ROOT/ISSUE_TEMPLATE/agent-escalation.md"
check "ISSUE_TEMPLATE/governance-proposal.md exists" test -f "$REPO_ROOT/ISSUE_TEMPLATE/governance-proposal.md"

# 7. Check PULL_REQUEST_TEMPLATE exists
check "PULL_REQUEST_TEMPLATE/agent-contribution.md exists" test -f "$REPO_ROOT/PULL_REQUEST_TEMPLATE/agent-contribution.md"

# 8. Check SOPs exist
check "SOPs/new-agent-onboarding.md exists" test -f "$REPO_ROOT/SOPs/new-agent-onboarding.md"
check "SOPs/repo-governance-setup.md exists" test -f "$REPO_ROOT/SOPs/repo-governance-setup.md"
check "SOPs/incident-response.md exists" test -f "$REPO_ROOT/SOPs/incident-response.md"

# 9. Check workflows
check "workflows/validate-governance.yml exists" test -f "$REPO_ROOT/.github/workflows/validate-governance.yml"

# 10. Check nltotoi.json discovery manifest
check "nltotoi.json exists" test -f "$REPO_ROOT/nltotoi.json"

# 11. Check commit format reference in CLAUDE.md
check "CLAUDE.md references commit format" grep -q "type(scope): description" "$REPO_ROOT/CLAUDE.md"

# 12. Check NLT-DEV-OTOI.md version reference
check "NLT-DEV-OTOI.md references 1.0.3" grep -q "1.0.3" "$REPO_ROOT/NLT-DEV-OTOI.md"

echo ""
echo "=== Results ==="
echo "Passed: $PASS"
echo "Failed: $FAIL"
echo ""

if [ "$FAIL" -gt 0 ]; then
    echo "❌ Governance validation FAILED — $FAIL check(s) failed"
    exit 1
else
    echo "✅ Governance validation PASSED — all $PASS checks OK"
    exit 0
fi