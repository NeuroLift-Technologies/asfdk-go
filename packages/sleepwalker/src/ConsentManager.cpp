/**
 * ConsentManager.cpp — Implementation of graduated consent model.
 *
 * Ported from: sleepwalker_protocol/consent.py
 * Governed by: ORG-DEV-OTOI-1.0.3
 */

#include "sleepwalker/ConsentManager.h"
#include <spdlog/spdlog.h>

namespace sleepwalker {

ConsentManager::ConsentManager(const nlohmann::json& userToi)
    : m_userToi(userToi) {
    // Extract SWP config from user_TOI; default to empty if not present
    if (m_userToi.contains("swp") && m_userToi["swp"].is_object()) {
        m_swpConfig = m_userToi["swp"];
    } else {
        m_swpConfig = nlohmann::json::object();
    }
}

std::string ConsentManager::getInterventionThreshold() const {
    if (m_swpConfig.contains("intervention_threshold") &&
        m_swpConfig["intervention_threshold"].is_string()) {
        return m_swpConfig["intervention_threshold"].get<std::string>();
    }
    return "user_initiated_only";  // default per Python reference
}

ConsentLevel ConsentManager::getAppropriateLevel(const EmotionalState& state) {
    return determineLevel(state);
}

ConsentLevel ConsentManager::determineLevel(const EmotionalState& state) {
    // 1. Crisis situations warrant RRTA handoff
    if (state.explicitSuicidalIdeation ||
        state.selfHarmIndicators ||
        state.inabilityToEnsureSafety) {
        return ConsentLevel::RRTAHandoff;
    }

    // 2. If state requires check-in but no crisis
    if (state.requiresCheckIn) {
        return ConsentLevel::SafetyCheck;
    }

    // 3. For protective states, respect user's intervention preference
    if (state.protective) {
        std::string threshold = getInterventionThreshold();
        if (threshold == "offer_support_without_pressure") {
            return ConsentLevel::LowPressure;
        }
        // Default to passive for protective states
        return ConsentLevel::Passive;
    }

    // 4. No intervention needed for neutral states
    return ConsentLevel::Passive;
}

bool ConsentManager::shouldIntervene(const EmotionalState& state,
                                     std::optional<ConsentLevel> consentLevel) {
    ConsentLevel level = consentLevel.value_or(determineLevel(state));

    // Only intervene for safety checks and RRTA handoffs
    return level == ConsentLevel::SafetyCheck || level == ConsentLevel::RRTAHandoff;
}

std::string ConsentManager::getConsentMessage(ConsentLevel level) {
    // Messages ported from consent.py get_consent_message
    switch (level) {
        case ConsentLevel::Passive:
            return "I'm here if you need anything. No pressure.";
        case ConsentLevel::LowPressure:
            return "I can provide support if you'd like. Your choice.";
        case ConsentLevel::SafetyCheck:
            return "I want to check in: Are you safe right now?";
        case ConsentLevel::RRTAHandoff:
            return "I'm concerned about your safety. Can I provide crisis resources?";
    }
    return "I'm here if you need anything. No pressure.";
}

} // namespace sleepwalker
