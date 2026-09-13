# Phase 1 — Discovery & Reference Analysis

**Repository:** `NeuroLift-Technologies/asfdk-cplus`
**Created:** 2026-09-06
**Governed by:** ORG-DEV-OTOI-1.0.3
**Status:** Draft (Phase 1 output)

---

## 1. Overview

This document captures the analysis of the original ASFDK reference packages (TypeScript/JavaScript/Python) and maps their APIs, data structures, and behavior contracts to C++ idioms. The goal is to produce a complete reference for the C++ porting work in Phases 2–7.

The original ASFDK consists of four pillars:

| Pillar | NPM/Repo Package | Language | Primary Location |
|--------|------------------|----------|------------------|
| TOI | `@neurolift-technologies/toi` | TypeScript | `packages/asfdk/node_modules/@neurolift-technologies/toi/` |
| OTOI | `@neurolift-technologies/otoi` | TypeScript | `packages/asfdk/node_modules/@neurolift-technologies/otoi/` |
| RRT Advocate | `rrt-advocate` | Python | `legacy/rrt-advocate/` |
| Sleepwalker | `sleepwalker-protocol` | Python/TS | `legacy/sleepwalker/` |

---

## 2. TOI (Terms of Interaction) — Reference Analysis

### 2.1 Specification

- **Version:** 1.0.0 (stable)
- **File extension:** `.toi`
- **Media type:** `application/toi+json`
- **Normative spec:** `SPEC.md` at `node_modules/@neurolift-technologies/toi/SPEC.md`
- **Schema:** Zod schema (`src/schema.ts`), JSON Schema artifact at `schema/toi-1.0.0.schema.json`

### 2.2 Document Model

A `.toi` document is a JSON object with two groups of keys:

| Group | Keys | Description |
|-------|------|-------------|
| **Reserved namespace** | keys beginning with `$` | Document metadata: `$toi`, `$tier`, `$created`, `$updated`, `$id`, `$license`, `$signature` |
| **Content sections** | all other keys | Interaction preferences (open/optional sections) |

### 2.3 Reserved Keys (`$`)

| Key | Required | Type | Rule |
|-----|----------|------|------|
| `$toi` | **Yes** | string | Format version, MUST be `"1.0.0"` |
| `$tier` | **Yes** | string | One of `personal`, `community`, `project` |
| `$created` | No | string | ISO 8601 date/datetime |
| `$updated` | No | string | ISO 8601 date/datetime |
| `$id` | No | string | UUID v4 |
| `$license` | No | string | SPDX license identifier |
| `$signature` | No | object | Ed25519 signature envelope |

### 2.4 Content Sections (all optional)

| Section | Fields | Description |
|---------|--------|-------------|
| `identity` (required) | `author` (required), `handle`, `organization`, `pronouns` | Who authored the preferences |
| `cognitive_profile` | `self_described`, `processing_style`, `attention_model`, `scaffolding_preference`, `energy_model`, `thread_support`, `hyperfocus_protection`, `executive_function_support` | Cognitive and working-style preferences |
| `privacy` | `retention`, `cross_platform_sharing`, `training_use`, `analytics`, `override_rights`, `data_requests` | Data handling preferences |
| `agency` | `task_initiation`, `ai_suggestions`, `interruptibility`, `action_confirmation`, `override_authority` | Agency/autonomy preferences |
| `communication` | `tone`, `verbosity`, `structure`, `language`, `jargon_tolerance`, `pattern_highlighting`, `summary_on_return`, `thread_reconnection` | Communication style preferences |
| `ethical_pillars` | array of strings | Principles the author asks consumers to uphold |
| `custom` | object | Author-defined data (only sanctioned location for non-schema content) |

### 2.5 Tier Precedence

```text
personal > community > project > platform defaults
```

