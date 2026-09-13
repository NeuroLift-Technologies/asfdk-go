/**
 * test_sleepwalker.cpp — Unit tests for SleepwalkerProtocol main orchestrator.
 *
 * Ported from: sleepwalker_protocol/tests/test_protocol.py
 * Governed by: ORG-DEV-OTOI-1.0.3
 *
 * This file also serves as the main test entry point (CATCH_CONFIG_MAIN).
 */

#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "sleepwalker/SleepwalkerProtocol.h"
#include "sleepwalker/StateDetector.h"
#include "sleepwalker/ConsentManager.h"
#include "sleepwalker/ContinuityManager.h"
#include "sleepwalker/SleepwalkerTypes.h"
#include <cstdlib>
#include <ctime>
#include <filesystem>

using namespace sleepwalker;

// ==================== SleepwalkerProtocol tests ====================

TEST_CASE("SleepwalkerProtocol: SWP initializes correctly", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);
    REQUIRE(true); // Just verify construction
}

TEST_CASE("SleepwalkerProtocol: detects neutral state", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    EmotionalState state = swp.detectEmotionalState("Can you help me with this task?");
    REQUIRE(state.stateType == StateType::Neutral);
    REQUIRE(state.protective == false);
}

TEST_CASE("SleepwalkerProtocol: detects dissociation", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    EmotionalState state = swp.detectEmotionalState("I feel really disconnected right now");
    REQUIRE(state.stateType == StateType::Dissociation);
    REQUIRE(state.protective == true);
}

TEST_CASE("SleepwalkerProtocol: detects numbing", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    EmotionalState state = swp.detectEmotionalState("I don't feel anything anymore");
    REQUIRE(state.stateType == StateType::Numbing);
    REQUIRE(state.protective == true);
}

TEST_CASE("SleepwalkerProtocol: detects avoidance", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    EmotionalState state = swp.detectEmotionalState("I'm not ready to talk about that");
    REQUIRE(state.stateType == StateType::Avoidance);
    REQUIRE(state.protective == true);
}

TEST_CASE("SleepwalkerProtocol: protective state response", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    ResponseGuidance response = swp.generateResponse("I'm feeling pretty detached");
    REQUIRE(response.responseType == "stable_low_demand");
    REQUIRE(response.intervention == "none");
    REQUIRE(response.focus.has_value());
    REQUIRE(response.focus.value() == "task_support");
}

TEST_CASE("SleepwalkerProtocol: neutral state response", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    ResponseGuidance response = swp.generateResponse("Can you help me write code?");
    REQUIRE(response.responseType == "neutral");
    REQUIRE(response.intervention == "none");
}

TEST_CASE("SleepwalkerProtocol: crisis detection", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    EmotionalState state = swp.detectEmotionalState("I don't feel safe right now");
    REQUIRE(state.requiresCheckIn == true);
}

TEST_CASE("SleepwalkerProtocol: RRTA handoff decision", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    // Non-crisis protective state should not trigger RRTA
    EmotionalState safeState = swp.detectEmotionalState("I'm feeling disconnected");
    REQUIRE(swp.requiresRrtaHandoff(safeState) == false);

    // Crisis state should trigger RRTA
    EmotionalState crisisState = swp.detectEmotionalState("I can't keep myself safe");
    REQUIRE(swp.requiresRrtaHandoff(crisisState) == true);
}

TEST_CASE("SleepwalkerProtocol: consent levels", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    // Neutral state -> PASSIVE
    EmotionalState neutralState = swp.detectEmotionalState("Help me with a task");
    ConsentLevel level = swp.determineAppropriateLevel(neutralState);
    REQUIRE(level == ConsentLevel::Passive);

    // Protective state -> PASSIVE (respects boundaries)
    EmotionalState protectiveState = swp.detectEmotionalState("I'm feeling numb");
    level = swp.determineAppropriateLevel(protectiveState);
    REQUIRE(level == ConsentLevel::Passive);
}

