/**
 * test_state_detection.cpp — Unit tests for StateDetector.
 *
 * Ported from: sleepwalker_protocol/tests/test_state_detection.py
 * Governed by: ORG-DEV-OTOI-1.0.3
 */

#include <catch2/catch_test_macros.hpp>
#include "sleepwalker/StateDetector.h"
#include "sleepwalker/SleepwalkerTypes.h"

using namespace sleepwalker;

TEST_CASE("StateDetector: initializes correctly", "[state-detection]") {
    StateDetector detector;
    REQUIRE(true); // Just verify construction doesn't throw
}

TEST_CASE("StateDetector: detects dissociation markers", "[state-detection]") {
    StateDetector detector;

    const std::vector<std::string> testInputs = {
        "I feel numb",
        "I'm feeling really detached",
        "I feel disconnected from myself",
        "I'm spaced out",
        "Everything feels foggy",
        "I'm experiencing derealization",
        "Depersonalization is happening",
        "I'm not really here",
        "I'm feeling nothing"
    };

    for (const auto& text : testInputs) {
        EmotionalState state = detector.detect(text);
        REQUIRE(state.indicators.dissociation == true);
        INFO("Failed to detect dissociation in: " << text);
    }
}

TEST_CASE("StateDetector: detects emotional numbing", "[state-detection]") {
    StateDetector detector;

    const std::vector<std::string> testInputs = {
        "I don't feel anything",
        "I don't feel much",
        "I'm emotionally flat",
        "I can't feel emotions",
        "I'm numb to it all",
        "I've shut down",
        "I feel turned off"
    };

    for (const auto& text : testInputs) {
        EmotionalState state = detector.detect(text);
        REQUIRE(state.indicators.numbing == true);
        INFO("Failed to detect numbing in: " << text);
    }
}

TEST_CASE("StateDetector: detects avoidance patterns", "[state-detection]") {
    StateDetector detector;

    const std::vector<std::string> testInputs = {
        "I'm not ready to talk about that",
        "I can't discuss this right now",
        "I can't think about this",
        "I'm avoiding thinking about it",
        "Maybe later, not now",
        "I don't want to go there"
    };

    for (const auto& text : testInputs) {
        EmotionalState state = detector.detect(text);
        REQUIRE(state.indicators.avoidance == true);
        INFO("Failed to detect avoidance in: " << text);
    }
}

TEST_CASE("StateDetector: detects protective detachment cues", "[state-detection]") {
    StateDetector detector;

    const std::vector<std::string> testInputs = {
        "I'm just fine",
        "It's whatever",
        "It doesn't matter",
        "I don't care",
        "It is what it is"
    };

    for (const auto& text : testInputs) {
        EmotionalState state = detector.detect(text);
        REQUIRE(state.indicators.detachment == true);
        INFO("Failed to detect detachment in: " << text);
    }
}

TEST_CASE("StateDetector: detects crisis indicators", "[state-detection]") {
    StateDetector detector;

    // Safety concern
    std::string crisisText = "I don't feel safe right now";
    CrisisIndicators crisis = detector.detect(crisisText).indicators.crisis;
    REQUIRE(crisis.safetyConcern == true);
    REQUIRE(crisis.suicidalIdeation == false);
    REQUIRE(crisis.selfHarm == false);

    // Suicidal ideation
    EmotionalState state1 = detector.detect("I keep thinking about suicide");
    REQUIRE(state1.indicators.crisis.suicidalIdeation == true);

    EmotionalState state2 = detector.detect("I want to kill myself");
    REQUIRE(state2.indicators.crisis.suicidalIdeation == true);

    // Self harm
    EmotionalState state3 = detector.detect("I cut myself");
    REQUIRE(state3.indicators.crisis.selfHarm == true);

    EmotionalState state4 = detector.detect("I hurt myself");
    REQUIRE(state4.indicators.crisis.selfHarm == true);

    // Safety concern variants
    EmotionalState state5 = detector.detect("I can't keep myself safe");
    REQUIRE(state5.indicators.crisis.safetyConcern == true);
}

TEST_CASE("StateDetector: neutral text produces no false positives", "[state-detection]") {
    StateDetector detector;

    const std::vector<std::string> neutralTexts = {
        "Can you help me with this project?",
        "I need to write some code",
        "What's the weather like?",
        "Tell me about C++ programming",
        "The meeting is at 3pm"
    };

    for (const auto& text : neutralTexts) {
        EmotionalState state = detector.detect(text);
        REQUIRE(state.protective == false);
        REQUIRE(state.stateType == StateType::Neutral);
        INFO("False positive for: " << text);
    }
}

TEST_CASE("StateDetector: confidence calculation", "[state-detection]") {
    StateDetector detector;

    // No indicators → 0.0
    EmotionalState noneState = detector.detect("Hello world");
    REQUIRE(noneState.confidence == 0.0);

    // Single indicator → 0.5
    EmotionalState singleState = detector.detect("I feel numb");
    REQUIRE(singleState.confidence == 0.5);
    REQUIRE(singleState.indicators.dissociation == true);

    // Multiple indicators (from different categories) → 0.8
    // "numb" → dissociation, "shut down" → numbing, "not ready to talk" → avoidance, "it's whatever" → detachment
    std::string multiText = "I feel numb, I've shut down, not ready to talk, it's whatever";
    EmotionalState multiState = detector.detect(multiText);
    REQUIRE(multiState.confidence == 0.8);
}

TEST_CASE("StateDetector: state type priority order", "[state-detection]") {
    StateDetector detector;

    // Dissociation takes priority
    EmotionalState state = detector.detect("I feel numb and I'm fine");
    REQUIRE(state.stateType == StateType::Dissociation);

    // Numbing when dissociation not present
    state = detector.detect("I can't feel anything");
    REQUIRE(state.stateType == StateType::Numbing);

    // Avoidance when none of the above
    state = detector.detect("I'm not ready to talk about it");
    REQUIRE(state.stateType == StateType::Avoidance);

    // Detachment when none of the above
    state = detector.detect("It doesn't matter to me");
    REQUIRE(state.stateType == StateType::Detachment);
}

TEST_CASE("StateDetector: protective flag and check-in", "[state-detection]") {
    StateDetector detector;

    // Protective state from dissociation
    EmotionalState state = detector.detect("I feel numb");
    REQUIRE(state.protective == true);
    REQUIRE(state.requiresCheckIn == false);

    // Crisis triggers requires_check_in
    state = detector.detect("I don't feel safe");
    REQUIRE(state.requiresCheckIn == true);
    REQUIRE(state.explicitSuicidalIdeation == false);
    REQUIRE(state.selfHarmIndicators == false);
    REQUIRE(state.inabilityToEnsureSafety == true);

    // Suicide text
    state = detector.detect("I keep thinking about suicide");
    REQUIRE(state.requiresCheckIn == true);
    REQUIRE(state.explicitSuicidalIdeation == true);
}
