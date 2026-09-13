#include "toi/TermsOfInteraction.h"
#include "toi/TOITypes.h"
#include <stdexcept>
#include <algorithm>

namespace toi {

TOIDocument parseTOI(const nlohmann::json& json) {
    if (!json.is_object()) {
        throw std::invalid_argument("TOI document must be a JSON object");
    }

    TOIDocument doc;

    // Required: $toi
    if (!json.contains("$toi") || !json["$toi"].is_string()) {
        throw std::invalid_argument("Required field $toi missing or not a string");
    }
    doc.$toi = json["$toi"].get<std::string>();
    if (doc.$toi != "1.0.0") {
        throw std::invalid_argument("Unsupported TOI version: " + doc.$toi + " (expected 1.0.0)");
    }

    // Required: $tier
    if (!json.contains("$tier") || !json["$tier"].is_string()) {
        throw std::invalid_argument("Required field $tier missing or not a string");
    }
    doc.$tier = json["$tier"].get<std::string>();
    if (doc.$tier != "personal" && doc.$tier != "community" && doc.$tier != "project") {
        throw std::invalid_argument("Invalid $tier value: " + doc.$tier);
    }

    // Required: identity
    if (!json.contains("identity") || !json["identity"].is_object()) {
        throw std::invalid_argument("Required field identity missing or not an object");
    }
    const auto& id = json["identity"];
    if (!id.contains("author") || !id["author"].is_string() || id["author"].get<std::string>().empty()) {
        throw std::invalid_argument("Required field identity.author missing or empty");
    }
    doc.identity.author = id["author"].get<std::string>();
    if (id.contains("handle") && id["handle"].is_string()) {
        doc.identity.handle = id["handle"].get<std::string>();
    }
    if (id.contains("organization") && id["organization"].is_string()) {
        doc.identity.organization = id["organization"].get<std::string>();
    }
    if (id.contains("pronouns") && id["pronouns"].is_string()) {
        doc.identity.pronouns = id["pronouns"].get<std::string>();
    }

    // Optional fields
    if (json.contains("$created") && json["$created"].is_string()) {
        doc.$created = json["$created"].get<std::string>();
    }
    if (json.contains("$updated") && json["$updated"].is_string()) {
        doc.$updated = json["$updated"].get<std::string>();
    }
    if (json.contains("$id") && json["$id"].is_string()) {
        doc.$id = json["$id"].get<std::string>();
    }
    if (json.contains("$license") && json["$license"].is_string()) {
        doc.$license = json["$license"].get<std::string>();
    }

    // Optional sections
    if (json.contains("cognitive_profile") && json["cognitive_profile"].is_object()) {
        doc.cognitive_profile = CognitiveProfile{};
        const auto& cp = json["cognitive_profile"];
        if (cp.contains("self_described") && cp["self_described"].is_string()) {
            doc.cognitive_profile->self_described = cp["self_described"].get<std::string>();
        }
        if (cp.contains("processing_style") && cp["processing_style"].is_string()) {
            const std::string ps = cp["processing_style"].get<std::string>();
            if (ps == "sequential") doc.cognitive_profile->processing_style = ProcessingStyle::Sequential;
            else if (ps == "parallel") doc.cognitive_profile->processing_style = ProcessingStyle::Parallel;
            else if (ps == "associative") doc.cognitive_profile->processing_style = ProcessingStyle::Associative;
            else if (ps == "variable") doc.cognitive_profile->processing_style = ProcessingStyle::Variable;
        }
        if (cp.contains("attention_model") && cp["attention_model"].is_string()) {
            const std::string am = cp["attention_model"].get<std::string>();
            if (am == "sustained") doc.cognitive_profile->attention_model = AttentionModel::Sustained;
            else if (am == "short-bursts") doc.cognitive_profile->attention_model = AttentionModel::ShortBursts;
            else if (am == "hyperfocus-prone") doc.cognitive_profile->attention_model = AttentionModel::HyperfocusProne;
            else if (am == "variable") doc.cognitive_profile->attention_model = AttentionModel::Variable;
        }
        if (cp.contains("scaffolding_preference") && cp["scaffolding_preference"].is_string()) {
            const std::string sp = cp["scaffolding_preference"].get<std::string>();
            if (sp == "minimal") doc.cognitive_profile->scaffolding_preference = ScaffoldingPreference::Minimal;
            else if (sp == "moderate") doc.cognitive_profile->scaffolding_preference = ScaffoldingPreference::Moderate;
            else if (sp == "extensive") doc.cognitive_profile->scaffolding_preference = ScaffoldingPreference::Extensive;
            else if (sp == "step-by-step") doc.cognitive_profile->scaffolding_preference = ScaffoldingPreference::StepByStep;
        }
        if (cp.contains("energy_model") && cp["energy_model"].is_string()) {
            const std::string em = cp["energy_model"].get<std::string>();
            if (em == "steady") doc.cognitive_profile->energy_model = EnergyModel::Steady;
            else if (em == "variable") doc.cognitive_profile->energy_model = EnergyModel::Variable;
            else if (em == "spoon-limited") doc.cognitive_profile->energy_model = EnergyModel::SpoonLimited;
            else if (em == "burst") doc.cognitive_profile->energy_model = EnergyModel::Burst;
        }
        if (cp.contains("thread_support") && cp["thread_support"].is_boolean()) {
            doc.cognitive_profile->thread_support = cp["thread_support"].get<bool>();
        }
        if (cp.contains("hyperfocus_protection") && cp["hyperfocus_protection"].is_boolean()) {
            doc.cognitive_profile->hyperfocus_protection = cp["hyperfocus_protection"].get<bool>();
        }
        if (cp.contains("executive_function_support") && cp["executive_function_support"].is_boolean()) {
            doc.cognitive_profile->executive_function_support = cp["executive_function_support"].get<bool>();
        }
    }

    if (json.contains("privacy") && json["privacy"].is_object()) {
        doc.privacy = Privacy{};
        const auto& p = json["privacy"];
        doc.privacy = Privacy{}; // engage optional
        if (p.contains("retention") && p["retention"].is_string()) {
            doc.privacy->retention = p["retention"].get<std::string>();
        }
        if (p.contains("cross_platform_sharing") && p["cross_platform_sharing"].is_string()) {
            doc.privacy->cross_platform_sharing = p["cross_platform_sharing"].get<std::string>();
        }
        if (p.contains("training_use") && p["training_use"].is_string()) {
            doc.privacy->training_use = p["training_use"].get<std::string>();
        }
        if (p.contains("analytics") && p["analytics"].is_string()) {
            doc.privacy->analytics = p["analytics"].get<std::string>();
        }
        if (p.contains("override_rights") && p["override_rights"].is_string()) {
            doc.privacy->override_rights = p["override_rights"].get<std::string>();
        }
        if (p.contains("data_requests") && p["data_requests"].is_string()) {
            doc.privacy->data_requests = p["data_requests"].get<std::string>();
        }
    }

    if (json.contains("agency") && json["agency"].is_object()) {
        doc.agency = Agency{};
        const auto& a = json["agency"];
        if (a.contains("task_initiation") && a["task_initiation"].is_string()) {
            doc.agency->task_initiation = a["task_initiation"].get<std::string>();
        }
        if (a.contains("ai_suggestions") && a["ai_suggestions"].is_string()) {
            doc.agency->ai_suggestions = a["ai_suggestions"].get<std::string>();
        }
        if (a.contains("interruptibility") && a["interruptibility"].is_string()) {
            doc.agency->interruptibility = a["interruptibility"].get<std::string>();
        }
        if (a.contains("action_confirmation") && a["action_confirmation"].is_string()) {
            doc.agency->action_confirmation = a["action_confirmation"].get<std::string>();
        }
        if (a.contains("override_authority") && a["override_authority"].is_string()) {
            doc.agency->override_authority = a["override_authority"].get<std::string>();
        }
    }

    if (json.contains("communication") && json["communication"].is_object()) {
        doc.communication = Communication{};
        const auto& c = json["communication"];
        if (c.contains("tone") && c["tone"].is_string()) {
            const std::string t = c["tone"].get<std::string>();
            if (t == "formal") doc.communication->tone = Tone::Formal;
            else if (t == "casual") doc.communication->tone = Tone::Casual;
            else if (t == "professional") doc.communication->tone = Tone::Professional;
            else if (t == "friendly") doc.communication->tone = Tone::Friendly;
            else if (t == "direct") doc.communication->tone = Tone::Direct;
            else if (t == "adaptive") doc.communication->tone = Tone::Adaptive;
        }
        if (c.contains("verbosity") && c["verbosity"].is_string()) {
            const std::string v = c["verbosity"].get<std::string>();
            if (v == "minimal") doc.communication->verbosity = Verbosity::Minimal;
            else if (v == "concise") doc.communication->verbosity = Verbosity::Concise;
            else if (v == "detailed") doc.communication->verbosity = Verbosity::Detailed;
            else if (v == "comprehensive") doc.communication->verbosity = Verbosity::Comprehensive;
            else if (v == "adaptive") doc.communication->verbosity = Verbosity::Adaptive;
        }
        if (c.contains("structure") && c["structure"].is_string()) {
            const std::string s = c["structure"].get<std::string>();
            if (s == "linear") doc.communication->structure = Structure::Linear;
            else if (s == "hierarchical") doc.communication->structure = Structure::Hierarchical;
            else if (s == "visual") doc.communication->structure = Structure::Visual;
            else if (s == "bullet-points") doc.communication->structure = Structure::BulletPoints;
            else if (s == "narrative") doc.communication->structure = Structure::Narrative;
        }
        if (c.contains("language") && c["language"].is_string()) {
            doc.communication->language = c["language"].get<std::string>();
        }
        if (c.contains("jargon_tolerance") && c["jargon_tolerance"].is_string()) {
            const std::string jt = c["jargon_tolerance"].get<std::string>();
            if (jt == "none") doc.communication->jargon_tolerance = JargonTolerance::None;
            else if (jt == "low") doc.communication->jargon_tolerance = JargonTolerance::Low;
            else if (jt == "moderate") doc.communication->jargon_tolerance = JargonTolerance::Moderate;
            else if (jt == "high") doc.communication->jargon_tolerance = JargonTolerance::High;
        }
        if (c.contains("pattern_highlighting") && c["pattern_highlighting"].is_boolean()) {
            doc.communication->pattern_highlighting = c["pattern_highlighting"].get<bool>();
        }
        if (c.contains("summary_on_return") && c["summary_on_return"].is_boolean()) {
            doc.communication->summary_on_return = c["summary_on_return"].get<bool>();
        }
        if (c.contains("thread_reconnection") && c["thread_reconnection"].is_string()) {
            const std::string tr = c["thread_reconnection"].get<std::string>();
            if (tr == "none") doc.communication->thread_reconnection = ThreadReconnection::None;
            else if (tr == "brief-summary") doc.communication->thread_reconnection = ThreadReconnection::BriefSummary;
            else if (tr == "full-context") doc.communication->thread_reconnection = ThreadReconnection::FullContext;
        }
    }

    if (json.contains("ethical_pillars") && json["ethical_pillars"].is_array()) {
        for (const auto& ep : json["ethical_pillars"]) {
            if (ep.is_string()) {
                doc.ethical_pillars.push_back(ep.get<std::string>());
            }
        }
    }

    if (json.contains("custom") && json["custom"].is_object()) {
        doc.custom = json["custom"].get<std::unordered_map<std::string, std::string>>();
    }

    return doc;
}

bool validateTOI(const TOIDocument& doc) {
    // $toi must be "1.0.0"
    if (doc.$toi != "1.0.0") return false;

    // $tier must be personal, community, or project
    if (doc.$tier != "personal" && doc.$tier != "community" && doc.$tier != "project") return false;

    // identity must have non-empty author
    if (doc.identity.author.empty()) return false;

    return true;
}

TOIDocument resolveTOI(std::vector<TOIDocument> docs) {
    if (docs.empty()) {
        TOIDocument empty;
        empty.$toi = "1.0.0";
        empty.$tier = "project"; // default tier
        return empty;
    }

    // Start with the first document (highest precedence)
    TOIDocument resolved = docs[0];

    // Apply gap-filling from lower-precedence documents
    for (size_t i = 1; i < docs.size(); ++i) {
        const auto& lower = docs[i];

        // Tier precedence: personal > community > project
        // If current doc has a lower tier than the new doc, replace
        auto tierPriority = [](const std::string& t) -> int {
            if (t == "personal") return 3;
            if (t == "community") return 2;
            return 1; // project
        };

        int currentTier = tierPriority(resolved.$tier);
        int newTier = tierPriority(lower.$tier);

        // If the new document has higher or equal tier, replace
        if (newTier >= currentTier) {
            resolved = lower;
        } else {
            // Gap-fill: merge objects per key, lower tier fills missing keys
            // This is a simplified merge - in full implementation, we'd do recursive object merging
            // For now, if a field is empty in the higher tier, fill from lower tier
            if (resolved.$toi.empty() && !lower.$toi.empty()) resolved.$toi = lower.$toi;
            if (resolved.$tier.empty() && !lower.$tier.empty()) resolved.$tier = lower.$tier;
            if (resolved.identity.author.empty() && !lower.identity.author.empty()) resolved.identity.author = lower.identity.author;
        }
    }

    return resolved;
}

std::string canonicalize(const std::string& json) {
    // TODO: Full JCS (RFC 8785) implementation
    // - Sort object keys by UTF-16 code units recursively
    // - Remove insignificant whitespace
    // - Minimal string escaping
    // - ECMAScript Number-to-string form
    // - Preserve array order
    // For now, return the input unchanged with a note
    // In production, integrate a JCS library or implement fully
    return json; // Placeholder: full JCS implementation needed
}

bool verifySignature(const std::string& canonicalBytes,
                     const nlohmann::json& signature,
                     const std::vector<uint8_t>& publicKey) {
    // TODO: Full Ed25519 signature verification
    // - Check alg is "ed25519"
    // - Decode public_key and value from base64url
    // - Verify Ed25519 signature over canonicalBytes with decoded public key
    // For now, return false (full implementation needed)
    (void)canonicalBytes;
    (void)signature;
    (void)publicKey;
    return false;
}

// Non-throwing variant: safeParseTOI
// Returns tl::expected<TOIDocument, TOIError>
// On success: returns the parsed TOIDocument
// On failure: returns tl::unexpected<TOIError> with error details
tl::expected<TOIDocument, TOIError> safeParseTOI(const nlohmann::json& json) {
    try {
        auto doc = ::toi::parseTOI(json);
        return doc; // success: return the document
    } catch (const std::invalid_argument& e) {
        TOIError error;
        const std::string msg = e.what();
        // Map exception messages to specific error codes
        if (msg.find("Unsupported TOI version") != std::string::npos) {
                    error.code = TOIError::Code::VersionMismatch;
        } else if (msg.find("$tier") != std::string::npos) {
                    error.code = TOIError::Code::InvalidTier;
        } else if (msg.find("identity.author") != std::string::npos) {
                    error.code = TOIError::Code::MissingAuthor;
        } else if (msg.find("cognitive_profile") != std::string::npos) {
                    error.code = TOIError::Code::InvalidCognitiveProfile;
        } else if (msg.find("privacy") != std::string::npos) {
                    error.code = TOIError::Code::InvalidPrivacy;
        } else if (msg.find("agency") != std::string::npos) {
                    error.code = TOIError::Code::InvalidAgency;
        } else if (msg.find("communication") != std::string::npos) {
                    error.code = TOIError::Code::InvalidCommunication;
        } else {
                    error.code = TOIError::Code::UnknownError;
        }
        error.message = e.what();
        return tl::unexpected<TOIError>(error);
    } catch (...) {
        TOIError error;
        error.code = TOIError::Code::UnknownError;
        error.message = "Unknown exception during TOI parsing";
        return tl::unexpected<TOIError>(error);
    }
}

} // namespace toi