TEST_CASE("SleepwalkerProtocol: assess interaction", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    InteractionAssessment assessment = swp.assessInteraction("I'm working on a project");
    REQUIRE(assessment.swpActive == true);
    REQUIRE(assessment.protectiveStateActive == false);
    REQUIRE(assessment.consentLevel == ConsentLevel::Passive);
}

TEST_CASE("SleepwalkerProtocol: assess interaction with protective state", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    InteractionAssessment assessment = swp.assessInteraction("I feel numb and detached");
    REQUIRE(assessment.protectiveStateActive == true);
    REQUIRE(assessment.emotionalState.stateType == StateType::Dissociation);
}

TEST_CASE("SleepwalkerProtocol: get SWP context", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    nlohmann::json context = swp.getContext();
    REQUIRE(context.contains("swp_active"));
    REQUIRE(context.contains("user_boundaries"));
    REQUIRE(context.contains("consent_preferences"));
    REQUIRE(context["swp_active"].is_boolean());
    REQUIRE(context["swp_active"].get<bool>() == true);
}

TEST_CASE("SleepwalkerProtocol: SWP inactive when configured", "[sleepwalker]") {
    nlohmann::json toi = R"({"swp": {"active": false}})"_json;
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    opts.userToi = toi;
    SleepwalkerProtocol swp(opts);

    nlohmann::json context = swp.getContext();
    REQUIRE(context["swp_active"].get<bool>() == false);

    // generateResponse should not produce stable_low_demand when SWP inactive
    ResponseGuidance response = swp.generateResponse("I feel numb");
    // When SWP is inactive, the protective state should not trigger stable_low_demand
    bool isNeutralOrConsent = (response.responseType == "neutral" ||
                               response.responseType == "consent_offer");
    REQUIRE(isNeutralOrConsent);
}

TEST_CASE("SleepwalkerProtocol: maintain continuity", "[sleepwalker]") {
    std::string storagePath = std::string(std::getenv("HOME")) + "/.swp_test_continuity";
    std::filesystem::remove_all(storagePath);

    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    opts.storagePath = storagePath;
    SleepwalkerProtocol swp(opts);

    nlohmann::json sessionData = {
        {"emotional_state", "neutral"},
        {"user_id", "test_continuity_user"}
    };
    swp.maintainContinuity("test_continuity_user", sessionData);

    // Verify session was saved
    ContinuityManager cm(storagePath);
    nlohmann::json lastSession = cm.retrieveLastSessionState("test_continuity_user");
    REQUIRE(lastSession.contains("emotional_state"));
    REQUIRE(lastSession["emotional_state"].get<std::string>() == "neutral");

    std::filesystem::remove_all(storagePath);
}

TEST_CASE("SleepwalkerProtocol: generate response with explicit state", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    // Create a crisis state
    EmotionalState crisisState;
    crisisState.stateType = StateType::Neutral;
    crisisState.protective = false;
    crisisState.requiresCheckIn = true;
    crisisState.explicitSuicidalIdeation = true;
    crisisState.confidence = 0.9;

    ResponseGuidance response = swp.generateResponse("some input", crisisState);
    REQUIRE(response.responseType == "consent_offer");
    REQUIRE(response.intervention == "consent_required");
    REQUIRE(response.level.has_value());
    REQUIRE(response.level.value() == ConsentLevel::RRTAHandoff);
}

TEST_CASE("SleepwalkerProtocol: graduated consent offer levels", "[sleepwalker]") {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    SleepwalkerProtocol swp(opts);

    // Create a safety check state
    EmotionalState safetyState;
    safetyState.stateType = StateType::Neutral;
    safetyState.protective = false;
    safetyState.requiresCheckIn = true;
    safetyState.explicitSuicidalIdeation = false;
    safetyState.selfHarmIndicators = false;
    safetyState.inabilityToEnsureSafety = false;
    safetyState.indicators.crisis.safetyConcern = false;

    ResponseGuidance response = swp.generateResponse("some input", safetyState);
    REQUIRE(response.responseType == "consent_offer");
    REQUIRE(response.level.has_value());
    REQUIRE(response.level.value() == ConsentLevel::SafetyCheck);
    // Should contain a check-in message
    REQUIRE(response.guidance.find("safe") != std::string::npos);
}

