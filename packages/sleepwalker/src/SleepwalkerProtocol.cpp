/**
 * SleepwalkerProtocol.cpp — Implementation of the main Sleepwalker Protocol.
 *
 * Ported from: sleepwalker_protocol/protocol.py (SWP class)
 * Integration reference: asfdk integration layer (sleepwalker.ts)
 * Governed by: ORG-DEV-OTOI-1.0.3
 */

#include "sleepwalker/SleepwalkerProtocol.h"
#include <spdlog/spdlog.h>

namespace sleepwalker {

SleepwalkerProtocol::SleepwalkerProtocol()
    : SleepwalkerProtocol(Options{}) {
}

SleepwalkerProtocol::SleepwalkerProtocol(const SleepwalkerProtocol::Options& options)
    : m_consentManager(options.userToi)
    , m_continuityManager(options.storagePath)
    , m_userToi(options.userToi)
    , m_privacyMode(options.privacyMode)
    , m_loggingEnabled(options.loggingEnabled) {
    // Initialize instance-level logger (no global state mutation per DECISIONS.md §7)
    if (m_loggingEnabled) {
        spdlog::set_level(spdlog::level::info);
    }

    if (m_loggingEnabled) {
        spdlog::info("Sleepwalker Protocol initialized");
    }
}

EmotionalState SleepwalkerProtocol::detectEmotionalState(
    const std::string& userInput,
    const std::vector<std::string>& sessionHistory
) {
    // Detect emotional state indicators
    EmotionalState state = m_stateDetector.detect(userInput, sessionHistory);

    // Log observation without intervening
    logObservation(state, /*intervention=*/false);

    return state;
}

InteractionAssessment SleepwalkerProtocol::assessInteraction(
    const std::string& userInput,
    const std::vector<std::string>& sessionHistory,
    const std::optional<std::string>& userId
) {
    // Detect emotional state
    EmotionalState emotionalState = detectEmotionalState(userInput, sessionHistory);

    // Check user's consent preferences
    ConsentLevel consentLevel = m_consentManager.getAppropriateLevel(emotionalState);

    // Build assessment result
    InteractionAssessment assessment;
    assessment.emotionalState = emotionalState;
    assessment.consentLevel = consentLevel;
    assessment.swpActive = isSwpActive();
    assessment.protectiveStateActive = emotionalState.protective;

    // Optionally retrieve continuity context
    if (userId) {
        assessment.continuityContext = m_continuityManager.getContext(*userId);
    }

    return assessment;
}

ResponseGuidance SleepwalkerProtocol::generateResponse(
    const std::string& userInput,
    std::optional<EmotionalState> detectedState
) {
    // Detect state if not provided
    if (!detectedState) {
        detectedState = detectEmotionalState(userInput);
    }

    const EmotionalState& state = *detectedState;

    // Determine appropriate intervention level
    ConsentLevel level = determineAppropriateLevel(state);

    // Check if SWP is active and user is in protective state
    if (isSwpActive() && state.protective) {
        return stableLowDemandResponse(
            "task_support",
            "minimal",
            "none"
        );
    }

    // Check if state requires graduated consent offer
    if (state.requiresCheckIn) {
        return graduatedConsentOffer(level);
    }

    // Default: neutral, supportive response
    ResponseGuidance guidance;
    guidance.responseType = "neutral";
    guidance.guidance = "Provide task-focused support without emotional demands";
    guidance.intervention = "none";
    return guidance;
}

ConsentLevel SleepwalkerProtocol::determineAppropriateLevel(const EmotionalState& state) {
    return m_consentManager.determineLevel(state);
}

bool SleepwalkerProtocol::requiresRrtaHandoff(const EmotionalState& state) {
    // Check for explicit safety risk indicators (matches Python reference)
    return state.explicitSuicidalIdeation ||
           state.selfHarmIndicators ||
           state.inabilityToEnsureSafety;
}

nlohmann::json SleepwalkerProtocol::getContext() {
    nlohmann::json context;
    context["swp_active"] = isSwpActive();

    // user_boundaries — protected_topics from SWP config
    nlohmann::json swp = m_userToi.value("swp", nlohmann::json::object());
    if (swp.contains("protected_topics")) {
        context["user_boundaries"] = swp["protected_topics"];
    } else {
        context["user_boundaries"] = nlohmann::json::array();
    }

    // consent_preferences — intervention_preference from SWP config
    // Note: the Python reference reads "intervention_preference" here but
    // ConsentManager reads "intervention_threshold". Default differs:
    //   get_context default: "offer_support_without_pressure"
    //   ConsentManager default: "user_initiated_only"
    if (swp.contains("intervention_preference")) {
        context["consent_preferences"] = swp["intervention_preference"];
    } else {
        context["consent_preferences"] = "offer_support_without_pressure";
    }

    return context;
}

void SleepwalkerProtocol::maintainContinuity(const std::string& userId,
                                             nlohmann::json sessionData) {
    m_continuityManager.saveSession(userId, std::move(sessionData));
}

nlohmann::json SleepwalkerProtocol::getStatus() {
    nlohmann::json status;
    status["active"] = true;
    status["mode"] = "emotional-continuity";
    return status;
}

void SleepwalkerProtocol::reset() {
    // This static method is provided for API parity with the reference.
    // The integration layer is responsible for resetting/recreating the instance.
    // Note: This is NOT a singleton — each SleepwalkerProtocol instance manages
    // its own state. This method clears any static/shared state if present.
}

bool SleepwalkerProtocol::isSwpActive() {
    // Check if SWP is active in user's TOI (default: true)
    // Matches Python: self.user_toi.get('swp', {}).get('active', True)
    if (!m_userToi.contains("swp")) return true;
    if (!m_userToi["swp"].is_object()) return true;
    return m_userToi["swp"].value("active", true);
}

ResponseGuidance SleepwalkerProtocol::stableLowDemandResponse(
    const std::string& focus,
    const std::string& emotionalDemands,
    const std::string& processingPressure
) {
    ResponseGuidance guidance;
    guidance.responseType = "stable_low_demand";
    guidance.focus = focus;
    guidance.emotionalDemands = emotionalDemands;
    guidance.processingPressure = processingPressure;
    guidance.guidance = "Maintain stable, task-focused interaction without emotional prompts";
    guidance.intervention = "none";
    return guidance;
}

ResponseGuidance SleepwalkerProtocol::graduatedConsentOffer(ConsentLevel level) {
    // Messages ported from sleepwalker_protocol/protocol.py _graduated_consent_offer
    // These are more detailed/conversational than ConsentManager::getConsentMessage
    std::string message;
    switch (level) {
        case ConsentLevel::Passive:
            message = "I'm here if you need anything. No pressure.";
            break;
        case ConsentLevel::LowPressure:
            message = "I noticed you might need support. I can help if you'd like, or we can continue with [current task]. Your choice.";
            break;
        case ConsentLevel::SafetyCheck:
            message = "I want to check in: Are you safe right now? You can answer yes/no, or we can keep working. If you need different support, let me know.";
            break;
        case ConsentLevel::RRTAHandoff:
            message = "I'm concerned about your safety. I'd like to connect you with crisis support. Can I provide crisis resources?";
            break;
    }

    ResponseGuidance guidance;
    guidance.responseType = "consent_offer";
    guidance.level = level;
    guidance.guidance = message;
    guidance.intervention = "consent_required";
    return guidance;
}

void SleepwalkerProtocol::logObservation(const EmotionalState& state, bool intervention) {
    if (m_loggingEnabled) {
        spdlog::info("SWP Observation - State: {}, Protective: {}, Intervention: {}",
                     stateTypeToString(state.stateType),
                     state.protective ? "true" : "false",
                     intervention ? "true" : "false");
    }
}

} // namespace sleepwalker