- `personal`-tier document is **terminal**: any field it specifies MUST NOT be overridden by a lower tier.
- Resolution is **gap-filling**: lower tiers supply values for fields not specified by higher tiers.
- Arrays and scalar values are **atomic leaves**: higher tier replaces wholesale.
- Objects are merged **per key**, recursively, under the gap-filling rule.

### 2.6 Canonicalization & Signing

- **JCS (JSON Canonicalization Scheme, RFC 8785):** object keys sorted by UTF-16 code units recursively, insignificant whitespace removed, strings use JSON minimal escaping, numbers use ECMAScript `Number`-to-string form.
- Signing: remove `$signature`, canonicalize remaining doc, encode as UTF-8, compute Ed25519 signature, set `$signature` envelope.
- Verification: decode public_key/value from base64url, remove `$signature`, canonicalize, encode, verify signature.

### 2.7 Reference Implementation API

| Function | Signature | Description |
|----------|-----------|-------------|
| `safeParseToi(candidate)` | `(candidate: unknown) => { success: boolean; data?: ToiDocument; error?: ToiValidationError }` | Non-throwing TOI validation/parsing |
| `validateTOI(candidate)` | `(candidate: unknown) => TOIValidationResult` | Returns `{ valid, errors?, toi? }` |
| `TOIDocumentGenerator.fromDefaults(author)` | `(author: string) => { document: ToiDocument }` | Generates minimal personal-tier TOI from defaults |
| `TOIDocumentGenerator.fromDict(obj)` | `(obj: Record<string, unknown>) => { document: ToiDocument }` | Generates TOI from partial/full document object |
| `resolveToi(doc1, doc2, ...)` | — | Resolves stack of TOIs into effective document using tier precedence |
| `canonicalize(doc)` | — | Returns canonical JSON string (RFC 8785) |
| `canonicalizeToBytes(doc)` | — | Returns canonical byte array |
| `verifyToi(doc, publicKey)` | — | Verifies Ed25519 signature |

### 2.8 TOI Types (from Zod schema)

Key types documented in SPEC.md Sections 7.1–7.7:

- `identity`: `{ author: string; handle?: string; organization?: string; pronouns?: string }`
- `cognitive_profile`: `{ self_described: string; processing_style: enum; attention_model: enum; scaffolding_preference: enum; energy_model: enum; thread_support: boolean; hyperfocus_protection: boolean; executive_function_support: boolean }`
- `privacy`: `{ retention: enum; cross_platform_sharing: enum; training_use: enum; analytics: enum; override_rights: enum; data_requests: enum }`
- `agency`: `{ task_initiation: enum; ai_suggestions: enum; interruptibility: enum; action_confirmation: enum; override_authority: enum }`
- `communication`: `{ tone: enum; verbosity: enum; structure: enum; language: string; jargon_tolerance: enum; pattern_highlighting: boolean; summary_on_return: boolean; thread_reconnection: enum }`
- `enum values`:
  - `processing_style`: `sequential | parallel | associative | variable`
  - `attention_model`: `sustained | short-bursts | hyperfocus-prone | variable`
  - `scaffolding_preference`: `minimal | moderate | extensive | step-by-step`
  - `energy_model`: `steady | variable | spoon-limited | burst`
  - `retention`: `session-only | short-term | long-term | permanent | user-controlled`
  - `cross_platform_sharing`: `never | explicit-only | aggregate-only | research-approved`
  - `training_use`: `prohibited | explicit-only | anonymized-only | permitted`
  - `analytics`: `prohibited | opt-in | anonymized-only | permitted`
  - `override_rights`: `user-only | delegated | admin-allowed`
  - `data_requests`: `honored-immediately | honored-on-request | not-supported`
  - `task_initiation`: `user-initiated | ai-may-suggest | ai-may-initiate`
  - `ai_suggestions`: `none | on-request | proactive`
  - `interruptibility`: `never | urgent-only | always`
  - `action_confirmation`: `always | destructive-only | never`
  - `override_authority`: `user-final | shared | ai-advisory`
  - `tone`: `formal | casual | professional | friendly | direct | adaptive`
  - `verbosity`: `minimal | concise | detailed | comprehensive | adaptive`
  - `structure`: `linear | hierarchical | visual | bullet-points | narrative`
  - `language`: string (BCP 47 tag such as `en`)
  - `jargon_tolerance`: `none | low | moderate | high`
  - `pattern_highlighting`: boolean
  - `summary_on_return`: boolean
  - `thread_reconnection`: `none | brief-summary | full-context`
  - `ethical_pillars`: `string[]` (free-form)

