# packages/asfdk — ASFDK-C++ Umbrella (Phase 6)

> One library, four pillars, zero duplication.

`asfdk-cplus` is the composition layer that joins the four pillar packages into
a single, coherent ASFDK instance. It **delegates** to the pillars — it does
not re-implement any pillar behaviour.

| Pillar | Package | Delegated APIs |
| :--- | :--- | :--- |
| TOI | `packages/toi` | `parseTOI`, `safeParseTOI`, `resolveTOI`, `validateTOI` |
| OTOI | `packages/otoi` | `parseCharter`, `safeParseCharter`, `honor`, `propagate` |
| RRT Advocate | `packages/rrt-advocate` | `assessMessage`, `assessBurnout`, `resetSession` |
| Sleepwalker | `packages/sleepwalker` | `detectEmotionalState`, `assessInteraction`, `generateResponse`, `maintainContinuity` |

## What the umbrella adds

The only new logic in this package lives **between** the pillars, where
multi-pillar coordination is required by the Solidarity Framework:

1. **D4 provenance envelope** (`process`) — normalises the interaction channel
   and marks the envelope `trusted` **only** for `user_input`. No synthetic or
   tool channel is ever treated as the user.
2. **Flagging** (`ProcessedInteraction`) — combines Sleepwalker protective
   state, check-in requirements, and crisis indicators into a single
   `flagged` + `flag_reason` signal (red-team-friendly, JSON-serialisable).
3. **RRT handoff** (`assess`) — when Sleepwalker detects a state that
   `requiresRrtaHandoff`, the umbrella invokes the RRT Advocate for the
   crisis assessment and surfaces it as one `AssessmentResult`.
4. **Composite status** (`getStatus`) — folds TOI (stateless, always active),
   OTOI mode, RRT monitoring state, and Sleepwalker activeness into one
   `FoundationStatus` with an `overall` health string (`operational` /
   `degraded`).

## Layout

```
packages/asfdk/
├── include/asfdk/
│   ├── ASFDK.h          # ASFDK class — all pillar surfaces + unified surface
│   └── ASFDKTypes.h     # ProcessedInteraction, AssessmentResult, FoundationStatus, ASFDKError
├── src/
│   └── ASFDK.cpp        # delegation wiring + integration-layer logic
├── tests/
│   └── test_asfdk_composition.cpp
├── CMakeLists.txt
└── README.md
```

## Build

`asfdk-cplus` is built as part of the top-level project (repo root
`CMakeLists.txt`), which aggregates all five packages:

```sh
cmake -B build
cmake --build build
ctest --test-dir build
```

## Phase 7 install contract

The top-level `CMakeLists.txt` installs the library and headers into
`include/` + `lib/`:

```sh
cmake --install build --prefix <install-root>
```

That tree is what `Plugins/NLTGovernanceSubsystem/ThirdParty/ASFDK/`
consumes in the Unreal Engine integration (Phase 7).

## Governance

- OTOI: `ORG-DEV-OTOI-1.0.3`
- Scope: `docs/PLAN.md` §8 (Phase 6)
- Design handoff: `docs/ASFDK-C++-HANDOFF.md` §§22–25