#pragma once

#include "TOITypes.h"
// nlohmann_json include is provided by CMake target_include_directories
// when find_package(nlohmann_json) is used. The compiler will resolve
// <nlohmann/json.hpp> via the library's include directories.
#include <nlohmann/json.hpp>

// tl::expected provides std::expected-compatible expected<T,E> for C++17/20
// The DECISIONS.md (Section 2) specifies dual error handling:
//   - Throwing variant: parseTOI() throws on invalid document
//   - Non-throwing variant: safeParseTOI() returns tl::expected<T,E>
// We use tl::expected as the C++17/20 backport for std::expected
#include <tl/expected.hpp>

namespace toi {

// Error type for TOI parsing/validation failures
struct TOIError {
    enum class Code {
        VersionMismatch,     // $toi version not "1.0.0"
        InvalidTier,         // $tier not personal/community/project
        MissingAuthor,       // identity.author empty or missing
        InvalidCognitiveProfile, // cognitive_profile field invalid
        InvalidPrivacy,      // privacy field invalid
        InvalidAgency,       // agency field invalid
        InvalidCommunication, // communication field invalid
        UnknownError         // generic fallback
    };
    Code code;
    std::string message;
};

// Parse a TOI document from a JSON object.
// Throws std::invalid_argument on invalid document (per DECISIONS.md §2)
// The throwing variant is the primary API; callers should use safeParseTOI()
// for non-throwing error handling.
TOIDocument parseTOI(const nlohmann::json& json);

// Non-throwing variant: returns tl::expected<TOIDocument, TOIError>
// If the document is invalid, returns tl::unexpected<TOIError>
// This provides the dual API pattern specified in DECISIONS.md §2:
tl::expected<TOIDocument, TOIError> safeParseTOI(const nlohmann::json& json);

// Validate a TOIDocument against the canonical schema.
// Returns true if valid, false otherwise.
// This is a non-throwing validation pass used by safeParseTOI.
bool validateTOI(const TOIDocument& doc);

// Resolve a stack of TOI documents into one effective document
// using tier precedence: personal > community > project > platform defaults.
// Gap-filling: lower tiers supply values for fields not specified by higher tiers.
// Arrays and scalars are atomic leaves (higher tier replaces wholesale).
// Objects are merged per key recursively under the gap-filling rule.
TOIDocument resolveTOI(std::vector<TOIDocument> docs);

// Canonicalizes a TOI document using JCS (JSON Canonicalization Scheme, RFC 8785).
// - Object keys sorted by UTF-16 code units, recursively
// - Insignificant whitespace removed
// - Strings use JSON minimal escaping
// - Numbers use ECMAScript Number-to-string form
// - Array order preserved
std::string canonicalize(const std::string& json);

// Verifies an Ed25519 signature on a TOI document.
// - Removes $signature key from document
// - Canonicalizes remaining document per JCS RFC 8785
// - Encodes as UTF-8
// - Verifies Ed25519 signature over those bytes with the decoded public key
bool verifySignature(const std::string& canonicalBytes,
                     const nlohmann::json& signature,
                     const std::vector<uint8_t>& publicKey);



} // namespace toi