---

## 3. OTOI (Operations Terms of Interaction) — Reference Analysis

### 3.1 Specification

- **Version:** 1.0.0 (draft)
- **File extension:** `.otoi`
- **Media type:** `application/otoi+json`
- **Builds on:** `.toi` v1.0.0
- **Normative spec:** `SPEC.md` at `node_modules/@neurolift-technologies/otoi/SPEC.md`

### 3.2 Document Model

A `.otoi` charter is a JSON object with reserved `$` keys and content keys:

| Key | Required | Type | Rule |
|-----|----------|------|------|
| `$otoi` | **Yes** | string | Format version, MUST be `"1.0.0"` |
| `$id` | No | string | UUID v4 |
| `$created` / `$updated` | No | string | ISO 8601 date/datetime |
| `identity` | No | object | `author` required if present |
| `agents` | No | array | Mesh: objects with required `id`, optional `role`, `modalities`, `affordances` |
| `enforcement` | No | object | Enforcement policy (Section 5) |
| `toi_sources` | No | array | `.toi` documents in force (Section 4) |

### 3.3 `toi_sources` Entry

Each source references exactly one `.toi` document and declares the `$tier` it contributes at:

| Field | Required | Rule |
|-------|----------|------|
| `tier` | **Yes** | One of `personal`, `community`, `project` |
| `uri` | one of | A locator the host resolves to `.toi` text |
| `inline` | one of | A `.toi` document embedded exactly one of `uri` or `inline` (never both) |

The loaded document's own `$tier` MUST equal the source's declared `tier`; a mismatch is rejected.

### 3.4 Enforcement Policy

| Field | Values | Default | Meaning |
|-------|--------|---------|---------|
| `mode` | `advisory`, `enforced`, `strict` | `enforced` | Strictness of honoring. `strict` additionally refuses to serve the policy to an agent not declared in `agents`. |
| `on_conflict` | `highest-tier-wins`, `reject`, `escalate` | `highest-tier-wins` | Disposition of a **same-tier** disagreement (Section 6). |
| `on_unsupported` | `ignore`, `degrade`, `reject` | `degrade` | Disposition of a preference no agent can satisfy. |
| `audit` | boolean | `true` | Whether honoring is recorded to an audit trail. |

### 3.5 Resolution & Conflicts

- Cross-tier disagreement is **not** a conflict — resolved by `.toi` precedence (`personal > community > project > platform defaults`).
- A **conflict** is a disagreement at the **same** tier: two documents assigning different values to the same leaf path.
- Under `on_conflict: "reject"` — hard failure.
- Under `on_conflict: "highest-tier-wins"` — document order within the tier settles it.
- Under `on_conflict: "escalate"` — surfaces the conflict for a human decision.

### 3.6 Reference Implementation API

| Function | Signature | Description |
|----------|-----------|-------------|
| `parseCharter(candidate)` | `(candidate: unknown) => OtoiCharter` | Non-throwing charter parsing |
| `validateCharter(candidate)` | `(candidate: unknown) => OTOIValidationResult` | Returns `{ valid, errors?, charter? }` |
| `getStatus()` | `() => { active: boolean; mode: string }` | Returns active TOI-OTOI component status |

### 3.7 OTOI Types (from SPEC.md)

Key types:

