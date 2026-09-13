#pragma once

/**
 * ConsentManager.h — Graduated consent model for AI interventions.
 *
 * Determines appropriate intervention consent levels based on detected
 * emotional state and user TOI preferences. Consent levels progress from
 * PASSIVE (no prompting) to RRTA_HANDOFF (crisis intervention).
 *
 * Ported from: sleepwalker_protocol/consent.py
 * Governed by: ORG-DEV-OTOI-1.0.3
 * Reference: docs/phase-1-reference-analysis.md §5.3
 */

#include "SleepwalkerTypes.h"
#include <nlohmann/json.hpp>
#include <string>

namespace sleepwalker {

class ConsentManager {
public:
    /**
     * Construct a ConsentManager from a user TOI config.
     *
     * @param userToi User TOI / SWP config (extracted from YAML/JSON). The
     *        "swp" object is read for SWP-specific preferences; if absent,
     *        privacy-first defaults are used (matching the Python reference).
     */
    explicit ConsentManager(const nlohmann::json& userToi);

    /**
     * Get the appropriate consent level for the current emotional state.
     * Alias for determineLevel(), matching the Python reference API.
     */
    ConsentLevel getAppropriateLevel(const EmotionalState& state);

    /**
     * Determine appropriate intervention level based on state and user
     * preferences.
     *
     * Logic (ported from consent.py determine_level):
     *   1. Crisis indicators → RRTA_HANDOFF
     *   2. requires_check_in → SAFETY_CHECK
     *   3. Protective state → PASSIVE or LOW_PRESSURE (per intervention_threshold)
     *   4. Neutral → PASSIVE
     */
    ConsentLevel determineLevel(const EmotionalState& state);

    /**
     * Determine whether AI should intervene at all.
     * Only SAFETY_CHECK and RRTA_HANDOFF levels warrant intervention.
     */
    bool shouldIntervene(const EmotionalState& state,
                         std::optional<ConsentLevel> consentLevel = std::nullopt);

    /**
     * Get the appropriate consent message for a given level.
     * These messages mirror the graduated consent language from the
     * Python/TypeScript reference implementations.
     */
    std::string getConsentMessage(ConsentLevel level);

private:
    nlohmann::json m_userToi;
    nlohmann::json m_swpConfig;

    std::string getInterventionThreshold() const;
};

} // namespace sleepwalker
