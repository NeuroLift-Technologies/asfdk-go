#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <cstdint>

namespace toi {

// Forward declarations
struct CognitiveProfile;

// Enum classes mapping from TOI Zod schema v1.0.0

// processing_style: sequential | parallel | associative | variable
enum class ProcessingStyle {
    Sequential,
    Parallel,
    Associative,
    Variable
};

// attention_model: sustained | short-bursts | hyperfocus-prone | variable
enum class AttentionModel {
    Sustained,
    ShortBursts,
    HyperfocusProne,
    Variable
};

// scaffolding_preference: minimal | moderate | extensive | step-by-step
enum class ScaffoldingPreference {
    Minimal,
    Moderate,
    Extensive,
    StepByStep
};

// energy_model: steady | variable | spoon-limited | burst
enum class EnergyModel {
    Steady,
    Variable,
    SpoonLimited,
    Burst
};

// tone: formal | casual | professional | friendly | direct | adaptive
enum class Tone {
    Formal,
    Casual,
    Professional,
    Friendly,
    Direct,
    Adaptive
};

// verbosity: minimal | concise | detailed | comprehensive | adaptive
enum class Verbosity {
    Minimal,
    Concise,
    Detailed,
    Comprehensive,
    Adaptive
};

// structure: linear | hierarchical | visual | bullet-points | narrative
enum class Structure {
    Linear,
    Hierarchical,
    Visual,
    BulletPoints,
    Narrative
};

// jargon_tolerance: none | low | moderate | high
enum class JargonTolerance {
    None,
    Low,
    Moderate,
    High
};

// thread_reconnection: none | brief-summary | full-context
enum class ThreadReconnection {
    None,
    BriefSummary,
    FullContext
};

// Boolean fields from TOI cognitive_profile and privacy sections

using ThreadSupport = bool;
using HyperfocusProtection = bool;
using ExecutiveFunctionSupport = bool;

using Retention = std::string;                       // session-only | short-term | long-term | permanent | user-controlled
using CrossPlatformSharing = std::string;            // never | explicit-only | aggregate-only | research-approved
using TrainingUse = std::string;                     // prohibited | explicit-only | anonymized-only | permitted
using Analytics = std::string;                       // prohibited | opt-in | anonymized-only | permitted
using OverrideRights = std::string;                  // user-only | delegated | admin-allowed
using DataRequests = std::string;                    // honored-immediately | honored-on-request | not-supported

// Agency enum values (free-form strings matching TOI schema)

using TaskInitiation = std::string;                // user-initiated | ai-may-suggest | ai-may-initiate
using AISuggestions = std::string;                 // none | on-request | proactive
using Interruptibility = std::string;              // never | urgent-only | always
using ActionConfirmation = std::string;            // always | destructive-only | never
using OverrideAuthority = std::string;             // user-final | shared | ai-advisory

// Communication enum values

using Language = std::string;                        // BCP 47 tag such as "en"

using JargonToleranceComm = JargonTolerance;         // none | low | moderate | high
using PatternHighlighting = bool;
using SummaryOnReturn = bool;

// ============ CognitiveProfile ============

struct CognitiveProfile {
    std::string self_described;
    std::optional<ProcessingStyle> processing_style;
    std::optional<AttentionModel> attention_model;
    std::optional<ScaffoldingPreference> scaffolding_preference;
    std::optional<EnergyModel> energy_model;
    ThreadSupport thread_support;              // boolean
    HyperfocusProtection hyperfocus_protection; // boolean
    ExecutiveFunctionSupport executive_function_support; // boolean
};

// ============ Identity ============

struct Identity {
    std::string author;             // required
    std::string handle;             // optional
    std::string organization;       // optional
    std::string pronouns;           // optional
};

// ============ Privacy ============

struct Privacy {
    Retention retention;            // session-only | short-term | long-term | permanent | user-controlled
    CrossPlatformSharing cross_platform_sharing; // never | explicit-only | aggregate-only | research-approved
    TrainingUse training_use;       // prohibited | explicit-only | anonymized-only | permitted
    Analytics analytics;            // prohibited | opt-in | anonymized-only | permitted
    OverrideRights override_rights; // user-only | delegated | admin-allowed
    DataRequests data_requests;     // honored-immediately | honored-on-request | not-supported
};

// ============ Agency ============

struct Agency {
    TaskInitiation task_initiation;     // user-initiated | ai-may-suggest | ai-may-initiate
    AISuggestions ai_suggestions;       // none | on-request | proactive
    Interruptibility interruptibility;  // never | urgent-only | always
    ActionConfirmation action_confirmation; // always | destructive-only | never
    OverrideAuthority override_authority; // user-final | shared | ai-advisory
};

// ============ Communication ============

struct Communication {
    Tone tone;                        // formal | casual | professional | friendly | direct | adaptive
    Verbosity verbosity;              // minimal | concise | detailed | comprehensive | adaptive
    Structure structure;              // linear | hierarchical | visual | bullet-points | narrative
    Language language;                // BCP 47 tag such as "en"
    JargonToleranceComm jargon_tolerance; // none | low | moderate | high
    PatternHighlighting pattern_highlighting; // boolean
    SummaryOnReturn summary_on_return; // boolean
    ThreadReconnection thread_reconnection; // none | brief-summary | full-context
};

// ============ Full TOI document structure (minimal conforming document) ============

struct TOIDocument {
    std::string $toi;               // "1.0.0" - required
    std::string $tier;              // personal | community | project - required
    std::string $created;           // ISO 8601 optional
    std::string $updated;           // ISO 8601 optional
    std::string $id;                // UUID v4 optional
    std::string $license;           // SPDX identifier optional
    Identity identity;              // required
    std::optional<CognitiveProfile> cognitive_profile;
    std::optional<Privacy> privacy;
    std::optional<Agency> agency;
    std::optional<Communication> communication;
    std::vector<std::string> ethical_pillars; // free-form string array
    std::optional<std::unordered_map<std::string, std::string>> custom; // author-defined data
};

// ============ Canonicalization support ============

// Sort object keys by UTF-16 code units (JCS RFC 8785 compatible)
// Implemented in TOITypes.cpp
std::string canonicalize(const std::string& json);

} // namespace toi