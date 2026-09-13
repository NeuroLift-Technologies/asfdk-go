# TOI C++ Package

**ASFDK Terms of Interaction (TOI) C++ Port**

## Overview

This package provides C++ idioms for the Terms of Interaction (`.toi`) file format specification v1.0.0 as defined by the NeuroLift Technologies ASFDK.

## Structure

```
packages/toi/
├── include/toi/       — Header files (TOITypes, TermsOfInteraction)
├── src/               — Implementation (TOI parsing, validation, canonicalization, signing)
├── tests/             — Catch2 unit tests
├── CMakeLists.txt     — Build configuration
└── README.md          — This file
```

## TOI Document Model

A `.toi` document is a JSON object per the `.toi` SPEC.md (RFC 8259, JCS RFC 8785):

- **Reserved keys** (prefixed with `$`): `$toi` (format version), `$tier` (personal/community/project), `$created`, `$updated`, `$id`, `$license`, `$signature`
- **Content sections** (all optional except `identity`): `identity`, `cognitive_profile`, `privacy`, `agency`, `communication`, `ethical_pillars`, `custom`

### Tier Precedence

```
personal > community > project > platform defaults
```

- `personal`-tier documents are **terminal**: fields specified in a personal document MUST NOT be overridden by lower tiers.
- Resolution is **gap-filling**: lower tiers supply values for fields not specified by higher tiers.
- Arrays and scalars are **atomic leaves**: higher tier replaces wholesale.
- Objects are merged **per key**, recursively under the gap-filling rule.

## API Surface

| Function | Description |
|---|---|
| `parseTOI(json)` | Parse JSON into `TOIDocument`, validate required fields |
| `validateTOI(doc)` | Validate document conformance (check `$toi`, `$tier`, `identity.author`) |
| `resolveTOI(docs)` | Resolve TOI stack using tier precedence + gap-filling |
| `canonicalize(json)` | JCS (RFC 8785) canonicalization — sort keys, remove whitespace, minimal escaping |
| `verifySignature(canonicalBytes, signature, publicKey)` | Ed25519 signature verification |

## Build

```bash
# From repository root
mkdir -p build && cd build
cmake ../packages/toi -DVPKG_DIR=${VPKG_DIR}
cmake --build .
# Run tests: ./tests/test_toi_manager
```

## Testing

Unit tests cover:
- Minimal TOI document parsing and validation
- Full TOI document with all sections
- Invalid document rejection (missing `$toi`, missing `identity.author`)
- Tier precedence resolution (`personal > community > project`)
- `ethical_pillars` array parsing

## Migration from TypeScript Reference

The original ASFDK TOI is implemented in TypeScript (`@neurolift-technologies/toi@1.0.3`) with a Zod schema. The C++ port maps:

| TypeScript | C++ |
|---|---|
| `interface` | `struct` + `enum class` |
| `type Foo = 'a' | 'b' | `enum class Foo { A, B }` |
| `optional T` | `std::optional<T>` |
| `Record<K,V>` | `std::unordered_map<K,V>` |
| `Promise<T>` | `std::future<T>` or callbacks |
| GC-managed | RAII / smart pointers |
| `interface` (polymorphic) | `struct` + `virtual` base |

## Governance

- All commits follow `[AGENT_NAME] type(scope): description`
- `validate-governance.sh` must pass (22/22 checks)
- No LLM provider lock-in
- No architecture decisions without human approval
- No production deployments without sign-off