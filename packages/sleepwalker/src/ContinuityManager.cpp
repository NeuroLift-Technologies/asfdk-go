/**
 * ContinuityManager.cpp — Implementation of temporal continuity management.
 *
 * Ported from: sleepwalker_protocol/continuity.py
 * Governed by: ORG-DEV-OTOI-1.0.3
 */

#include "sleepwalker/ContinuityManager.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <limits>

namespace sleepwalker {

ContinuityManager::ContinuityManager(const std::string& storagePath)
    : m_storagePath(storagePath) {
    // Create storage directory if it doesn't exist (matching Python's
    // Path.mkdir(exist_ok=True) behavior)
    if (!m_storagePath.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(m_storagePath, ec);
        if (ec) {
            spdlog::warn("Sleepwalker: Could not create storage directory {}: {}",
                         m_storagePath.string(), ec.message());
        }
    }
}

nlohmann::json ContinuityManager::loadUserData(const std::string& userId) {
    std::filesystem::path userFile = m_storagePath / (userId + ".json");

    if (!std::filesystem::exists(userFile)) {
        return nlohmann::json::object();
    }

    try {
        std::ifstream f(userFile);
        nlohmann::json data;
        f >> data;
        return data;
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::warn("Sleepwalker: Failed to parse user data for {}: {}", userId, e.what());
        return nlohmann::json::object();
    } catch (const std::exception& e) {
        spdlog::warn("Sleepwalker: Failed to load user data for {}: {}", userId, e.what());
        return nlohmann::json::object();
    }
}

std::optional<int> ContinuityManager::calculateDaysSince(const std::string& timestampStr) {
    if (timestampStr.empty()) {
        return std::nullopt;
    }

    // Parse ISO 8601 timestamp (Python uses datetime.fromisoformat)
    // Handle both "YYYY-MM-DDTHH:MM:SS" and "YYYY-MM-DDTHH:MM:SS.ffffff" formats
    std::tm tm = {};
    std::istringstream ss(timestampStr);

    // Try parsing with fractional seconds
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) {
        return std::nullopt;
    }

    // Skip optional fractional seconds (saveSession writes %Y-%m-%dT%H:%M:%SZ without fractions)
    if (ss.peek() == '.') {
        ss.ignore(std::numeric_limits<std::streamsize>::max(), 'Z');
    }

    // Convert parsed time to time_t and compute delta
    tm.tm_isdst = -1;
    std::time_t parsedTime = std::mktime(&tm);
    if (parsedTime == -1) {
        return std::nullopt;
    }

    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);

    double diffSeconds = std::difftime(nowTime, parsedTime);
    int days = static_cast<int>(diffSeconds / 86400);
    return days;
}

void ContinuityManager::saveSession(const std::string& userId, nlohmann::json sessionData) {
    // Add timestamp (matching Python's datetime.now().isoformat())
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    if (gmtime_r(&nowTime, &tm) == nullptr) {
        spdlog::error("Sleepwalker: Failed to convert current time to UTC for {}", userId);
        return;
    }
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    sessionData["timestamp"] = ss.str();

    // Load existing data
    nlohmann::json existingData = loadUserData(userId);

    // Update with new session data
    existingData["last_session"] = sessionData;
    existingData["session_count"] = existingData.value("session_count", 0) + 1;

    // Preserve declared boundaries across sessions
    if (sessionData.contains("declared_boundaries")) {
        existingData["declared_boundaries"] = sessionData["declared_boundaries"];
    }

    // Save updated data
    std::filesystem::path userFile = m_storagePath / (userId + ".json");
    try {
        std::ofstream f(userFile);
        f << std::setw(2) << existingData;
    } catch (const std::exception& e) {
        spdlog::error("Sleepwalker: Failed to save session for {}: {}", userId, e.what());
    }
}

ContinuityContext ContinuityManager::getContext(const std::string& userId) {
    nlohmann::json userData = loadUserData(userId);

    if (userData.empty()) {
        ContinuityContext ctx;
        ctx.hasHistory = false;
        ctx.protectiveStateActive = false;
        ctx.declaredBoundaries = nlohmann::json::array();
        return ctx;
    }

    ContinuityContext ctx;
    ctx.hasHistory = true;

    nlohmann::json lastSession = userData.value("last_session", nlohmann::json::object());

    // lastSessionState — defaults to "unknown"
    if (lastSession.contains("emotional_state") && lastSession["emotional_state"].is_string()) {
        ctx.lastSessionState = lastSession["emotional_state"].get<std::string>();
    } else {
        ctx.lastSessionState = "unknown";
    }

    // protectiveStateActive
    ctx.protectiveStateActive = lastSession.value("protective_state_active", false);

    // declared_boundaries — may be array or object
    ctx.declaredBoundaries = userData.value("declared_boundaries", nlohmann::json::array());

    // days_since_last_session
    std::string timestamp = lastSession.value("timestamp", "");
    ctx.daysSinceLastSession = calculateDaysSince(timestamp);

    // session_count
    ctx.sessionCount = userData.value("session_count", 0);

    return ctx;
}

nlohmann::json ContinuityManager::retrieveLastSessionState(const std::string& userId) {
    nlohmann::json userData = loadUserData(userId);
    return userData.value("last_session", nlohmann::json::object());
}

void ContinuityManager::updateBoundary(const std::string& userId,
                                       const std::string& boundaryType,
                                       const nlohmann::json& boundaryValue) {
    nlohmann::json userData = loadUserData(userId);

    if (!userData.contains("declared_boundaries")) {
        userData["declared_boundaries"] = nlohmann::json::object();
    }

    userData["declared_boundaries"][boundaryType] = boundaryValue;

    // Add boundary_updated timestamp
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    if (gmtime_r(&nowTime, &tm) != nullptr) {
        std::ostringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        userData["boundary_updated"] = ss.str();
    } else {
        spdlog::error("Sleepwalker: Failed to generate UTC timestamp for boundary update");
    }

    // Save updated data
    std::filesystem::path userFile = m_storagePath / (userId + ".json");
    try {
        std::ofstream f(userFile);
        f << std::setw(2) << userData;
    } catch (const std::exception& e) {
        spdlog::error("Sleepwalker: Failed to update boundary for {}: {}", userId, e.what());
    }
}

} // namespace sleepwalker
