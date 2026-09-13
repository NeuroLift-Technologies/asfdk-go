#pragma once

/**
 * ContinuityManager.h — Temporal continuity management.
 *
 * Preserves emotional state and boundary awareness across sessions. Session
 * data is stored as JSON files on disk (one per user), enabling state
 * persistence across process restarts.
 *
 * Ported from: sleepwalker_protocol/continuity.py
 * Governed by: ORG-DEV-OTOI-1.0.3
 * Reference: docs/phase-1-reference-analysis.md §5 (ContinuityManager)
 */

#include "SleepwalkerTypes.h"
#include <nlohmann/json.hpp>
#include <string>
#include <filesystem>

namespace sleepwalker {

class ContinuityManager {
public:
    /**
     * Construct a ContinuityManager with a storage directory.
     * The directory is created if it does not exist (matching Python's
     * mkdir(exist_ok=True) behavior).
     *
     * @param storagePath Path for storing session continuity data
     *        (default: ".swp_storage")
     */
    explicit ContinuityManager(const std::string& storagePath = ".swp_storage");

    /**
     * Save session data for temporal continuity.
     * Adds an ISO 8601 timestamp, increments session count, and preserves
     * declared_boundaries across sessions.
     *
     * @param userId User identifier
     * @param sessionData Current session data to store (JSON object)
     */
    void saveSession(const std::string& userId, nlohmann::json sessionData);

    /**
     * Retrieve continuity context for a user.
     *
     * @param userId User identifier
     * @return ContinuityContext with has_history flag and state summary
     */
    ContinuityContext getContext(const std::string& userId);

    /**
     * Retrieve last session state for a user.
     *
     * @param userId User identifier
     * @return Last session state data as JSON (empty object if no history)
     */
    nlohmann::json retrieveLastSessionState(const std::string& userId);

    /**
     * Update user's declared boundaries.
     *
     * @param userId User identifier
     * @param boundaryType Type of boundary (e.g., "protected_topics")
     * @param boundaryValue Value for the boundary (any JSON value)
     */
    void updateBoundary(const std::string& userId,
                        const std::string& boundaryType,
                        const nlohmann::json& boundaryValue);

    /**
     * Get the storage path for this manager.
     */
    const std::filesystem::path& storagePath() const { return m_storagePath; }

private:
    std::filesystem::path m_storagePath;

    /**
     * Load user data from storage. Returns empty JSON object if file
     * does not exist or is unreadable.
     */
    nlohmann::json loadUserData(const std::string& userId);

    /**
     * Calculate days since a given ISO 8601 timestamp string.
     * Returns std::nullopt if the timestamp is invalid or empty.
     */
    std::optional<int> calculateDaysSince(const std::string& timestampStr);
};

} // namespace sleepwalker
