/**
 * StateDetector.cpp — Implementation of emotional state detection.
 *
 * Pattern-matching rules ported from sleepwalker_protocol/state_detection.py.
 * All regexes are compiled case-insensitive (std::regex_constants::icase) to
 * match the Python reference's `text.lower()` normalization approach.
 *
 * Ported from: sleepwalker_protocol/state_detection.py
 * Governed by: ORG-DEV-OTOI-1.0.3
 */

#include "sleepwalker/StateDetector.h"
#include <algorithm>

namespace sleepwalker {

StateDetector::StateDetector() {
    initializePatterns();
}

void StateDetector::initializePatterns() {
    // --- Dissociation markers ---
    // Source: state_detection.py dissociation_patterns
    const std::vector<std::string> dissociation = {
        R"(\bnumb\b)",
        R"(\bdetached\b)",
        R"(\bdisconnected\b)",
        R"(\bnot really here\b)",
        R"(\bfeeling nothing\b)",
        R"(\bspaced out\b)",
        R"(\bfoggy\b)",
        R"(\bderealization\b)",
        R"(\bdepersonalization\b)"
    };
    for (const auto& p : dissociation) {
        m_dissociationPatterns.emplace_back(p, std::regex_constants::icase);
    }

    // --- Emotional numbing markers ---
    // Source: state_detection.py numbing_patterns
    const std::vector<std::string> numbing = {
        R"(\bdon't feel (much|anything)\b)",
        R"(\bemotionally flat\b)",
        R"(\bcan't feel\b)",
        R"(\bnumb to\b)",
        R"(\bshut down\b)",
        R"(\bturned off\b)"
    };
    for (const auto& p : numbing) {
        m_numbingPatterns.emplace_back(p, std::regex_constants::icase);
    }

    // --- Avoidance patterns ---
    // Source: state_detection.py avoidance_patterns
    const std::vector<std::string> avoidance = {
        R"(\bnot ready to (talk|discuss|think)\b)",
        R"(\bcan't (talk|think|discuss) (about|this)\b)",
        R"(\bavoid(ing)?\b)",
        R"(\bdon't want to go there\b)",
        R"(\bnot now\b)",
        R"(\bmaybe later\b)"
    };
    for (const auto& p : avoidance) {
        m_avoidancePatterns.emplace_back(p, std::regex_constants::icase);
    }

    // --- Protective detachment cues ---
    // Source: state_detection.py detachment_patterns
    const std::vector<std::string> detachment = {
        R"(\bjust fine\b)",
        R"(\bi'm fine\b)",
        R"(\bit's whatever\b)",
        R"(\bdoesn't matter\b)",
        R"(\bdon't care\b)",
        R"(\bit is what it is\b)"
    };
    for (const auto& p : detachment) {
        m_detachmentPatterns.emplace_back(p, std::regex_constants::icase);
    }

    // --- Crisis indicators ---
    // Source: state_detection.py crisis_patterns
    const std::vector<std::string> suicidalIdeation = {
        R"(\bsuicide\b)",
        R"(\bend(ing)? (my|it all)\b)",
        R"(\bkill myself\b)",
        R"(\bdon't want to (live|be here)\b)",
        R"(\bbetter off dead\b)"
    };
    for (const auto& p : suicidalIdeation) {
        m_suicidalIdeationPatterns.emplace_back(p, std::regex_constants::icase);
    }

    const std::vector<std::string> selfHarm = {
        R"(\bcut(ting)? myself\b)",
        R"(\bhurt(ing)? myself\b)",
        R"(\bself(-| )harm\b)",
        R"(\bburning myself\b)"
    };
    for (const auto& p : selfHarm) {
        m_selfHarmPatterns.emplace_back(p, std::regex_constants::icase);
    }

    const std::vector<std::string> safetyConcern = {
        R"(\bnot safe\b)",
        R"(\bdon't feel safe\b)",
        R"(\bcan't keep (myself )?safe\b)",
        R"(\bgonna hurt\b)",
        R"(\blose control\b)"
    };
    for (const auto& p : safetyConcern) {
        m_safetyConcernPatterns.emplace_back(p, std::regex_constants::icase);
    }
}

bool StateDetector::checkPatterns(const std::string& text,
                                  const std::vector<std::regex>& patterns) const {
    return std::any_of(patterns.begin(), patterns.end(),
                       [&](const std::regex& r) {
                           return std::regex_search(text, r);
                       });
}

CrisisIndicators StateDetector::checkCrisisIndicators(const std::string& text) const {
    CrisisIndicators crisis;
    crisis.suicidalIdeation = checkPatterns(text, m_suicidalIdeationPatterns);
    crisis.selfHarm = checkPatterns(text, m_selfHarmPatterns);
    crisis.safetyConcern = checkPatterns(text, m_safetyConcernPatterns);
    return crisis;
}

double StateDetector::calculateConfidence(const EmotionalIndicators& indicators) const {
    // Count active non-crisis indicators (matches Python reference logic):
    // 0 active → 0.0, 1 active → 0.5, 2+ active → 0.8
    int active = (indicators.dissociation ? 1 : 0) +
                 (indicators.numbing ? 1 : 0) +
                 (indicators.avoidance ? 1 : 0) +
                 (indicators.detachment ? 1 : 0);

    if (active == 0) return 0.0;
    if (active == 1) return 0.5;
    return 0.8;
}

EmotionalState StateDetector::detect(const std::string& userInput,
                                     const std::vector<std::string>& /*sessionHistory*/) {
    // sessionHistory is accepted for API parity but not used in the simplified
    // reference port (Python notes "simplified for now").

    bool dissociation = checkPatterns(userInput, m_dissociationPatterns);
    bool numbing = checkPatterns(userInput, m_numbingPatterns);
    bool avoidance = checkPatterns(userInput, m_avoidancePatterns);
    bool detachment = checkPatterns(userInput, m_detachmentPatterns);

    CrisisIndicators crisis = checkCrisisIndicators(userInput);

    EmotionalIndicators indicators{};
    indicators.dissociation = dissociation;
    indicators.numbing = numbing;
    indicators.avoidance = avoidance;
    indicators.detachment = detachment;
    indicators.crisis = crisis;

    // Protective state is active if any protective indicator is detected
    bool protective = dissociation || numbing || avoidance || detachment;

    // Check-in is warranted for any crisis indicator
    bool requiresCheckIn = crisis.suicidalIdeation || crisis.selfHarm || crisis.safetyConcern;

    // Determine primary state type (priority order matches Python reference)
    StateType stateType = StateType::Neutral;
    if (dissociation)       stateType = StateType::Dissociation;
    else if (numbing)       stateType = StateType::Numbing;
    else if (avoidance)     stateType = StateType::Avoidance;
    else if (detachment)    stateType = StateType::Detachment;

    double confidence = calculateConfidence(indicators);

    EmotionalState state;
    state.stateType = stateType;
    state.protective = protective;
    state.requiresCheckIn = requiresCheckIn;
    state.indicators = indicators;
    state.confidence = confidence;
    state.explicitSuicidalIdeation = crisis.suicidalIdeation;
    state.selfHarmIndicators = crisis.selfHarm;
    state.inabilityToEnsureSafety = crisis.safetyConcern;

    return state;
}

} // namespace sleepwalker
