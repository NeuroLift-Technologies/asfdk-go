#pragma once

/**
 * StateDetector.h — Emotional state detection via regex pattern matching.
 *
 * Detects protective psychological states (dissociation, numbing, avoidance,
 * detachment) and crisis indicators (suicidal ideation, self-harm, safety
 * concern) from free-text input. Detection is non-interventionist: it only
 * *flags* indicators, never *acts* on them.
 *
 * Ported from: sleepwalker_protocol/state_detection.py
 * Governed by: ORG-DEV-OTOI-1.0.3
 * Reference: docs/phase-1-reference-analysis.md §5.2, §5.5
 */

#include "SleepwalkerTypes.h"
#include <regex>
#include <vector>
#include <string>

namespace sleepwalker {

class StateDetector {
public:
    StateDetector();

    /**
     * Detect emotional state from current input and session history.
     *
     * @param userInput Current user input text
     * @param sessionHistory Optional list of previous interactions (unused in
     *        simplified port — accepted for API parity with reference)
     * @return EmotionalState populated with detection results
     */
    EmotionalState detect(const std::string& userInput,
                          const std::vector<std::string>& sessionHistory = {});

private:
    // Pattern lists (case-insensitive, compiled once in constructor)
    std::vector<std::regex> m_dissociationPatterns;
    std::vector<std::regex> m_numbingPatterns;
    std::vector<std::regex> m_avoidancePatterns;
    std::vector<std::regex> m_detachmentPatterns;

    // Crisis pattern groups
    std::vector<std::regex> m_suicidalIdeationPatterns;
    std::vector<std::regex> m_selfHarmPatterns;
    std::vector<std::regex> m_safetyConcernPatterns;

    void initializePatterns();

    bool checkPatterns(const std::string& text, const std::vector<std::regex>& patterns) const;

    CrisisIndicators checkCrisisIndicators(const std::string& text) const;
    double calculateConfidence(const EmotionalIndicators& indicators) const;
};

} // namespace sleepwalker
