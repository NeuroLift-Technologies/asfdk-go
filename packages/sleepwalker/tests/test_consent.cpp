/**
 * test_consent.cpp — Unit tests for ConsentManager.
 *
 * Ported from: sleepwalker_protocol/tests/test_consent.py
 * Governed by: ORG-DEV-OTOI-1.0.3
 */

#include <catch2/catch_test_macros.hpp>
#include "sleepwalker/ConsentManager.h"
#include "sleepwalker/SleepwalkerTypes.h"

using namespace sleepwalker;

// Helper to create an EmotionalState quickly
static EmotionalState makeState(StateType type, bool protective, bool requiresCheckIn,
                                bool suicidal = false, bool selfHarm = false,
                                bool safety = false, double conf = 0.5) {
    EmotionalState state;
    state.stateType = type;
    state.protective = protective;
    state.requiresCheckIn = requiresCheckIn;
    state.indicators.crisis.suicidalIdeation = suicidal;
    state.indicators.crisis.selfHarm = selfHarm;
    state.indicators.crisis.safetyConcern = safety;
    state.explicitSuicidalIdeation = suicidal;
    state.selfHarmIndicators = selfHarm;
    state.inabilityToEnsureSafety = safety;
    state.confidence = conf;
    return state;
}

TEST_CASE("ConsentManager: initializes with TOI config", "[consent]") {
    nlohmann::json toi = R"({
        "swp": {
            "active": true,
            "intervention_threshold": "user_initiated_only"
        }
    })"_json;

    ConsentManager manager(toi);
    REQUIRE(true); // Just verify construction
}

TEST_CASE("ConsentManager: PASSIVE for protective state with user_initiated_only", "[consent]") {
    nlohmann::json toi = R"({"swp": {"intervention_threshold": "user_initiated_only"}})"_json;
    ConsentManager manager(toi);

    EmotionalState state = makeState(StateType::Dissociation, true, false);
    ConsentLevel level = manager.determineLevel(state);
    REQUIRE(level == ConsentLevel::Passive);
}

TEST_CASE("ConsentManager: LOW_PRESSURE with offer_support_without_pressure", "[consent]") {
    nlohmann::json toi = R"({"swp": {"intervention_threshold": "offer_support_without_pressure"}})"_json;
    ConsentManager manager(toi);

    EmotionalState state = makeState(StateType::Numbing, true, false);
    ConsentLevel level = manager.determineLevel(state);
    REQUIRE(level == ConsentLevel::LowPressure);
}

TEST_CASE("ConsentManager: RRTA_HANDOFF for crisis situations", "[consent]") {
    nlohmann::json toi = R"({"swp": {}})"_json;
    ConsentManager manager(toi);

    EmotionalState state = makeState(StateType::Neutral, false, true,
                                     /*suicidal=*/false, /*selfHarm=*/false,
                                     /*safety=*/true, 0.9);
    ConsentLevel level = manager.determineLevel(state);
    REQUIRE(level == ConsentLevel::RRTAHandoff);
}

TEST_CASE("ConsentManager: RRTA_HANDOFF for suicidal ideation", "[consent]") {
    nlohmann::json toi = R"({"swp": {}})"_json;
    ConsentManager manager(toi);

    EmotionalState state = makeState(StateType::Neutral, false, true,
                                     /*suicidal=*/true);
    ConsentLevel level = manager.determineLevel(state);
    REQUIRE(level == ConsentLevel::RRTAHandoff);
}

TEST_CASE("ConsentManager: RRTA_HANDOFF for self-harm", "[consent]") {
    nlohmann::json toi = R"({"swp": {}})"_json;
    ConsentManager manager(toi);

    EmotionalState state = makeState(StateType::Neutral, false, true,
                                     /*suicidal=*/false, /*selfHarm=*/true);
    ConsentLevel level = manager.determineLevel(state);
    REQUIRE(level == ConsentLevel::RRTAHandoff);
}

TEST_CASE("ConsentManager: SAFETY_CHECK for requires_check_in without crisis", "[consent]") {
    nlohmann::json toi = R"({"swp": {}})"_json;
    ConsentManager manager(toi);

    EmotionalState state = makeState(StateType::Neutral, false, true,
                                     /*suicidal=*/false, /*selfHarm=*/false,
                                     /*safety=*/false);
    ConsentLevel level = manager.determineLevel(state);
    REQUIRE(level == ConsentLevel::SafetyCheck);
}