- `agents` array entry: `{ id: string; role?: string; modalities?: string[]; affordances?: string[] }`
- `enforcement`: `{ mode: enum; on_conflict: enum; on_unsupported: enum; audit: boolean }`
- `enum values`:
  - `mode`: `advisory | enforced | strict`
  - `on_conflict`: `highest-tier-wins | reject | escalate`
  - `on_unsupported`: `ignore | degrade | reject`

---

## 4. RRT Advocate (Rapid Response Team Advocate) — Reference Analysis

### 4.1 Package Location

- **Original:** `asfdk/legacy/rrt-advocate/`
- **Language:** Python (primary), with TypeScript wrapper under `packages/asfdk/node_modules/@neurolift-technologies/rrt-advocate/`
- **Status:** Experimental, stubbed intervention layers

### 4.2 Primary API (Python `rrt_advocate.src`)

| Function | Signature | Description |
|----------|-----------|-------------|
| `CrisisEngine(userId).assess(input)` | `-> CrisisAssessment` | Runs 3-layer crisis-detection engine on free-text input |
| `assess(userId, input, channel)` | `-> CrisisAssessment & {channel, trusted, flagged?, flagReason?}` | C++ adapter signature (see below) |
| `resetSession(userId)` | `-> None` | Re-baselines per-user crisis-detection engine |
| `reset(userId?)` | `-> None` | Clears all cached engines (omit to clear all, specify userId for single user) |

### 4.3 `CrisisAssessment` Type

| Field | Type | Description |
|-------|------|-------------|
| `timestamp` | `datetime` | Assessment timestamp |
| `crisis_level` | `CrisisLevel` | One of: `GREEN`, `YELLOW`, `ORANGE`, `RED`, `BLACK` |
| `primary_indicators` | `list[str]` | Primary crisis indicators |
| `secondary_indicators` | `list[str]` | Secondary crisis indicators |
| `confidence_score` | `float` | Aggregate confidence score (0.0 to 1.0) |
| `estimated_duration` | `timedelta \| None` | Estimated crisis duration |
| `recommended_interventions` | `list[str]` | Recommended intervention actions |
| `escalation_threshold` | `float` | Threshold for escalation |
| `user_safety_score` | `float` | User safety score (0.0 = unsafe, 1.0 = fully safe) |
| `context_factors` | `dict[str, Any]` | Additional context (self_harm_risk, sentiment_trend, looping_detected, etc.) |

**Note:** `channel`, `trusted`, `flagged`, `flagReason` are NOT part of `CrisisAssessment`. They are added by the foundation integration layer (`foundation.ts`) as provenance envelope fields.

### 4.4 `CrisisLevel` Enum

| Value | Meaning |
|-------|---------|
| `GREEN` | No crisis detected |
| `YELLOW` | Possible concern, mild indicators |
| `ORANGE` | High concern, moderate indicators |
| `RED` | Clear crisis signal |
| `BLACK` | Emergency/critical crisis |

### 4.5 Emotional State (from Sleepwalker integration)

The RRT engine receives emotional state from Sleepwalker protocol. Key flags:

| Field | Meaning |
|-------|---------|
| `explicitSuicidalIdeation` | boolean — explicit statement of suicidal intent |
| `selfHarmIndicators` | boolean — self-harm signals detected |
| `inabilityToEnsureSafety` | boolean — user cannot ensure own safety |

### 4.6 RRT Advocate Integration Flow (from `foundation.ts`)

```text
EMOTIONAL_ASSESSMENT interaction → Sleepwalker.detectEmotionalState()
   → if requiresRrtaHandoff(state) then
       → RRTAdvocate.assess(userId, input, channel)
       → crisisLevel determines gateUp = true/false
```

### 4.6 Security Warning (from `packages/asfdk/src/integration/rrt.ts`)

```text
⚠️ PROTOTYPE — NOT A SAFETY SYSTEM.
This adapter wraps an experimental crisis-detection library with stubbed
intervention layers. It is not medical advice, not a crisis service, performs
no real-time monitoring, and can miss real crisis signals.
Never rely on it as the sole safety mechanism.
```

