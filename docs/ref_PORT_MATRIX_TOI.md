# ASFDK C++ Port Matrix — TOI

Source: `@neurolift-technologies/toi@1.0.3` (npm, verified via direct registry
pull — `npm view` + `npm pack`, not secondhand). SPEC.md is the normative
reference; where SPEC and code disagree, SPEC + Zod schema govern (per SPEC §1).

## Reference package

| Field | Value |
|---|---|
| Package | `@neurolift-technologies/toi@1.0.3` |
| Unpacked size | 177.0 kB, 53 files |
| Deps | `@noble/ed25519@^2.1.0`, `@noble/hashes@^1.4.0`, `zod@^4.0.0` |
| Spec | `SPEC.md` v1.0.0 (Stable), + JSON Schema draft 2020-12 (`schema/toi-1.0.0.schema.json`) |
| Consumed by | `otoi@>=1.0.0` (only pillar with an internal cross-pillar dependency) |

## Public API surface (by module)

| Module | Exports | C++ port target |
|---|---|---|
| `types.ts` | `ToiDocument`, `ToiSignature` (both `z.infer` — schema-derived, never hand-declared) | `struct ToiDocument`, `struct ToiSignature` — hand-written structs kept in lockstep with a schema validator, since C++ has no type-inference-from-schema equivalent |
| `schema.ts` | `toiSchema`, `toiSignatureSchema` (Zod — **the actual source of truth** per SPEC §1) | Needs a real validation layer: hand-rolled visitor, or a JSON Schema validator (`valijson`, `nlohmann-json-schema-validator`) driven from the *published* `schema/toi-1.0.0.schema.json` artifact rather than re-deriving by hand |
| `constants.ts` | `TOI_FORMAT_VERSION`, `TOI_FILE_EXTENSION`, `TOI_MEDIA_TYPE`, `TOI_RESERVED_PREFIX`, `TOI_RESERVED_KEYS`, `TOI_TIERS`, `ToiTier`, `TIER_PRECEDENCE`, `TIER_RANK` | `constexpr` values + `enum class ToiTier { Personal, Community, Project }`; straightforward, no semantic risk |
| `errors.ts` | `ToiError` (base), `ToiErrorCode` (5-way discriminant), `ToiParseError`, `ToiValidationError` (+`ToiIssue[]`), `ToiCanonicalizationError`, `ToiSignatureError`, `ToiTierError` | **Open decision, not default**: exception hierarchy (`ToiError : std::exception`) vs `std::expected<T, ToiError>` result type. JS already has a throwing/non-throwing split at the API level (`parseToi` vs `safeParseToi`) — C++ port should preserve *both* paths, not collapse to one |
| `canonicalize.ts` | `canonicalize`, `canonicalizeToBytes`, `JsonValue` | RFC 8785 (JCS) implementation. **Risk flagged below.** |
| `b64url.ts` | (not yet inspected in detail — bytes↔base64url helpers) | Direct port; low risk, well-defined RFC 4648 §5 |
| `sign.ts` | `ToiKeyPair`, `generateKeyPair`, `signingPayload`, `isSigned`, `signToi`, `verifyToi` | Ed25519 via libsodium (`crypto_sign_ed25519`) or equivalent. `verifyToi` **must never throw** — defensive contract, returns `false` on any malformed input. Padded/whitespaced base64url is deliberately **rejected**, not normalized — do not "fix" this in port |
| `tier.ts` | `compareTier`, `sortByPrecedence`, `ResolveOptions`, `resolveToi` | Recursive gap-fill merge over heterogeneous JSON. **Needs a real JSON value type** (`std::variant`-based or a JSON library), not typed structs — the merge is generic over unknown/forward-compat keys and the open `custom` bag |
| `parse.ts` | `SerializeOptions`, `SafeParseResult` (discriminated union), `parseToi` (throws), `safeParseToi` (no-throw), `isToi`, `serializeToi` | Maps cleanly to C++ exception + `std::expected`/`std::optional` dual API |
| `generator.ts` | `DEFAULT_DOCUMENT` (privacy-first defaults — **policy, not incidental**), `FromDefaultsOptions`, `FromDictOptions`, `TOIDocumentGenerator` class (`fromDefaults`, `fromDict`, `validate`, `toDict`, `toJson`, `toMarkdown`, `write`) | Class with clear ownership/lifecycle; `toMarkdown()` is a review aid only, not canonical — don't let it drift into being treated as a serialization format |
| `extract.ts` | `ExtractOptions`, `extractToi` | **Highest-risk module.** ~452 lines, pure deterministic regex/phrase matching (explicitly documented as "no LLM, no network" — a design invariant, not an implementation detail). See risk note below |

## Behavioral rules to preserve exactly

