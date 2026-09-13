/**
 * test_continuity.cpp — Unit tests for ContinuityManager.
 *
 * Ported from: sleepwalker_protocol/tests/test_continuity.py
 * Governed by: ORG-DEV-OTOI-1.0.3
 */

#include <catch2/catch_test_macros.hpp>
#include "sleepwalker/ContinuityManager.h"
#include "sleepwalker/SleepwalkerTypes.h"
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <fstream>

using namespace sleepwalker;

// Helper to get a unique temp directory for each test
static std::string tempDir() {
    std::string dir = std::string(std::getenv("HOME")) + "/.swp_test_";
    dir += std::to_string(std::rand()) + "_" + std::to_string(std::time(nullptr));
    std::filesystem::create_directories(dir);
    return dir;
}

TEST_CASE("ContinuityManager: initializes and creates storage directory", "[continuity]") {
    std::string storagePath = tempDir();
    {
        ContinuityManager manager(storagePath);
        REQUIRE(std::filesystem::exists(storagePath));
    }
    // Cleanup
    std::filesystem::remove_all(storagePath);
}

TEST_CASE("ContinuityManager: save and retrieve session", "[continuity]") {
    std::string storagePath = tempDir();
    ContinuityManager manager(storagePath);

    std::string userId = "test_user_123";
    nlohmann::json sessionData = {
        {"emotional_state", "dissociation"},
        {"protective_state_active", true},
        {"declared_boundaries", {"topic1", "topic2"}}
    };

    manager.saveSession(userId, sessionData);

    ContinuityContext ctx = manager.getContext(userId);
    REQUIRE(ctx.hasHistory == true);
    REQUIRE(ctx.lastSessionState.has_value());
    REQUIRE(ctx.lastSessionState.value() == "dissociation");
    REQUIRE(ctx.protectiveStateActive == true);
    REQUIRE(ctx.declaredBoundaries.is_array());
    REQUIRE(ctx.declaredBoundaries.size() == 2);

    std::filesystem::remove_all(storagePath);
}

TEST_CASE("ContinuityManager: multiple sessions for same user", "[continuity]") {
    std::string storagePath = tempDir();
    ContinuityManager manager(storagePath);

    std::string userId = "test_user_456";

    manager.saveSession(userId, {{"emotional_state", "neutral"}});
    manager.saveSession(userId, {{"emotional_state", "numbing"}});
    manager.saveSession(userId, {{"emotional_state", "avoidance"}});

    ContinuityContext ctx = manager.getContext(userId);
    REQUIRE(ctx.hasHistory == true);
    REQUIRE(ctx.sessionCount == 3);
    REQUIRE(ctx.lastSessionState.has_value());
    REQUIRE(ctx.lastSessionState.value() == "avoidance");

    std::filesystem::remove_all(storagePath);
}

TEST_CASE("ContinuityManager: new user has empty context", "[continuity]") {
    std::string storagePath = tempDir();
    ContinuityManager manager(storagePath);

    ContinuityContext ctx = manager.getContext("new_user_789");
    REQUIRE(ctx.hasHistory == false);
    REQUIRE(ctx.protectiveStateActive == false);
    REQUIRE(ctx.declaredBoundaries.is_array());
    REQUIRE(ctx.declaredBoundaries.size() == 0);
    REQUIRE(ctx.sessionCount == 0);

    std::filesystem::remove_all(storagePath);
}

TEST_CASE("ContinuityManager: retrieve last session state", "[continuity]") {
    std::string storagePath = tempDir();
    ContinuityManager manager(storagePath);

    std::string userId = "test_user_last_session";
    nlohmann::json sessionData = {
        {"emotional_state", "detachment"},
        {"task_context", "email writing"}
    };

    manager.saveSession(userId, sessionData);

    nlohmann::json lastSession = manager.retrieveLastSessionState(userId);
    REQUIRE(lastSession.contains("emotional_state"));
    REQUIRE(lastSession["emotional_state"].get<std::string>() == "detachment");
    REQUIRE(lastSession["task_context"].get<std::string>() == "email writing");
    REQUIRE(lastSession.contains("timestamp"));

    std::filesystem::remove_all(storagePath);
}

TEST_CASE("ContinuityManager: update boundary", "[continuity]") {
    std::string storagePath = tempDir();
    ContinuityManager manager(storagePath);

    std::string userId = "test_user_boundary";
    manager.saveSession(userId, {{"emotional_state", "neutral"}});

    nlohmann::json boundaryValue = {"trauma", "relationships"};
    manager.updateBoundary(userId, "protected_topics", boundaryValue);

    ContinuityContext ctx = manager.getContext(userId);
    REQUIRE(ctx.declaredBoundaries.is_object());
    REQUIRE(ctx.declaredBoundaries.contains("protected_topics"));
    REQUIRE(ctx.declaredBoundaries["protected_topics"].size() == 2);

    std::filesystem::remove_all(storagePath);
}

TEST_CASE("ContinuityManager: boundary persistence across sessions", "[continuity]") {
    std::string storagePath = tempDir();
    ContinuityManager manager(storagePath);

    std::string userId = "test_user_persist";

    // First session with boundaries
    manager.saveSession(userId, {
        {"emotional_state", "neutral"},
        {"declared_boundaries", {{"protected_topics", {"topic_a"}}}}
    });

    // Second session without boundaries in session data
    manager.saveSession(userId, {{"emotional_state", "numbing"}});

    // Boundaries should still be there from first session
    ContinuityContext ctx = manager.getContext(userId);
    REQUIRE(ctx.hasHistory == true);
    REQUIRE(ctx.sessionCount == 2);
    // Boundaries preserved from first session
    REQUIRE(ctx.declaredBoundaries.contains("protected_topics"));

    std::filesystem::remove_all(storagePath);
}

TEST_CASE("ContinuityManager: corrupt file returns empty data", "[continuity]") {
    std::string storagePath = tempDir();
    ContinuityManager manager(storagePath);

    std::string userId = "test_user_corrupt";
    std::filesystem::path userFile = std::filesystem::path(storagePath) / (userId + ".json");

    // Write invalid JSON to the user file
    std::ofstream f(userFile);
    f << "{ this is not valid json }";
    f.close();

    ContinuityContext ctx = manager.getContext(userId);
    REQUIRE(ctx.hasHistory == false);

    std::filesystem::remove_all(storagePath);
}
