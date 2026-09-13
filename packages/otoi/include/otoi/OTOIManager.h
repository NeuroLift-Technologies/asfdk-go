#pragma once

#include "OTOITypes.h"
// Use tl::expected as C++17/20 backport for std::expected
#include <tl/expected.hpp>
#include <atomic>
#include <nlohmann/json.hpp>

namespace otoi {

// Forward declarations
class OTOIManager;

// ============ GovernanceContext ============

// Runtime governance context for an agent session
struct GovernanceContext {
    std::string session_id;
    std::string agent_id;
    std::optional<Tier> effective_tier;
    ResolvedEnforcement enforcement;
    nlohmann::json effective_preferences;
    bool strict_mode_active() const {
        return enforcement.mode == EnforcementMode::Strict;
    }
    bool audit_enabled() const {
        return enforcement.audit;
    }
};

// ============ OTOIValidator ============

// Validates .otoi charter documents against the canonical schema
class OTOIValidator {
public:
    OTOIValidator() = default;

    // Validate a parsed OtoiCharter
    // Returns tl::expected<void, OtoiValidationError>
    // On success: returns void
    // On failure: returns unexpected with validation issues
    tl::expected<void, OtoiValidationError> validate(const OtoiCharter& charter) const;

    // Validate a raw JSON object as an .otoi charter
    // Throws OtoiParseError if input is not valid JSON object
    // Returns tl::expected<OtoiCharter, OtoiError> with validation issues if invalid
    tl::expected<OtoiCharter, OtoiError> parseAndValidate(const nlohmann::json& json) const;

    // Detect same-tier conflicts across .toi documents
    // Cross-tier disagreement is NOT a conflict — that is what tier precedence is for
    std::vector<PolicyConflict> detectConflicts(const std::vector<nlohmann::json>& documents) const;

public:
    // Resolve enforcement to concrete values with defaults.
    // Pure resolver over raw enforcement settings; exposed so OTOIManager
    // can reuse it when honoring charters.
    ResolvedEnforcement resolveEnforcement(const OtoiEnforcement& raw) const;

private:
    // Validate reserved keys
    std::vector<OtoiIssue> validateReservedKeys(const nlohmann::json& json) const;

    // Validate identity section
    std::vector<OtoiIssue> validateIdentity(const nlohmann::json& identity) const;

    // Validate agents array
    std::vector<OtoiIssue> validateAgents(const nlohmann::json& agents) const;

    // Validate enforcement policy
    std::vector<OtoiIssue> validateEnforcement(const nlohmann::json& enforcement) const;

    // Validate toi_sources array
    std::vector<OtoiIssue> validateToiSources(const nlohmann::json& sources) const;
};

// ============ OTOIManager ============

// Main entry point for OTOI operations
// Composes validator, conflict detection, and honor logic
class OTOIManager {
public:
    OTOIManager() = default;

    // Parse an .otoi charter from JSON
    // Throws OtoiParseError if input is not valid JSON object
    // Throws OtoiValidationError if charter violates schema
    OtoiCharter parseCharter(const nlohmann::json& json);

    // Non-throwing parse variant
    // Returns tl::expected<OtoiCharter, OtoiError>
    tl::expected<OtoiCharter, OtoiError> safeParseCharter(const nlohmann::json& json) const;

    // Fold a charter and its .toi sources into one effective policy
    // Resolution delegates tier precedence to TOI layer's resolveToi
    // This function adds source loading, same-tier conflict detection, and enforcement
    //
    // Throws OtoiHonorError when:
    //   - Nothing to resolve
    //   - A uri source is present without a loadSource callback
    //   - A same-tier conflict is found under on_conflict: "reject"
    //
    // Returns EffectivePolicy with resolved preferences
    EffectivePolicy honor(const OtoiCharter& charter, const HonorOptions& options = {});

    // Non-throwing honor variant
    // Returns tl::expected<EffectivePolicy, OtoiHonorError>
    tl::expected<EffectivePolicy, OtoiHonorError> safeHonor(const OtoiCharter& charter, const HonorOptions& options = {}) const;

    // Return the effective preferences a specific agent must honor
    // Under strict enforcement, an agent not declared in charter mesh is refused
    nlohmann::json propagate(const EffectivePolicy& policy, const std::string& agentId) const;

    // Get the current status of the TOI-OTOI component
    struct Status {
        bool active;
        EnforcementMode mode;
    };
    Status getStatus() const;

private:
    // Load a .toi source (uri or inline)
    nlohmann::json loadSource(const OtoiSource& source, const HonorOptions& options) const;

    // Resolve a stack of .toi documents into one effective document
    nlohmann::json resolveDocuments(const std::vector<nlohmann::json>& documents, const ResolvedEnforcement& enforcement) const;

    // Apply enforcement policy to resolved preferences
    nlohmann::json applyEnforcement(const nlohmann::json& effective, const ResolvedEnforcement& enforcement) const;

    // Check if an agent is declared in the charter mesh
    bool isAgentDeclared(const OtoiCharter& charter, const std::string& agentId) const;

    // Current component state
    // Both m_active and m_mode are atomically updated by safeHonor() (const)
    // to record governance engagement and the charter-resolved enforcement mode.
    // Using std::atomic for both fields ensures safe concurrent access between
    // safeHonor() writes (any thread) and getStatus() reads (const, any thread).
    // The release store on m_active in safeHonor() synchronizes with the acquire
    // load in getStatus(), establishing a proper happens-before relationship
    // for m_mode reads as well.
    mutable std::atomic<bool> m_active{true};
    mutable std::atomic<EnforcementMode> m_mode{EnforcementMode::Enforced};
};

// ============ Convenience Functions ============

// Parse charter with automatic error conversion (non-throwing)
inline tl::expected<OtoiCharter, OtoiError> parseCharterSafe(const nlohmann::json& json) {
    OTOIManager manager;
    return manager.safeParseCharter(json);
}

// Honor charter with automatic error conversion (non-throwing)
inline tl::expected<EffectivePolicy, OtoiHonorError> honorSafe(const OtoiCharter& charter, const HonorOptions& options = {}) {
    OTOIManager manager;
    return manager.safeHonor(charter, options);
}

} // namespace otoi