TEST_CASE("ConsentManager: shouldIntervene logic", "[consent]") {
    nlohmann::json toi = R"({"swp": {}})"_json;
    ConsentManager manager(toi);

    // Passive level should not intervene
    EmotionalState passiveState = makeState(StateType::Neutral, false, false);
    REQUIRE(manager.shouldIntervene(passiveState) == false);

    // Crisis level should intervene
    EmotionalState crisisState = makeState(StateType::Neutral, false, true,
                                           /*suicidal=*/false, /*selfHarm=*/false,
                                           /*safety=*/true);
    REQUIRE(manager.shouldIntervene(crisisState) == true);

    // Safety check should intervene
    EmotionalState safetyState = makeState(StateType::Neutral, false, true,
                                           /*suicidal=*/false, /*selfHarm=*/false,
                                           /*safety=*/false);
    REQUIRE(manager.shouldIntervene(safetyState) == true);

    // Low pressure should not intervene
    EmotionalState lowPressureState = makeState(StateType::Numbing, true, false);
    nlohmann::json lowToi = R"({"swp": {"intervention_threshold": "offer_support_without_pressure"}})"_json;
    ConsentManager lowManager(lowToi);
    REQUIRE(lowManager.shouldIntervene(lowPressureState) == false);
}

TEST_CASE("ConsentManager: consent messages for all levels", "[consent]") {
    nlohmann::json toi = R"({"swp": {}})"_json;
    ConsentManager manager(toi);

    // All levels should have non-empty messages
    std::string passiveMsg = manager.getConsentMessage(ConsentLevel::Passive);
    REQUIRE(passiveMsg.length() > 0);
    REQUIRE(passiveMsg.find("No pressure") != std::string::npos);

    std::string lowPressureMsg = manager.getConsentMessage(ConsentLevel::LowPressure);
    REQUIRE(lowPressureMsg.length() > 0);

    std::string safetyMsg = manager.getConsentMessage(ConsentLevel::SafetyCheck);
    REQUIRE(safetyMsg.length() > 0);
    REQUIRE(safetyMsg.find("safe") != std::string::npos);

    std::string rrtaMsg = manager.getConsentMessage(ConsentLevel::RRTAHandoff);
    REQUIRE(rrtaMsg.length() > 0);
    // RRTA handoff message should mention crisis or safety
    bool mentionsCrisis = rrtaMsg.find("crisis") != std::string::npos;
    bool mentionsSafety = rrtaMsg.find("safety") != std::string::npos;
    bool hasMatch = mentionsCrisis || mentionsSafety;
    REQUIRE(hasMatch);
}

TEST_CASE("ConsentManager: getAppropriateLevel mirrors determineLevel", "[consent]") {
    nlohmann::json toi = R"({"swp": {"intervention_threshold": "user_initiated_only"}})"_json;
    ConsentManager manager(toi);

    EmotionalState state = makeState(StateType::Avoidance, true, false);
    ConsentLevel level = manager.getAppropriateLevel(state);
    REQUIRE(level == ConsentLevel::Passive);

    // Equivalent to determineLevel
    REQUIRE(manager.getAppropriateLevel(state) == manager.determineLevel(state));
}

TEST_CASE("ConsentManager: neutral state returns PASSIVE", "[consent]") {
    nlohmann::json toi = R"({"swp": {"intervention_threshold": "user_initiated_only"}})"_json;
    ConsentManager manager(toi);

    EmotionalState state = makeState(StateType::Neutral, false, false);
    ConsentLevel level = manager.determineLevel(state);
    REQUIRE(level == ConsentLevel::Passive);
}

TEST_CASE("ConsentManager: empty TOI defaults to user_initiated_only", "[consent]") {
    nlohmann::json toi = nlohmann::json::object();
    ConsentManager manager(toi);

    EmotionalState state = makeState(StateType::Dissociation, true, false);
    ConsentLevel level = manager.determineLevel(state);
    REQUIRE(level == ConsentLevel::Passive);
}
