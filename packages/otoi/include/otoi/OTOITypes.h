#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <variant>
#include <functional>
#include <nlohmann/json.hpp>

namespace otoi {

// ============ Enums ============

// Tier values for .toi documents and .otoi sources
enum class Tier {
    Personal,
    Community,
    Project
};

// Enforcement modes (ascending strictness)
enum class EnforcementMode {
    Advisory,
    Enforced,
    Strict
};

// Conflict resolution strategies for same-tier disagreements
enum class ConflictStrategy {
    HighestTierWins,
    Reject,
    Escalate
};

// Unsupported preference strategies
enum class UnsupportedStrategy {
    Ignore,
    Degrade,
    Reject
};

// ============ Agent Types ============

// A participant in the mesh that must honor resolved preferences
struct OtoiAgent {
    std::string id;
    std::optional<std::string> role;
    std::optional<std::vector<std::string>> modalities;
    std::optional<std::vector<std::string>> affordances;
};

// ============ Source Types ============

// A reference to one .toi document participating in resolution
// Must provide either uri or inline (never both)
struct OtoiSource {
    Tier tier;
    std::optional<std::string> uri;
    std::optional<nlohmann::json> inline_doc; // opaque .toi document, validated by TOI layer
};

// ============ Enforcement Types ============

// Raw enforcement settings as authored in the charter
struct OtoiEnforcement {
    std::optional<EnforcementMode> mode;
    std::optional<ConflictStrategy> on_conflict;
    std::optional<UnsupportedStrategy> on_unsupported;
    std::optional<bool> audit;
};

// Enforcement settings with every field resolved to a concrete value
struct ResolvedEnforcement {
    EnforcementMode mode;
    ConflictStrategy on_conflict;
    UnsupportedStrategy on_unsupported;
    bool audit;
};

// ============ Conflict Types ============

// A same-tier disagreement on a single leaf field across two .toi documents
struct PolicyConflict {
    Tier tier;
    std::string path;              // dotted path to contested leaf
    std::vector<std::string> values; // distinct values asserted at that path
};

// ============ Charter Types ============

// The full .otoi charter document
// Reserved keys: $otoi, $id, $created, $updated
// Content keys: identity, agents, enforcement, toi_sources
struct OtoiCharter {
    std::string $otoi;             // "1.0.0" - required
    std::optional<std::string> $id;
    std::optional<std::string> $created;
    std::optional<std::string> $updated;
    std::optional<std::unordered_map<std::string, std::string>> identity; // author required if present
    std::vector<OtoiAgent> agents;
    std::optional<OtoiEnforcement> enforcement;
    std::vector<OtoiSource> toi_sources;
    // Forward-compatibility: preserve unknown keys
    std::unordered_map<std::string, nlohmann::json> extra;
};

// ============ Result Types ============

// Options for honor()
struct HonorOptions {
    std::optional<std::vector<nlohmann::json>> documents; // already-parsed .toi documents
    std::optional<nlohmann::json> platform_defaults;
    std::optional<std::function<std::string(const std::string&)>> load_source; // uri -> .toi text
};

// The synthesized result every agent in the mesh honors
struct EffectivePolicy {
    nlohmann::json effective;      // resolved effective .toi view (tier precedence applied)
    std::vector<Tier> tiers;       // which tiers contributed, highest precedence first
    std::vector<OtoiAgent> agents; // agents bound by the charter
    ResolvedEnforcement enforcement;
    std::vector<PolicyConflict> conflicts;
};

// ============ Error Types ============

enum class OtoiErrorCode {
    Parse,
    Validation,
    Honor
};

// A single charter-schema violation, flattened to a dotted path and a message
struct OtoiIssue {
    std::string path;
    std::string message;
};

// Base error type for OTOI operations
struct OtoiError {
    OtoiErrorCode code;
    std::string message;
    std::optional<std::vector<OtoiIssue>> issues;
    std::optional<std::vector<PolicyConflict>> conflicts;
};

// Input was not well-formed JSON, or its root was not a JSON object
struct OtoiParseError : OtoiError {
    OtoiParseError(const std::string& msg) {
        code = OtoiErrorCode::Parse;
        message = msg;
    }
};

// A charter violated the .otoi schema
struct OtoiValidationError : OtoiError {
    std::vector<OtoiIssue> issues;
    OtoiValidationError(const std::string& msg, std::vector<OtoiIssue> iss) {
        code = OtoiErrorCode::Validation;
        message = msg;
        issues = std::move(iss);
    }
};

// Honoring could not be completed
struct OtoiHonorError : OtoiError {
    std::vector<PolicyConflict> conflicts;
    OtoiHonorError(const std::string& msg, std::vector<PolicyConflict> conf) {
        code = OtoiErrorCode::Honor;
        message = msg;
        conflicts = std::move(conf);
    }
};

// ============ Helper Functions ============

// Convert Tier enum to string
inline std::string to_string(Tier t) {
    switch (t) {
        case Tier::Personal: return "personal";
        case Tier::Community: return "community";
        case Tier::Project: return "project";
    }
    return "unknown";
}

// Convert string to Tier enum
inline std::optional<Tier> tier_from_string(const std::string& s) {
    if (s == "personal") return Tier::Personal;
    if (s == "community") return Tier::Community;
    if (s == "project") return Tier::Project;
    return std::nullopt;
}

// Convert EnforcementMode enum to string
inline std::string to_string(EnforcementMode m) {
    switch (m) {
        case EnforcementMode::Advisory: return "advisory";
        case EnforcementMode::Enforced: return "enforced";
        case EnforcementMode::Strict: return "strict";
    }
    return "unknown";
}

// Convert string to EnforcementMode enum
inline std::optional<EnforcementMode> enforcement_mode_from_string(const std::string& s) {
    if (s == "advisory") return EnforcementMode::Advisory;
    if (s == "enforced") return EnforcementMode::Enforced;
    if (s == "strict") return EnforcementMode::Strict;
    return std::nullopt;
}

// Convert ConflictStrategy enum to string
inline std::string to_string(ConflictStrategy s) {
    switch (s) {
        case ConflictStrategy::HighestTierWins: return "highest-tier-wins";
        case ConflictStrategy::Reject: return "reject";
        case ConflictStrategy::Escalate: return "escalate";
    }
    return "unknown";
}

// Convert string to ConflictStrategy enum
inline std::optional<ConflictStrategy> conflict_strategy_from_string(const std::string& s) {
    if (s == "highest-tier-wins") return ConflictStrategy::HighestTierWins;
    if (s == "reject") return ConflictStrategy::Reject;
    if (s == "escalate") return ConflictStrategy::Escalate;
    return std::nullopt;
}

// Convert UnsupportedStrategy enum to string
inline std::string to_string(UnsupportedStrategy s) {
    switch (s) {
        case UnsupportedStrategy::Ignore: return "ignore";
        case UnsupportedStrategy::Degrade: return "degrade";
        case UnsupportedStrategy::Reject: return "reject";
    }
    return "unknown";
}

// Convert string to UnsupportedStrategy enum
inline std::optional<UnsupportedStrategy> unsupported_strategy_from_string(const std::string& s) {
    if (s == "ignore") return UnsupportedStrategy::Ignore;
    if (s == "degrade") return UnsupportedStrategy::Degrade;
    if (s == "reject") return UnsupportedStrategy::Reject;
    return std::nullopt;
}

// Convert OtoiErrorCode to string
inline std::string to_string(OtoiErrorCode c) {
    switch (c) {
        case OtoiErrorCode::Parse: return "PARSE";
        case OtoiErrorCode::Validation: return "VALIDATION";
        case OtoiErrorCode::Honor: return "HONOR";
    }
    return "UNKNOWN";
}

} // namespace otoi