// ==================== Integration-layer tests ====================

TEST_CASE("SleepwalkerProtocol: getStatus returns correct structure", "[sleepwalker][integration]") {
    nlohmann::json status = SleepwalkerProtocol::getStatus();
    REQUIRE(status["active"].get<bool>() == true);
    REQUIRE(status["mode"].get<std::string>() == "emotional-continuity");
}

TEST_CASE("SleepwalkerProtocol: reset does not throw", "[sleepwalker][integration]") {
    REQUIRE_NOTHROW(SleepwalkerProtocol::reset());
}

// ==================== Channel normalization tests ====================

TEST_CASE("Channel: normalizes known channels", "[types][channel]") {
    REQUIRE(normalizeChannel("user_input") == Channel::UserInput);
    REQUIRE(normalizeChannel("model_output") == Channel::ModelOutput);
    REQUIRE(normalizeChannel("tool_result") == Channel::ToolResult);
    REQUIRE(normalizeChannel("system") == Channel::System);
}

TEST_CASE("Channel: unknown values collapse to Unknown", "[types][channel]") {
    REQUIRE(normalizeChannel("unknown") == Channel::Unknown);
    REQUIRE(normalizeChannel("garbage") == Channel::Unknown);
    REQUIRE(normalizeChannel("USER_INPUT") == Channel::Unknown); // case sensitive
    REQUIRE(normalizeChannel("") == Channel::Unknown);
}

TEST_CASE("Channel: only UserInput is trusted", "[types][channel]") {
    // Only USER_INPUT is trusted per Phase 1 §5.4
    REQUIRE(normalizeChannel("user_input") == Channel::UserInput);
    REQUIRE(normalizeChannel("model_output") != Channel::UserInput);
    REQUIRE(normalizeChannel("tool_result") != Channel::UserInput);
    REQUIRE(normalizeChannel("system") != Channel::UserInput);
    REQUIRE(normalizeChannel("unknown") != Channel::UserInput);
}

TEST_CASE("Channel: toString round-trips", "[types][channel]") {
    REQUIRE(channelToString(Channel::UserInput) == "user_input");
    REQUIRE(channelToString(Channel::ModelOutput) == "model_output");
    REQUIRE(channelToString(Channel::ToolResult) == "tool_result");
    REQUIRE(channelToString(Channel::System) == "system");
    REQUIRE(channelToString(Channel::Unknown) == "unknown");
}

// ==================== Enum conversion tests ====================

TEST_CASE("StateType: toString round-trips", "[types]") {
    REQUIRE(stateTypeToString(StateType::Dissociation) == "dissociation");
    REQUIRE(stateTypeToString(StateType::Numbing) == "numbing");
    REQUIRE(stateTypeToString(StateType::Avoidance) == "avoidance");
    REQUIRE(stateTypeToString(StateType::Detachment) == "detachment");
    REQUIRE(stateTypeToString(StateType::Neutral) == "neutral");
}

TEST_CASE("StateType: stringToStateType round-trips", "[types]") {
    REQUIRE(stringToStateType("dissociation") == StateType::Dissociation);
    REQUIRE(stringToStateType("numbing") == StateType::Numbing);
    REQUIRE(stringToStateType("avoidance") == StateType::Avoidance);
    REQUIRE(stringToStateType("detachment") == StateType::Detachment);
    REQUIRE(stringToStateType("neutral") == StateType::Neutral);
    REQUIRE(stringToStateType("unknown") == StateType::Neutral);
}

TEST_CASE("ConsentLevel: toString round-trips", "[types]") {
    REQUIRE(consentLevelToString(ConsentLevel::Passive) == "PASSIVE");
    REQUIRE(consentLevelToString(ConsentLevel::LowPressure) == "LOW_PRESSURE");
    REQUIRE(consentLevelToString(ConsentLevel::SafetyCheck) == "SAFETY_CHECK");
    REQUIRE(consentLevelToString(ConsentLevel::RRTAHandoff) == "RRTA_HANDOFF");
}
