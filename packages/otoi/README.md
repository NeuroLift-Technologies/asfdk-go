# OTOI-C++ Package

**Package:** `otoi-cplus`
**Version:** 1.0.0
**Governed by:** ORG-DEV-OTOI-1.0.3

## Overview

C++ port of the OTOI (Orchestrated Terms of Interaction) enforcement/governance layer. OTOI declares how a mesh of agents honors a stack of `.toi` documents at runtime.

## Dependencies

- C++20
- nlohmann::json (JSON parsing/manipulation)
- tl::expected (std::expected backport for C++17/20)
- spdlog (logging)
- Catch2 v3 (testing)

## Structure

```
packages/otoi/
├── include/otoi/
│   ├── OTOITypes.h      # Core types: enums, structs, error types
│   └── OTOIManager.h    # Public API: validator, manager, governance context
├── src/
│   └── OTOIManager.cpp  # Implementation
├── tests/
│   └── test_otoi_manager.cpp  # Catch2 unit tests
├── CMakeLists.txt
└── README.md
```

## API Surface

### OTOIManager

```cpp
// Parse and validate an .otoi charter (throwing)
OtoiCharter parseCharter(const nlohmann::json& json);

// Non-throwing variant
std::expected<OtoiCharter, OtoiError> safeParseCharter(const nlohmann::json& json) const;

// Fold charter + sources into effective policy (throwing)
EffectivePolicy honor(const OtoiCharter& charter, const HonorOptions& options = {});

// Non-throwing variant
std::expected<EffectivePolicy, OtoiHonorError> safeHonor(const OtoiCharter& charter, const HonorOptions& options = {}) const;

// Get effective preferences for specific agent
nlohmann::json propagate(const EffectivePolicy& policy, const std::string& agentId) const;

// Component status
struct Status { bool active; EnforcementMode mode; };
Status getStatus() const;
```

### OTOIValidator

```cpp
// Validate parsed charter
std::expected<void, OtoiValidationError> validate(const OtoiCharter& charter) const;

// Parse and validate in one step
std::expected<OtoiCharter, OtoiError> parseAndValidate(const nlohmann::json& json) const;

// Detect same-tier conflicts
std::vector<PolicyConflict> detectConflicts(const std::vector<nlohmann::json>& documents) const;
```

## Key Types

- `OtoiCharter` - Full charter document with reserved keys + content
- `OtoiAgent` - Mesh participant
- `OtoiSource` - Reference to a .toi document (uri or inline)
- `OtoiEnforcement` - Raw enforcement settings
- `ResolvedEnforcement` - Enforcement with defaults resolved
- `EffectivePolicy` - Synthesized result for mesh
- `PolicyConflict` - Same-tier disagreement
- `OtoiError`, `OtoiParseError`, `OtoiValidationError`, `OtoiHonorError` - Error hierarchy

## Governance Compliance

- All commits follow `[AGENT_NAME] type(scope): description`
- No credentials or secrets exposed
- No external integrations without approval
- Scope declaration satisfied (no new top-level directories)
