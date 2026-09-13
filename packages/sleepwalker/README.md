# Sleepwalker Protocol C++ Package

**ASFDK Sleepwalker Protocol C++ Port**

## Overview

This package provides a C++17/20 port of the Sleepwalker Protocol (SWP), the
emotional-continuity governance layer from the original ASFDK reference
implementations (Python `sleepwalker_protocol` and TypeScript
`@neurolift-technologies/sleepwalker-protocol`).

Sleepwalker monitors for protective psychological states — dissociation,
emotional numbing, avoidance, and detachment — without intervening, and
coordinates graduated consent for AI responses. It also maintains temporal
continuity of user boundaries and protective states across sessions.

**Governed by:** ORG-DEV-OTOI-1.0.3  
**Reference analysis:** `docs/phase-1-reference-analysis.md` §5

## Structure

```
packages/sleepwalker/
├── include/sleepwalker/        — Header files
│   ├── SleepwalkerTypes.h       — Core types (enums, structs, conversions)
│   ├── StateDetector.h          — Emotional state detection (regex)
│   ├── ConsentManager.h         — Graduated consent model
│   ├── ContinuityManager.h      — Cross-session state persistence
│   └── SleepwalkerProtocol.h    — Main orchestrator
├── src/                        — Implementations
│   ├── StateDetector.cpp
│   ├── ConsentManager.cpp
│   ├── ContinuityManager.cpp
│   └── SleepwalkerProtocol.cpp
├── tests/                      — Catch2 v3 unit tests
│   ├── test_state_detection.cpp
│   ├── test_consent.cpp
│   ├── test_continuity.cpp
│   └── test_sleepwalker.cpp     (CATCH_CONFIG_MAIN entry point)
├── CMakeLists.txt              — Build configuration
└── README.md                   — This file
```

## Core Types

### StateType

Enum classifying the detected emotional state:

| Value | Meaning |
|-------|---------|
| `Dissociation` | Dissociative indicators detected |
| `Numbing` | Emotional numbing detected |
| `Avoidance` | Avoidance patterns detected |
| `Detachment` | Protective detachment cues detected |
| `Neutral` | No protective state detected |

### ConsentLevel

Graduated consent levels for AI intervention:

| Value | Meaning |
|-------|---------|
| `Passive` | Passive availability, no prompting |
| `LowPressure` | Low-pressure offer of support |
| `SafetyCheck` | Direct safety check (risk indicators present) |
| `RRTAHandoff` | Crisis intervention handoff to Rapid Response Team |

### Channel

Determines trust level of interaction provenance:

| Value | String | Trusted? |
|-------|--------|----------|
| `UserInput` | `user_input` | ✅ Yes |
| `ModelOutput` | `model_output` | ❌ No |
| `ToolResult` | `tool_result` | ❌ No |
| `System` | `system` | ❌ No |
| `Unknown` | `unknown` | ❌ No |

Only `UserInput` is trusted; all other channels collapse to `Unknown`.

### EmotionalState

Result of state detection, containing:

- `stateType` — classified `StateType`
- `protective` — whether a protective psychological state is active
- `requiresCheckIn` — whether a safety check-in is warranted
- `indicators` — nested indicator flags (`dissociation`, `numbing`, `avoidance`, `detachment`, `crisis`)
- `confidence` — confidence score (0.0 to 1.0)
- Crisis flags: `explicitSuicidalIdeation`, `selfHarmIndicators`, `inabilityToEnsureSafety`

## API Surface

| Component | Methods |
|-----------|---------|
| `StateDetector` | `detect(userInput, sessionHistory)` → `EmotionalState` |
| `ConsentManager` | `determineLevel(state)`, `getAppropriateLevel(state)`, `shouldIntervene(state)`, `getConsentMessage(level)` |
| `ContinuityManager` | `saveSession(userId, sessionData)`, `getContext(userId)`, `retrieveLastSessionState(userId)`, `updateBoundary(userId, type, value)` |
| `SleepwalkerProtocol` | `detectEmotionalState()`, `assessInteraction()`, `generateResponse()`, `determineAppropriateLevel()`, `requiresRrtaHandoff()`, `getContext()`, `maintainContinuity()`, `getStatus()`, `reset()` |

## Usage

