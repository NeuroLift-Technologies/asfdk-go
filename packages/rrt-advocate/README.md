# RRT Advocate — C++ Port

**Rapid Response Team Advocate** — crisis detection, assessment, and burnout detection for the ASFDK C++ governance framework.

## Overview

This package ports the RRT Advocate crisis-detection engine from the canonical Python/TypeScript implementation in `asfdk/legacy/rrt-advocate/`. It provides:

- **3-Layer Crisis Detection Engine (CDE)**: keyword/semantic field analysis, sentiment analysis, and behavioral pattern analysis
- **Crisis Assessment**: maps aggregated indicators to crisis levels (GREEN → BLACK) with safety scores and recommended interventions
- **Burnout Detection**: monitors session metrics and crisis-level history for burnout indicators
- **CrisisEngine Facade**: unified detection + assessment API

## Architecture

```
RRTAdvocate (facade)
├── CrisisEngine
│   ├── CrisisDetector
│   │   ├── KeywordLayer (Layer 1) — semantic field matching
│   │   ├── SentimentLayer (Layer 2) — polarity + trend analysis
│   │   └── BehavioralLayer (Layer 3) — complexity + looping detection
│   └── CrisisAssessor — confidence → CrisisLevel mapping
└── BurnoutDetector — session metrics + crisis history
```

## Crisis Levels

| Level | Meaning | Trigger |
|-------|---------|---------|
| GREEN | stable | aggregate confidence < 0.20 |
| YELLOW | elevated | 0.20 ≤ confidence < 0.40 |
| ORANGE | high | 0.40 ≤ confidence < 0.70 |
| RED | critical | 0.70 ≤ confidence < 0.90 |
| BLACK | emergency | confidence ≥ 0.90 OR self-harm detected |

## Layer Weights

- Layer 1 (keyword): 0.45
- Layer 2 (sentiment): 0.35
- Layer 3 (behavioral): 0.20

## Building

```bash
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build .
ctest
```

## Usage

```cpp
#include "rrt/RRTAdvocate.h"

rrt::RRTAdvocate advocate("user-123");
auto assessment = advocate.assessMessage("I can't cope with this anymore");

if (assessment.crisisLevel == rrt::CrisisLevel::BLACK) {
    // Emergency escalation required
}
```

## Governance Compliance

- Technology decisions per `docs/DECISIONS.md`
- C++20 standard, Catch2 tests, nlohmann::json, spdlog
- Dual error handling pattern (exceptions + std::expected)
- Commit format: `[AGENT_NAME] type(scope): description`

## Security

⚠️ **PROTOTYPE — NOT A SAFETY SYSTEM.** This is an experimental crisis-detection library. It is not medical advice, not a crisis service, and can miss real crisis signals. In the US, call or text **988** for crisis support.