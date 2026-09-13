#pragma once

/**
 * SleepwalkerTypes.h — Core type definitions for the Sleepwalker Protocol C++ port.
 *
 * Maps the Sleepwalker Protocol emotional-continuity governance types from the
 * original Python/TypeScript reference (`sleepwalker_protocol`) to idiomatic C++20.
 *
 * Governed by: ORG-DEV-OTOI-1.0.3
 * Reference: docs/phase-1-reference-analysis.md §5
 */

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace sleepwalker {

// ===================== StateType =====================
// Emotional state classification (from Phase 1 §5.3)
// Values: dissociation | numbing | avoidance | detachment | neutral

enum class StateType {
    Dissociation,
    Numbing,
    Avoidance,
    Detachment,
    Neutral
};

inline std::string stateTypeToString(StateType t) {
    switch (t) {
        case StateType::Dissociation:  return "dissociation";
        case StateType::Numbing:       return "numbing";
        case StateType::Avoidance:     return "avoidance";
        case StateType::Detachment:    return "detachment";
        case StateType::Neutral:       return "neutral";
    }
    return "neutral";
}

inline StateType stringToStateType(const std::string& s) {
    if (s == "dissociation")   return StateType::Dissociation;
    if (s == "numbing")        return StateType::Numbing;
    if (s == "avoidance")      return StateType::Avoidance;
    if (s == "detachment")     return StateType::Detachment;
    return StateType::Neutral;
}

// ===================== ConsentLevel =====================
// Graduated consent levels for AI intervention (from Phase 1 §5.3, consent.py)
// Values: PASSIVE | LOW_PRESSURE | SAFETY_CHECK | RRTA_HANDOFF

enum class ConsentLevel {
    Passive,
    LowPressure,
    SafetyCheck,
    RRTAHandoff
};

inline std::string consentLevelToString(ConsentLevel l) {
    switch (l) {
        case ConsentLevel::Passive:       return "PASSIVE";
        case ConsentLevel::LowPressure:   return "LOW_PRESSURE";
        case ConsentLevel::SafetyCheck:   return "SAFETY_CHECK";
        case ConsentLevel::RRTAHandoff:   return "RRTA_HANDOFF";
    }
    return "PASSIVE";
}

// ===================== Channel =====================
// Interaction arrival channel — determines trust (from Phase 1 §5.4, types.ts)
// Values: user_input | model_output | tool_result | system | unknown
// Only USER_INPUT is trusted; everything else collapses to UNKNOWN.

enum class Channel {
    UserInput,
    ModelOutput,
    ToolResult,
    System,
    Unknown
};

inline std::string channelToString(Channel c) {
    switch (c) {
        case Channel::UserInput:  return "user_input";
        case Channel::ModelOutput: return "model_output";
        case Channel::ToolResult:  return "tool_result";
        case Channel::System:      return "system";
        case Channel::Unknown:     return "unknown";
    }
    return "unknown";
}

inline Channel stringToChannel(const std::string& s) {
    if (s == "user_input")    return Channel::UserInput;
    if (s == "model_output")  return Channel::ModelOutput;
    if (s == "tool_result")   return Channel::ToolResult;
    if (s == "system")        return Channel::System;
    return Channel::Unknown;
}

/// Coerces an arbitrary string to a Channel. Exact closed-enum members pass
/// through; every malformed value collapses to Unknown (never elevates).
/// This mirrors normalizeChannel() in the TypeScript reference (types.ts).
inline Channel normalizeChannel(const std::string& value) {
    return stringToChannel(value);
}

// ===================== CrisisIndicators =====================
// Sub-indicators for crisis detection (from state_detection.py)

struct CrisisIndicators {
    bool suicidalIdeation = false;
    bool selfHarm = false;
    bool safetyConcern = false;
};

// ===================== EmotionalIndicators =====================
// Aggregated indicator flags from state detection (from state_detection.py)

struct EmotionalIndicators {
    bool dissociation = false;
    bool numbing = false;
    bool avoidance = false;
    bool detachment = false;
    CrisisIndicators crisis{};
};

// ===================== EmotionalState =====================
// Full emotional state assessment result (from state_detection.py EmotionalState)
//
// Note: channel, trusted, flagged, flagReason are NOT part of the base
// EmotionalState — they are added by the foundation integration layer as
// provenance envelope fields (mirrors Python/TS reference per Phase 1 §5.3 note).

struct EmotionalState {
    StateType stateType = StateType::Neutral;
    bool protective = false;
    bool requiresCheckIn = false;
    EmotionalIndicators indicators{};
    double confidence = 0.0;

    // Crisis flags (duplicate convenience access to indicators.crisis fields)
    bool explicitSuicidalIdeation = false;
    bool selfHarmIndicators = false;
    bool inabilityToEnsureSafety = false;
};

// ===================== SWP Configuration =====================
// SWP-specific TOI extension keys (from sleepwalker TOILoader default config).
// These live under the "swp" key in a user's TOI YAML/JSON, separate from the
// formal .toi specification. The TOILoader loads these as a nlohmann::json
// config object; this struct provides typed access for C++ consumers.

struct SwpConfig {
    bool active = true;
    std::string interventionThreshold = "user_initiated_only";
    bool processingConsent = false;
    std::vector<std::string> protectedTopics{};
};

// ===================== Continuity Context =====================
// Result of ContinuityManager::getContext() (from continuity.py get_context)

struct ContinuityContext {
    bool hasHistory = false;
    std::optional<std::string> lastSessionState;
    bool protectiveStateActive = false;
    /// declared_boundaries may be a JSON array (from session save) or a
    /// JSON object (from update_boundary). Stored as json for flexibility.
    nlohmann::json declaredBoundaries = nlohmann::json::array();
    std::optional<int> daysSinceLastSession;
    int sessionCount = 0;
};

// ===================== Assessment Result =====================
// Result of SleepwalkerProtocol::assessInteraction() (from protocol.py assess_interaction)
// Uses a flexible map to mirror the dict-based return in the reference implementation.

struct InteractionAssessment {
    EmotionalState emotionalState;
    ConsentLevel consentLevel;
    bool swpActive = true;
    bool protectiveStateActive = false;
    std::optional<ContinuityContext> continuityContext;
};

// ===================== Response Guidance =====================
// Result of SleepwalkerProtocol::generateResponse()

struct ResponseGuidance {
    std::string responseType;
    std::string guidance;
    std::string intervention;
    std::optional<ConsentLevel> level;
    std::optional<std::string> focus;
    std::optional<std::string> emotionalDemands;
    std::optional<std::string> processingPressure;
};

} // namespace sleepwalker