---

## 5. Sleepwalker Protocol — Reference Analysis

### 5.1 Package Location

- **Original:** `/home/joshd/Desktop/nlt-repos/asfdk/legacy/sleepwalker/`
- **Dual implementation:** Python (`sleepwalker/src/`) and TypeScript (`packages/asfdk/node_modules/@neurolift-technologies/sleepwalker-protocol/`)
- **Status:** Active in production foundation

### 5.2 Primary API (TypeScript `sleepwalker.ts`)

| Function | Signature | Description |
|----------|-----------|-------------|
| `detectEmotionalState(userInput, sessionHistory, channel, userId)` | `-> EmotionalState & {channel, trusted, flagged?, flagReason?}` | Classifies emotional state from free-text input |
| `assessInteraction(userInput, sessionHistory, channel)` | `-> unknown` | Full interaction assessment object |
| `requiresRrtaHandoff(state)` | `-> boolean` | Returns `true` when emotional state warrants RRT Advocate handoff |
| `reset()` | `-> None` | Resets singleton instance (called during foundation.shutdown) |

### 5.3 `EmotionalState` Type (from sleepwalker-protocol)

| Field | Type | Description |
|-------|------|-------------|
| `stateType` | `string` | Emotional state classification (e.g., `dissociation`, `numbing`, `avoidance`, `detachment`, `neutral`) |
| `protective` | `boolean` | Whether protective psychological state detected |
| `requiresCheckIn` | `boolean` | Whether crisis check-in required |
| `indicators` | `object` | Nested indicator flags: `dissociation`, `numbing`, `avoidance`, `detachment`, `crisis.suicidalIdeation`, `crisis.selfHarm`, `crisis.safetyConcern` |
| `confidence` | `number` | Confidence score (0.0 to 1.0) |
| `explicitSuicidalIdeation` | `boolean` | Explicit statement of suicidal intent |
| `selfHarmIndicators` | `boolean` | Self-harm signals detected |
| `inabilityToEnsureSafety` | `boolean` | User cannot ensure own safety |

**Note:** `channel`, `trusted`, `flagged`, `flagReason` are NOT part of `EmotionalState`. They are added by the foundation integration layer (`sleepwalker.ts`) as provenance envelope fields.

### 5.4 `Channel` Enum

| Value | Description |
|-------|-------------|
| `user_input` | Interaction arrived via user input (trusted) |
| `model_output` | Interaction arrived via model output (untrusted) |
| `tool_result` | Interaction arrived via tool result (untrusted) |
| `system` | Interaction originated from system (untrusted) |
| `unknown` | Channel could not be determined |

### 5.5 Emotional State Assessment Logic (from `sleepwalker.ts`)

1. Input sanitization (fail-open on detection)
2. `getInstance().detectEmotionalState(sanitizedContent, sessionHistory)` — core classification
3. Resolve channel provenance: `state.channel = resolved; state.trusted = resolved === Channel.USER_INPUT`
4. If input flagged by sanitization: `state.flagged = true; state.flagReason = sanitizationResult.reason`
5. Return state with `channel`, `trusted`, and optional `flagged`/`flagReason`

### 5.6 RRT Handoff Condition (from `foundation.ts`)

```text
sleepwalker.requiresRrtaHandoff(state) → true when:
  - state.explicitSuicidalIdeation === true
  OR state.selfHarmIndicators === true
  OR state.inabilityToEnsureSafety === true
```

### 5.7 Security (from sleepwalker.ts)

- Input is sanitized to prevent prompt injection attacks
- Flagged result is logged but assessed defensively (fail-open)
- A flagged result does not suppress a genuine signal

---

## 6. Integration Architecture (from Foundation)

### 6.1 Process Flow (`NeuroLiftFoundation.processInteraction()`)