1. **Declarative, never executable** (SPEC §2, §13). A `.toi` document is data. No field — including `custom` — may ever be evaluated as instructions. This is a security invariant, not a style preference; the C++ port must not add any "convenience" evaluation path.
2. **Forward compatibility is structural** (SPEC §8): every object is loose/open. A v1.0.0 processor MUST preserve unknown keys, at every nesting level, including unrecognized `$`-prefixed reserved keys. A hand-written C++ struct-per-section approach will silently violate this unless every section also carries an "extra/unknown fields" bag.
3. **Tier resolution is terminal + gap-filling, not deep-merge** (SPEC §9): `personal` is authoritative and unmodifiable by lower tiers; arrays/scalars are atomic (never merged); only plain objects merge, recursively, per-key.
4. **Canonicalization is presentation-independent**: signing operates on RFC 8785 bytes, never on-disk formatting. `serializeToi`'s pretty-printing is NOT the signing form — keep these two code paths structurally separate in C++ so no future contributor "optimizes" them into one.
5. **`verifyToi` never throws** under any input. This is an explicit defensive contract in the JS source, not accidental.
6. **Privacy-first defaults** (`generator.ts`): `DEFAULT_DOCUMENT` encodes a policy stance (most-restrictive/most-private options), referenced back to project governance (`CLAUDE.md`). This must be a deliberate, documented constant in C++, not whatever a zero-initialized struct happens to produce.

## Cross-language risk register

| Risk | Detail | Mitigation |
|---|---|---|
|| **UTF-16 vs UTF-8 sort order** | RFC 8785 canonicalization sorts object keys by UTF-16 code unit (JS native string comparison). C++ `std::string` holds UTF-8 bytes; naive `operator<` byte-comparison does **not** match UTF-16 code-unit order for all Unicode ranges (surrogate pairs, astral-plane chars). A signature computed by the C++ port could silently diverge from one computed by TS/Python for documents containing such characters. | Write an explicit UTF-8→UTF-16-code-unit comparator that decodes to UTF-16 code units (not Unicode code points) and compares those unsigned 16-bit values. Add conformance tests covering supplementary characters (non-BMP) and BMP characters above the surrogate range. |
| **Regex dialect divergence (`extract.ts`)** | JS regex semantics (lookahead/lookbehind support, Unicode word-boundary behavior, the manual `positive()` negation-window workaround) may not translate identically to `std::regex`, PCRE2, or RE2. The manual lookbehind-simulation code (`positive()`) exists specifically to work around a JS limitation — a C++ regex engine with native lookbehind might not need the workaround, which is itself a difference to document, not silently "improve." | Treat `extract.ts` as its own sub-matrix: enumerate every regex pattern and test string, verify identical match/no-match behavior in the target C++ regex engine before porting logic, not after. Consider whether `extractToi` even belongs in a game-engine-facing C++ port at all — it's a natural-language authoring aid, not a runtime governance primitive; may be lower priority than the runtime-critical modules (schema, sign, tier). |
| **Zod schema as source of truth with no C++ equivalent** | SPEC §1 explicitly names the Zod schema (not the prose, not the JSON Schema artifact) as normative. C++ has no direct analog. | Treat the **published JSON Schema artifact** (`schema/toi-1.0.0.schema.json`, JSON Schema draft 2020-12) as the practical source of truth for the C++ port instead, since it's derivable and toolable, and cross-check it against the Zod `.d.ts` shapes captured above at each pillar version bump. |
| **`structuredClone` fallback** | `tier.ts`'s `clone()` uses `structuredClone` where available, else `JSON.parse(JSON.stringify(...))`. Both assume a JSON-only value space (already guaranteed by schema validation upstream). | Direct port: deep-copy of a JSON value tree. No special risk once the JSON value type is chosen, but confirm the C++ JSON type's deep-copy semantics match (value semantics by default, e.g. via `nlohmann::json` copy, not a shared-pointer aliasing bug). |
| **Version drift** | ASFDK umbrella `0.2.4` pins `toi@^1.0.1`; actual latest is `1.0.3`. Unknown whether `1.0.1→1.0.3` changed schema/behavior in ways relevant to the port. | Diff `1.0.1` against `1.0.3` before treating `1.0.3` as the sole reference — pull `1.0.1` from the registry too if the umbrella's pin is meaningful, or confirm with Josh whether the umbrella's pin is just stale and `1.0.3` is the real target. |

## Open questions for Josh

1. Exception-vs-`std::expected` — pick one policy for the whole C++ port now, since OTOI/RRT/Sleepwalker will follow whatever TOI establishes.
2. Is `extractToi` (NL extraction) in scope for the *Unreal-facing* runtime path at all, or is it authoring tooling that stays server/dev-side and never needs to run inside the World Engine? This affects priority ordering within the TOI port itself.
3. Which JSON library for the C++ core (`nlohmann::json`, `simdjson`, `rapidjson`, or a minimal hand-rolled variant type)? This decision cascades into OTOI and the tier-merge logic and should be made once, here, not per-pillar.
4. Confirm `1.0.1` (umbrella's pin) vs `1.0.3` (latest) — do we port against latest and update the umbrella's pin, or is `0.2.4`'s pin intentional?