```cpp
#include <sleepwalker/SleepwalkerProtocol.h>
#include <nlohmann/json.hpp>

using namespace sleepwalker;

// Configure with TOI preferences
nlohmann::json toi = R"({
    "swp": {
        "active": true,
        "intervention_threshold": "user_initiated_only"
    }
})"_json;

SleepwalkerProtocol::Options opts;
opts.userToi = toi;
opts.loggingEnabled = true;

SleepwalkerProtocol swp(opts);

// Detect emotional state
EmotionalState state = swp.detectEmotionalState(
    "I don't feel safe right now"
);

// Check if crisis handoff is needed
if (swp.requiresRrtaHandoff(state)) {
    // Route to crisis support
}

// Get response guidance
ResponseGuidance guidance = swp.generateResponse(
    "I don't feel safe right now"
);
```

## Detection Logic

### Protective States

The `StateDetector` uses regex pattern matching to identify four protective
psychological states:

1. **Dissociation** — numbness, disconnectedness, derealization, depersonalization
2. **Emotional Numbing** — inability to feel, emotionally flat, shutdown
3. **Avoidance** — unwillingness to discuss topics, deferring engagement
4. **Detachment** — "I'm fine", "it doesn't matter", "whatever"

Multiple detected indicators increase confidence (0.5 for single, 0.8 for
multiple).

### Crisis Indicators

Three crisis categories trigger safety check-ins:

1. **Suicidal Ideation** — suicide, kill myself, better off dead
2. **Self-Harm** — cutting, hurting self, self-harm
3. **Safety Concern** — not safe, lose control, can't keep safe

### RRTA Handoff Condition

`requiresRrtaHandoff()` returns `true` when any crisis indicator is present:
- `explicitSuicidalIdeation == true`
- `selfHarmIndicators == true`
- `inabilityToEnsureSafety == true`

## Continuity Management

The `ContinuityManager` persists session data as JSON files per user,
preserving:

- Emotional state history across sessions
- Declared boundaries (protected topics)
- Session count and timestamps
- Days since last session

## Build

```bash
# From repository root
mkdir -p build && cd build
cmake ../packages/sleepwalker
cmake --build .
# Run tests: ./test_sleepwalker
```

## Testing

Unit tests cover (porting from the Python reference test suite):

- State detection: dissociation, numbing, avoidance, detachment, crisis
- Neutral text: no false positives
- Confidence calculation: 0.0, 0.5, 0.8 scales
- Consent levels: PASSIVE, LOW_PRESSURE, SAFETY_CHECK, RRTA_HANDOFF
- Consent messages for all levels
- Continuity: save/retrieve, multiple sessions, boundary persistence, corrupt file handling
- SleepwalkerProtocol: initialization, detection, assessment, response generation
- Channel normalization: known values pass through, unknown collapses

## Integration Architecture

Per `docs/phase-1-reference-analysis.md` §6, the Sleepwalker Protocol is
activated in the `CONTINUITY_ONLY`, `UNIFIED`, and `DEVELOPMENT` foundation
modes. It integrates with:

- **RRT Advocate** — via `requiresRrtaHandoff()` when crisis indicators are present
- **TOI/OTOI Framework** — via SWP config in user TOI (`swp` key)

## Migration from Reference

The original Sleepwalker Protocol is implemented in Python
(`sleepwalker_protocol/`) and TypeScript (`legacy/sleepwalker/src/`). The C++
port maps:

| Python/TypeScript | C++ |
|---|---|
| `EmotionalState` dataclass/interface | `struct` with scoped fields |
| `ConsentLevel` enum | `enum class ConsentLevel` |
| `Channel` enum | `enum class Channel` + `normalizeChannel()` |
| `re.search(pattern, text_lower)` | `std::regex_search(text, std::regex(pattern, icase))` |
| `json` dict | `nlohmann::json` |
| `Path.mkdir(exist_ok=True)` | `std::filesystem::create_directories` |
| `datetime.now().isoformat()` | `std::chrono::system_clock` + `std::put_time` |

## Governance

- All commits follow `[AGENT_NAME] type(scope): description`
- `validate-governance.sh` must pass (22/22 checks)
- No LLM provider lock-in
- No architecture decisions without human approval
- No production deployments without sign-off
- No credential storage in code or VCS
- Input sanitization is a foundation-layer responsibility (not in this package)

> **⚠️ Important:** This package performs *detection only* — it does not provide
> crisis intervention. Crisis detection relies on pattern matching and may miss
> real signals. Never rely on it as the sole safety mechanism. Crisis situations
> should always be escalated to human review.
