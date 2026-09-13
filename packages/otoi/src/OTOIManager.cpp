#include "otoi/OTOIManager.h"
#include "otoi/OTOITypes.h"
#include <set>
#include <stdexcept>
#include <algorithm>
#include <sstream>

namespace otoi {

// ============ OTOIValidator Implementation ============

tl::expected<void, OtoiValidationError> OTOIValidator::validate(const OtoiCharter& charter) const {
    std::vector<OtoiIssue> issues;

    // Validate $otoi version
    if (charter.$otoi != "1.0.0") {
        issues.push_back({"$otoi", "Version must be '1.0.0', got: " + charter.$otoi});
    }

    // Validate identity if present
    if (charter.identity.has_value()) {
        auto identityIssues = validateIdentity(nlohmann::json(*charter.identity));
        issues.insert(issues.end(), identityIssues.begin(), identityIssues.end());
    }

    // Validate agents array (bridge typed agents to the JSON shape the
    // section validators expect)
    nlohmann::json agentsJson = nlohmann::json::array();
    for (const auto& agent : charter.agents) {
        nlohmann::json a{{"id", agent.id}};
        if (agent.role.has_value()) a["role"] = *agent.role;
        if (agent.modalities.has_value()) a["modalities"] = *agent.modalities;
        if (agent.affordances.has_value()) a["affordances"] = *agent.affordances;
        agentsJson.push_back(std::move(a));
    }
    auto agentIssues = validateAgents(agentsJson);
    issues.insert(issues.end(), agentIssues.begin(), agentIssues.end());

    // Validate enforcement policy if present
    auto modeLabel = [](EnforcementMode m) {
        switch (m) {
            case EnforcementMode::Advisory: return "advisory";
            case EnforcementMode::Enforced: return "enforced";
            case EnforcementMode::Strict: return "strict";
        }
        return "enforced";
    };
    auto conflictLabel = [](ConflictStrategy s) {
        switch (s) {
            case ConflictStrategy::HighestTierWins: return "highest-tier-wins";
            case ConflictStrategy::Reject: return "reject";
            case ConflictStrategy::Escalate: return "escalate";
        }
        return "highest-tier-wins";
    };
    auto unsupportedLabel = [](UnsupportedStrategy s) {
        switch (s) {
            case UnsupportedStrategy::Ignore: return "ignore";
            case UnsupportedStrategy::Degrade: return "degrade";
            case UnsupportedStrategy::Reject: return "reject";
        }
        return "degrade";
    };
    if (charter.enforcement.has_value()) {
        nlohmann::json enforcementJson = nlohmann::json::object();
        if (charter.enforcement->mode.has_value()) {
            enforcementJson["mode"] = modeLabel(*charter.enforcement->mode);
        }
        if (charter.enforcement->on_conflict.has_value()) {
            enforcementJson["on_conflict"] = conflictLabel(*charter.enforcement->on_conflict);
        }
        if (charter.enforcement->on_unsupported.has_value()) {
            enforcementJson["on_unsupported"] = unsupportedLabel(*charter.enforcement->on_unsupported);
        }
        auto enforcementIssues = validateEnforcement(enforcementJson);
        issues.insert(issues.end(), enforcementIssues.begin(), enforcementIssues.end());
    }

    // Validate toi_sources array
    auto tierLabel = [](Tier t) {
        switch (t) {
            case Tier::Personal: return "personal";
            case Tier::Community: return "community";
            case Tier::Project: return "project";
        }
        return "project";
    };
    nlohmann::json sourcesJson = nlohmann::json::array();
    for (const auto& source : charter.toi_sources) {
        nlohmann::json s{{"tier", tierLabel(source.tier)}};
        // Emit each field independently rather than as an else-if chain so
        // validateToiSources can still flag mutually-exclusive uri/inline
        // sources when both happen to be present.
        if (source.uri.has_value()) {
            s["uri"] = *source.uri;
        }
        if (source.inline_doc.has_value()) {
            s["inline"] = *source.inline_doc;
        }
        sourcesJson.push_back(std::move(s));
    }
    auto sourceIssues = validateToiSources(sourcesJson);
    issues.insert(issues.end(), sourceIssues.begin(), sourceIssues.end());

    if (!issues.empty()) {
        return tl::unexpected<OtoiValidationError>(
            OtoiValidationError("Charter validation failed", std::move(issues))
        );
    }

    return {};
}

tl::expected<OtoiCharter, OtoiError> OTOIValidator::parseAndValidate(const nlohmann::json& json) const {
    if (!json.is_object()) {
        return tl::unexpected<OtoiParseError>("Charter must be a JSON object");
    }

    OtoiCharter charter;

    // Reserved keys
    if (json.contains("$otoi") && json["$otoi"].is_string()) {
        charter.$otoi = json["$otoi"].get<std::string>();
    }
    if (json.contains("$id") && json["$id"].is_string()) {
        charter.$id = json["$id"].get<std::string>();
    }
    if (json.contains("$created") && json["$created"].is_string()) {
        charter.$created = json["$created"].get<std::string>();
    }
    if (json.contains("$updated") && json["$updated"].is_string()) {
        charter.$updated = json["$updated"].get<std::string>();
    }

    // Identity
    if (json.contains("identity") && json["identity"].is_object()) {
        const auto& id = json["identity"];
        std::unordered_map<std::string, std::string> identity;
        for (const auto& [key, value] : id.items()) {
            if (value.is_string()) {
                identity[key] = value.get<std::string>();
            }
        }
        charter.identity = identity;
    }

    // Agents
    if (json.contains("agents") && json["agents"].is_array()) {
        for (const auto& agent : json["agents"]) {
            if (!agent.is_object()) continue;
            OtoiAgent otoiAgent;
            if (agent.contains("id") && agent["id"].is_string()) {
                otoiAgent.id = agent["id"].get<std::string>();
            }
            if (agent.contains("role") && agent["role"].is_string()) {
                otoiAgent.role = agent["role"].get<std::string>();
            }
            if (agent.contains("modalities") && agent["modalities"].is_array()) {
                std::vector<std::string> modalities;
                for (const auto& m : agent["modalities"]) {
                    if (m.is_string()) modalities.push_back(m.get<std::string>());
                }
                otoiAgent.modalities = modalities;
            }
            if (agent.contains("affordances") && agent["affordances"].is_array()) {
                std::vector<std::string> affordances;
                for (const auto& a : agent["affordances"]) {
                    if (a.is_string()) affordances.push_back(a.get<std::string>());
                }
                otoiAgent.affordances = affordances;
            }
            charter.agents.push_back(otoiAgent);
        }
    }

    // Enforcement
    if (json.contains("enforcement") && json["enforcement"].is_object()) {
        const auto& enf = json["enforcement"];
        OtoiEnforcement enforcement;
        if (enf.contains("mode") && enf["mode"].is_string()) {
            auto mode = enforcement_mode_from_string(enf["mode"].get<std::string>());
            if (mode) enforcement.mode = *mode;
        }
        if (enf.contains("on_conflict") && enf["on_conflict"].is_string()) {
            auto strategy = conflict_strategy_from_string(enf["on_conflict"].get<std::string>());
            if (strategy) enforcement.on_conflict = *strategy;
        }
        if (enf.contains("on_unsupported") && enf["on_unsupported"].is_string()) {
            auto strategy = unsupported_strategy_from_string(enf["on_unsupported"].get<std::string>());
            if (strategy) enforcement.on_unsupported = *strategy;
        }
        if (enf.contains("audit") && enf["audit"].is_boolean()) {
            enforcement.audit = enf["audit"].get<bool>();
        }
        charter.enforcement = enforcement;
    }

    // toi_sources
    if (json.contains("toi_sources") && json["toi_sources"].is_array()) {
        for (const auto& source : json["toi_sources"]) {
            if (!source.is_object()) continue;
            OtoiSource otoiSource;
            if (source.contains("tier") && source["tier"].is_string()) {
                auto tier = tier_from_string(source["tier"].get<std::string>());
                if (tier) otoiSource.tier = *tier;
            }
            if (source.contains("uri") && source["uri"].is_string()) {
                otoiSource.uri = source["uri"].get<std::string>();
            }
            if (source.contains("inline") && !source["inline"].is_null()) {
                otoiSource.inline_doc = source["inline"];
            }
            charter.toi_sources.push_back(otoiSource);
        }
    }

    // Preserve unknown keys for forward compatibility
    for (const auto& [key, value] : json.items()) {
        if (key[0] != '$' && key != "identity" && key != "agents" &&
            key != "enforcement" && key != "toi_sources") {
            charter.extra[key] = value;
        }
    }

    // Validate the parsed charter
    auto validation = validate(charter);
    if (!validation.has_value()) {
        return tl::unexpected<OtoiError>(
            OtoiValidationError(validation.error().message, std::move(validation.error().issues))
        );
    }

    return charter;
}

std::vector<PolicyConflict> OTOIValidator::detectConflicts(const std::vector<nlohmann::json>& documents) const {
    std::vector<PolicyConflict> conflicts;

    if (documents.size() < 2) {
        return conflicts; // Need at least 2 documents to have a conflict
    }

    // Group documents by tier
    std::unordered_map<std::string, std::vector<nlohmann::json>> byTier;
    for (const auto& doc : documents) {
        if (!doc.contains("$tier") || !doc["$tier"].is_string()) continue;
        std::string tier = doc["$tier"].get<std::string>();
        byTier[tier].push_back(doc);
    }

    // Check for conflicts within each tier
    for (const auto& [tier, tierDocs] : byTier) {
        auto resolvedTier = tier_from_string(tier);
        if (!resolvedTier.has_value()) {
            // Unresolvable $tier: surface it as a conflict so callers using
            // ConflictStrategy::Reject refuse rather than silently dropping
            // the document (resolveDocuments drops them, so the two paths
            // must agree — see resolveDocuments).
            PolicyConflict conflict;
            conflict.tier = Tier::Project;
            conflict.path = "$tier";
            conflict.values = {tier};
            conflicts.push_back(conflict);
            continue;
        }
        if (tierDocs.size() < 2) continue;

        // Collect all leaf paths across documents in this tier
        std::set<std::string> allPaths;
        for (const auto& doc : tierDocs) {
            std::function<void(const nlohmann::json&, std::string)> collectPaths =
                [&](const nlohmann::json& obj, std::string path) {
                    if (obj.is_object()) {
                        for (const auto& [key, value] : obj.items()) {
                            std::string newPath = path.empty() ? key : path + "." + key;
                            if (key[0] == '$') continue; // Skip reserved keys
                            if (value.is_object()) {
                                collectPaths(value, newPath);
                            } else {
                                allPaths.insert(newPath);
                            }
                        }
                    }
                };
            collectPaths(doc, "");
        }

        // Check each path for conflicting values
        for (const auto& path : allPaths) {
            std::set<std::string> values;
            for (const auto& doc : tierDocs) {
                std::function<bool(const nlohmann::json&, std::string)> getValue =
                    [&](const nlohmann::json& obj, std::string p) -> bool {
                        if (p.empty()) return false;
                        size_t dot = p.find('.');
                        std::string first = dot == std::string::npos ? p : p.substr(0, dot);
                        std::string rest = dot == std::string::npos ? "" : p.substr(dot + 1);
                        if (!obj.contains(first)) return false;
                        if (rest.empty()) {
                            if (obj[first].is_string()) {
                                values.insert(obj[first].get<std::string>());
                                return true;
                            }
                            return false;
                        }
                        if (obj[first].is_object()) {
                            return getValue(obj[first], rest);
                        }
                        return false;
                    };
                getValue(doc, path);
            }

            if (values.size() > 1) {
                PolicyConflict conflict;
                conflict.tier = *resolvedTier;
                conflict.path = path;
                conflict.values = std::vector<std::string>(values.begin(), values.end());
                conflicts.push_back(conflict);
            }
        }
    }

    return conflicts;
}

std::vector<OtoiIssue> OTOIValidator::validateReservedKeys(const nlohmann::json& json) const {
    std::vector<OtoiIssue> issues;

    if (!json.contains("$otoi") || !json["$otoi"].is_string()) {
        issues.push_back({"$otoi", "Required field $otoi missing or not a string"});
    } else if (json["$otoi"].get<std::string>() != "1.0.0") {
        issues.push_back({"$otoi", "Version must be '1.0.0'"});
    }

    return issues;
}

std::vector<OtoiIssue> OTOIValidator::validateIdentity(const nlohmann::json& identity) const {
    std::vector<OtoiIssue> issues;

    if (!identity.contains("author") || !identity["author"].is_string()) {
        issues.push_back({"identity.author", "Required field author missing or not a string"});
    } else if (identity["author"].get<std::string>().empty()) {
        issues.push_back({"identity.author", "Required field author must not be empty"});
    }

    return issues;
}

std::vector<OtoiIssue> OTOIValidator::validateAgents(const nlohmann::json& agents) const {
    std::vector<OtoiIssue> issues;

    if (!agents.is_array()) {
        issues.push_back({"agents", "Field must be an array"});
        return issues;
    }

    for (size_t i = 0; i < agents.size(); ++i) {
        const auto& agent = agents[i];
        if (!agent.is_object()) {
            issues.push_back({"agents[" + std::to_string(i) + "]", "Agent must be an object"});
            continue;
        }
        if (!agent.contains("id") || !agent["id"].is_string()) {
            issues.push_back({"agents[" + std::to_string(i) + "].id", "Required field id missing or not a string"});
        }
    }

    return issues;
}

std::vector<OtoiIssue> OTOIValidator::validateEnforcement(const nlohmann::json& enforcement) const {
    std::vector<OtoiIssue> issues;

    if (!enforcement.is_object()) {
        issues.push_back({"enforcement", "Field must be an object"});
        return issues;
    }

    if (enforcement.contains("mode") && enforcement["mode"].is_string()) {
        std::string mode = enforcement["mode"].get<std::string>();
        if (mode != "advisory" && mode != "enforced" && mode != "strict") {
            issues.push_back({"enforcement.mode", "Invalid mode: " + mode});
        }
    }

    if (enforcement.contains("on_conflict") && enforcement["on_conflict"].is_string()) {
        std::string strategy = enforcement["on_conflict"].get<std::string>();
        if (strategy != "highest-tier-wins" && strategy != "reject" && strategy != "escalate") {
            issues.push_back({"enforcement.on_conflict", "Invalid on_conflict: " + strategy});
        }
    }

    if (enforcement.contains("on_unsupported") && enforcement["on_unsupported"].is_string()) {
        std::string strategy = enforcement["on_unsupported"].get<std::string>();
        if (strategy != "ignore" && strategy != "degrade" && strategy != "reject") {
            issues.push_back({"enforcement.on_unsupported", "Invalid on_unsupported: " + strategy});
        }
    }

    return issues;
}

std::vector<OtoiIssue> OTOIValidator::validateToiSources(const nlohmann::json& sources) const {
    std::vector<OtoiIssue> issues;

    if (!sources.is_array()) {
        issues.push_back({"toi_sources", "Field must be an array"});
        return issues;
    }

    for (size_t i = 0; i < sources.size(); ++i) {
        const auto& source = sources[i];
        if (!source.is_object()) {
            issues.push_back({"toi_sources[" + std::to_string(i) + "]", "Source must be an object"});
            continue;
        }

        bool hasUri = source.contains("uri") && source["uri"].is_string();
        bool hasInline = source.contains("inline") && !source["inline"].is_null();

        if (!hasUri && !hasInline) {
            issues.push_back({"toi_sources[" + std::to_string(i) + "]", "Source must have either uri or inline"});
        }

        if (hasUri && hasInline) {
            issues.push_back({"toi_sources[" + std::to_string(i) + "]", "Source must have exactly one of uri or inline"});
        }

        if (source.contains("tier") && source["tier"].is_string()) {
            std::string tier = source["tier"].get<std::string>();
            if (tier != "personal" && tier != "community" && tier != "project") {
                issues.push_back({"toi_sources[" + std::to_string(i) + "].tier", "Invalid tier: " + tier});
            }
        }
    }

    return issues;
}

ResolvedEnforcement OTOIValidator::resolveEnforcement(const OtoiEnforcement& raw) const {
    ResolvedEnforcement resolved;

    resolved.mode = raw.mode.value_or(EnforcementMode::Enforced);
    resolved.on_conflict = raw.on_conflict.value_or(ConflictStrategy::HighestTierWins);
    resolved.on_unsupported = raw.on_unsupported.value_or(UnsupportedStrategy::Degrade);
    resolved.audit = raw.audit.value_or(true);

    return resolved;
}

// ============ OTOIManager Implementation ============

OtoiCharter OTOIManager::parseCharter(const nlohmann::json& json) {
    auto result = safeParseCharter(json);
    if (!result.has_value()) {
        // Re-throw as appropriate error type
        switch (result.error().code) {
            case OtoiErrorCode::Parse:
                throw OtoiParseError(result.error().message);
            case OtoiErrorCode::Validation: {
                // Extract issues from validation error if present
                const auto& issues = result.error().issues.value_or(std::vector<OtoiIssue>{});
                throw OtoiValidationError(result.error().message, issues);
            }
            case OtoiErrorCode::Honor: {
                const auto& conflicts = result.error().conflicts.value_or(std::vector<PolicyConflict>{});
                throw OtoiHonorError(result.error().message, conflicts);
            }
        }
        throw std::runtime_error(result.error().message);
    }
    return *result;
}

tl::expected<OtoiCharter, OtoiError> OTOIManager::safeParseCharter(const nlohmann::json& json) const {
    OTOIValidator validator;
    return validator.parseAndValidate(json);
}

EffectivePolicy OTOIManager::honor(const OtoiCharter& charter, const HonorOptions& options) {
    auto result = safeHonor(charter, options);
    if (!result.has_value()) {
        throw OtoiHonorError(result.error().message, result.error().conflicts);
    }
    return *result;
}

tl::expected<EffectivePolicy, OtoiHonorError> OTOIManager::safeHonor(const OtoiCharter& charter, const HonorOptions& options) const {
    EffectivePolicy policy;

    // Check if we have sources to resolve
    if (charter.toi_sources.empty() && (!options.documents || options.documents->empty())) {
        return tl::unexpected<OtoiHonorError>(
            OtoiHonorError("No .toi sources to resolve", {})
        );
    }

    // Resolve enforcement policy
    ResolvedEnforcement resolvedEnforcement;
    if (charter.enforcement.has_value()) {
        OTOIValidator validator;
        resolvedEnforcement = validator.resolveEnforcement(*charter.enforcement);
    } else {
        resolvedEnforcement = ResolvedEnforcement{
            EnforcementMode::Enforced,
            ConflictStrategy::HighestTierWins,
            UnsupportedStrategy::Degrade,
            true
        };
    }
    policy.enforcement = resolvedEnforcement;

    // Collect .toi documents from sources and options
    std::vector<nlohmann::json> documents;

    // Load from charter sources
    for (const auto& source : charter.toi_sources) {
        try {
            nlohmann::json doc = loadSource(source, options);
            if (doc.is_object()) {
                documents.push_back(doc);
            }
        } catch (const std::exception& e) {
            if (resolvedEnforcement.on_unsupported == UnsupportedStrategy::Reject) {
                return tl::unexpected<OtoiHonorError>(
                    OtoiHonorError(std::string("Failed to load source: ") + e.what(), {})
                );
            }
            // For ignore/degrade, skip failed sources
        }
    }

    // Add additional documents from options
    if (options.documents) {
        for (const auto& doc : *options.documents) {
            if (doc.is_object()) {
                documents.push_back(doc);
            }
        }
    }

    if (documents.empty()) {
        return tl::unexpected<OtoiHonorError>(
            OtoiHonorError("No valid .toi documents to resolve", {})
        );
    }

    // Detect same-tier conflicts before resolution
    OTOIValidator validator;
    std::vector<PolicyConflict> conflicts = validator.detectConflicts(documents);
    policy.conflicts = conflicts;

    // Handle conflicts based on enforcement policy
    if (!conflicts.empty() && resolvedEnforcement.on_conflict == ConflictStrategy::Reject) {
        return tl::unexpected<OtoiHonorError>(
            OtoiHonorError("Same-tier conflicts detected under reject strategy", conflicts)
        );
    }

    // Resolve documents using tier precedence
    policy.effective = resolveDocuments(documents, resolvedEnforcement);

    // Set tiers (unique, highest precedence first)
    std::set<Tier> tierSet;
    for (const auto& doc : documents) {
        if (doc.contains("$tier") && doc["$tier"].is_string()) {
            auto tier = tier_from_string(doc["$tier"].get<std::string>());
            if (tier) tierSet.insert(*tier);
        }
    }
    // Order: personal > community > project
    if (tierSet.contains(Tier::Personal)) policy.tiers.push_back(Tier::Personal);
    if (tierSet.contains(Tier::Community)) policy.tiers.push_back(Tier::Community);
    if (tierSet.contains(Tier::Project)) policy.tiers.push_back(Tier::Project);

    // Set agents
    policy.agents = charter.agents;

    // Governance is engaged once a charter has been successfully honored.
    // Publish m_mode with a relaxed store BEFORE signaling m_active with a
    // release store, so that getStatus()'s acquire-load of m_active makes the
    // m_mode write visible. Both fields are now atomic, eliminating the data
    // race between safeHonor() writes and getStatus() reads.
    m_mode.store(resolvedEnforcement.mode, std::memory_order_relaxed);
    m_active.store(true, std::memory_order_release);

    return policy;
}

nlohmann::json OTOIManager::propagate(const EffectivePolicy& policy, const std::string& agentId) const {
    // Under strict enforcement, refuse unknown agents
    if (policy.enforcement.mode == EnforcementMode::Strict) {
        for (const auto& agent : policy.agents) {
            if (agent.id == agentId) {
                return policy.effective;
            }
        }
        // Agent not declared - return empty object or throw
        return nlohmann::json::object();
    }

    // For advisory and enforced modes, serve the effective policy
    return policy.effective;
}

OTOIManager::Status OTOIManager::getStatus() const {
    // Acquire-load m_active: synchronizes with the release-store in safeHonor().
    // Because m_mode is also atomic and written before the release store on
    // m_active, the acquire-load establishes a happens-before relationship
    // that makes the m_mode value visible here. This eliminates the data race
    // that CodeRabbit flagged under concurrent safeHonor() writes and
    // getStatus() reads.
    bool active = m_active.load(std::memory_order_acquire);
    EnforcementMode mode = m_mode.load(std::memory_order_relaxed);
    return {active, mode};
}

nlohmann::json OTOIManager::loadSource(const OtoiSource& source, const HonorOptions& options) const {
    if (source.inline_doc.has_value()) {
        return *source.inline_doc;
    }

    if (source.uri.has_value()) {
        if (!options.load_source) {
            throw std::runtime_error("No load_source callback provided for uri source: " + *source.uri);
        }
        std::string toiText = (*options.load_source)(*source.uri);
        return nlohmann::json::parse(toiText);
    }

    throw std::runtime_error("Source has neither uri nor inline");
}

nlohmann::json OTOIManager::resolveDocuments(const std::vector<nlohmann::json>& documents, const ResolvedEnforcement& enforcement) const {
    if (documents.empty()) {
        return nlohmann::json::object();
    }

    // Sort documents by tier precedence: personal > community > project.
    // Documents whose $tier cannot be resolved are dropped here (not silently
    // merged with priority 0) so that detectConflicts and resolveDocuments
    // agree: an unparseable $tier never participates in resolution.
    auto tierPriority = [](const nlohmann::json& doc) -> int {
        if (doc.contains("$tier") && doc["$tier"].is_string()) {
            auto tier = tier_from_string(doc["$tier"].get<std::string>());
            if (tier) {
                switch (*tier) {
                    case Tier::Personal: return 3;
                    case Tier::Community: return 2;
                    case Tier::Project: return 1;
                }
            }
        }
        return -1; // unresolvable — filtered out below
    };

    std::vector<nlohmann::json> sorted;
    for (const auto& doc : documents) {
        if (tierPriority(doc) >= 0) sorted.push_back(doc);
    }

    if (sorted.empty()) {
        return nlohmann::json::object();
    }

    std::sort(sorted.begin(), sorted.end(), [&](const nlohmann::json& a, const nlohmann::json& b) {
        return tierPriority(a) > tierPriority(b);
    });

    // Start with highest precedence document
    nlohmann::json resolved = sorted[0];

    // Apply gap-filling from lower precedence documents
    for (size_t i = 1; i < sorted.size(); ++i) {
        const auto& lower = sorted[i];

        // Merge objects per key recursively
        std::function<void(nlohmann::json&, const nlohmann::json&)> merge =
            [&](nlohmann::json& target, const nlohmann::json& source) {
                if (!source.is_object() || !target.is_object()) return;

                for (const auto& [key, value] : source.items()) {
                    if (key[0] == '$') continue; // Skip reserved keys

                    if (value.is_object() && target.contains(key) && target[key].is_object()) {
                        merge(target[key], value);
                    } else if (!target.contains(key)) {
                        target[key] = value;
                    }
                }
            };

        merge(resolved, lower);
    }

    return resolved;
}

nlohmann::json OTOIManager::applyEnforcement(const nlohmann::json& effective, const ResolvedEnforcement& enforcement) const {
    // Placeholder: enforcement application logic
    // In full implementation, this would handle unsupported preferences
    // based on on_unsupported strategy
    return effective;
}

bool OTOIManager::isAgentDeclared(const OtoiCharter& charter, const std::string& agentId) const {
    for (const auto& agent : charter.agents) {
        if (agent.id == agentId) {
            return true;
        }
    }
    return false;
}

} // namespace otoi