```text
UserInteraction → normalizeChannel → determine trusted flag

IF interactionType === PREFERENCE_UPDATE AND active.toi:
  → validateTOI(data?.['toi']) → components.push('toi_otoi_framework')

IF interactionType === EMOTIONAL_ASSESSMENT AND active.swp:
  → detectEmotionalState(input, [], channel, userId) → content.emotionalState
  → highSeverity = state.explicitSuicidalIdeation || state.selfHarmIndicators || state.inabilityToEnsureSafety
  → if requiresRrtaHandoff(state) AND active.rrt:
       → gateUp = highSeverity
       → rrt.assess(userId, input, channel) → content.rrt
       → components.push('rrt_advocate')
  → components.push('sleepwalker_protocol')

IF interactionType === CRISIS_ALERT OR EMERGENCY_ESCALATION AND active.rrt:
  → rrt.assess(userId, input, channel) → content.rrt
  → components.push('rrt_advocate')

Final output: { timestamp, responseType, content, componentsInvolved, success }
```

### 6.2 Component Activation Matrix (from `componentsForMode()`)

| FoundationMode | TOI | Sleepwalker | RRT Advocate |
|----------------|-----|-------------|--------------|
| `UNIFIED` | true | true | true |
| `CRISIS_ONLY` | false | false | true |
| `CONTINUITY_ONLY` | false | true | false |
| `FRAMEWORK_ONLY` | true | false | false |
| `DEVELOPMENT` | true | true | false |

### 6.3 Output Contract (`FoundationResponse`)

| Field | Description |
|-------|-------------|
| `timestamp` | `new Date()` |
| `responseType` | The `interactionType` from the input |
| `content` | Record of assessed values (`emotionalState`, `rrt`, `toiValidation`, etc.) |
| `componentsInvolved` | `string[]` of active component names |
| `success` | Always `true` (errors are excluded from content) |

---

## 7. Phase 1 Deliverable

**Output:** `docs/phase-1-reference-analysis.md`

This document provides the complete reference for C++ porting work in Phases 2–7. It includes:

- TOI document model, reserved keys, content sections, tier precedence, canonicalization, and signing
- OTOI charter model, `toi_sources` structure, enforcement policy, resolution/conflict handling
- RRT Advocate crisis assessment API, `CrisisLevel` enum, `CrisisAssessment` type
- Sleepwalker Protocol emotional state assessment API, `EmotionalState` type, `Channel` enum
- Full integration architecture from `NeuroLiftFoundation.processInteraction()`

### 7.1 Next Steps (Per PLAN.md)

| Task | Owner | Status |
|------|-------|--------|
| Create package directory structure (TOI, OTOI, RRT, Sleepwalker) | — | ⬜ Pending |
|| Define core C++ types mapping TOI/OTOI/RRT/Sleepwalker schemas | — | ⬜ Pending |
| Implement core logic in C++ (validators, parsers, assessment engines) | — | ⬜ Pending |
| Write unit tests (Catch2/GoogleTest) | — | ⬜ Pending |
| Create CMake build configuration | — | ⬜ Pending |
| Update governance file registry | — | ⬜ Pending |

---

## 8. Open Questions (Carried Forward from PLAN.md Section 14)

| # | Question | Impact | Status |
|---|----------|--------|--------|
| 1 | Should pillar packages be separate git submodules? | Build complexity | ⬜ Open |
| 2 | C++ standard: C++17 or C++20? | Feature availability (e.g., `std::optional`, `std::variant`) | ⬜ Open |
| 3 | Test framework: Catch2 or GoogleTest? | Dependencies | ⬜ Open |
| 4 | Package manager: vcpkg, Conan, or none? | Distribution | ⬜ Open |
| 5 | How to handle async: coroutines or callbacks? | API design | ⬜ Open |
| 6 | Serialization format: JSON, protobuf, or both? | Interop | ⬜ Open |
| 7 | Logging: spdlog, custom, or pluggable? | Observability | ⬜ Open |

---

*This plan is a living document. Update as implementation progresses.*