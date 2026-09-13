#pragma once

/**
 * SleepwalkerProtocol.h — Main Sleepwalker Protocol orchestrator.
 *
 * Composes StateDetector, ConsentManager, and ContinuityManager to provide
 * emotional-continuity governance for AI systems. Detects protective
 * psychological states, manages graduated consent, and preserves temporal
 * continuity across sessions.
 *
 * Ported from: sleepwalker_protocol/protocol.py (SWP class)
 * Integration reference: asfdk integration layer (sleepwalker.ts / sleepwalker.py)
 * Governed by: ORG-DEV-OTOI-1.0.3
 * Reference: docs/phase-1-reference-analysis.md §5
 */

#include "SleepwalkerTypes.h"
#include "StateDetector.h"
#include "ConsentManager.h"
#include "ContinuityManager.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <optional>

namespace sleepwalker {

class SleepwalkerProtocol {
public:
    /**
     * Construction options for SleepwalkerProtocol.
     * Mirrors SWPOptions in the TypeScript reference.
     */
    struct Options {
        nlohmann::json userToi = nlohmann::json::object();
        std::string privacyMode = "local_only";
        bool loggingEnabled = true;
        std::string storagePath = ".swp_storage";
    };

    SleepwalkerProtocol();
    explicit SleepwalkerProtocol(const Options& options);

    /**
     * Detect emotional state from user input.
     * Monitors for protective psychological states without intervention.
     *
     * @param userInput Current user input text
     * @param sessionHistory Optional list of previous interactions
     * @return EmotionalState with detected indicators
     */
    EmotionalState detectEmotionalState(
        const std::string& userInput,
        const std::vector<std::string>& sessionHistory = {}
    );

    /**
     * Assess current interaction context including emotional state, consent
     * level, and (optionally) continuity context.
     *
     * @param userInput Current user input text
     * @param sessionHistory Optional list of previous interactions
     * @param userId Optional user ID for continuity context lookup
     * @return InteractionAssessment with full assessment
     */
    InteractionAssessment assessInteraction(
        const std::string& userInput,
        const std::vector<std::string>& sessionHistory = {},
        const std::optional<std::string>& userId = std::nullopt
    );

    /**
     * Generate response guidance that respects user's protective state
     * and TOI boundaries.
     *
     * @param userInput Current user input text
     * @param detectedState Previously detected emotional state (optional;
     *        auto-detected if not provided)
     * @return ResponseGuidance with response type, guidance, and intervention
     */
    ResponseGuidance generateResponse(
        const std::string& userInput,
        std::optional<EmotionalState> detectedState = std::nullopt
    );

    /**
     * Determine appropriate consent/intervention level for the given emotional
     * state. Delegates to ConsentManager.
     */
    ConsentLevel determineAppropriateLevel(const EmotionalState& state);

    /**
     * Determine if the situation requires RRT Advocate handoff.
     * Returns true when any crisis indicator is present.
     */
    bool requiresRrtaHandoff(const EmotionalState& state);

    /**
     * Get current SWP context for sharing with other systems.
     * Returns a JSON object with swp_active, user_boundaries, consent_preferences.
     */
    nlohmann::json getContext();

    /**
     * Preserve emotional boundaries across sessions.
     * Delegates to ContinuityManager::saveSession.
     */
    void maintainContinuity(const std::string& userId, nlohmann::json sessionData);

    /**
     * Get SWP status (for foundation integration).
     * Mirrors get_status() in the Python/TypeScript integration layer.
     */
    static nlohmann::json getStatus();

    /**
     * Reset SWP state. Call during shutdown to clear state.
     * Note: SleepwalkerProtocol is NOT a singleton — each instance manages
     * its own state. This method is provided for API parity with the reference.
     * The integration layer should recreate the instance for a full reset.
     */
    static void reset();

private:
    StateDetector m_stateDetector;
    ConsentManager m_consentManager;
    ContinuityManager m_continuityManager;
    nlohmann::json m_userToi;
    std::string m_privacyMode;
    // Logger removed — using spdlog::info/warn/error directly (shim-compatible)
    bool m_loggingEnabled;

    /// Check if SWP is active in user's TOI (default: true)
    bool isSwpActive();

    /// Generate guidance for stable, low-demand response
    ResponseGuidance stableLowDemandResponse(
        const std::string& focus,
        const std::string& emotionalDemands,
        const std::string& processingPressure
    );

    /// Generate graduated consent offer based on level
    ResponseGuidance graduatedConsentOffer(ConsentLevel level);

    /// Log state observation for monitoring purposes
    void logObservation(const EmotionalState& state, bool intervention = false);
};

} // namespace sleepwalker
