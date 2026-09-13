#include <catch2/catch_test_macros.hpp>
#include "rrt/RRTAdvocate.h"
#include "rrt/CrisisEngine.h"
#include "rrt/RRTTypes.h"

using namespace rrt;

// --- structural / safe-default behavior ------------------------------------

TEST_CASE("RRT: status report has expected shape", "[rrt]") {
    RRTAdvocate advocate("user-1");
    auto status = advocate.getStatusReport();
    REQUIRE(status["user_id"] == "user-1");
    REQUIRE(status["monitoring_active"] == false);
    REQUIRE(status["current_crisis"]["level"] == "stable");
    REQUIRE(status["performance"]["avg_response_time"] == 0.0);
}

TEST_CASE("RRT: empty message is green", "[rrt]") {
    RRTAdvocate advocate;
    auto assessment = advocate.assessMessage("");
    REQUIRE(assessment.crisisLevel == CrisisLevel::GREEN);
    REQUIRE(assessment.confidenceScore == 0.0);
}

// --- real 3-layer detection behavior ---------------------------------------

TEST_CASE("RRT: benign message is green", "[rrt]") {
    RRTAdvocate advocate;
    auto assessment = advocate.assessMessage("Thanks, the report looks great!");
    REQUIRE(assessment.crisisLevel == CrisisLevel::GREEN);
    REQUIRE(assessment.recommendedInterventions.empty());
    REQUIRE(assessment.contextFactors["self_harm_risk"] == false);
}

TEST_CASE("RRT: distress language elevates but not critical", "[rrt]") {
    RRTAdvocate advocate;
    auto assessment = advocate.assessMessage(
        "I'm worthless and I can't cope. Everything is too much and I'm falling apart.");
    REQUIRE((assessment.crisisLevel == CrisisLevel::YELLOW || assessment.crisisLevel == CrisisLevel::ORANGE));
    auto detected = assessment.secondaryIndicators;
    bool hasRelevant = false;
    for (const auto& f : detected) {
        if (f == "overwhelm" || f == "negative_self_talk") hasRelevant = true;
    }
    REQUIRE(hasRelevant);
    REQUIRE(assessment.contextFactors["self_harm_risk"] == false);
    REQUIRE(!assessment.recommendedInterventions.empty());
}

TEST_CASE("RRT: self-harm language escalates to emergency", "[rrt]") {
    RRTAdvocate advocate;
    auto assessment = advocate.assessMessage("I can't do this anymore, I want to kill myself.");
    REQUIRE(assessment.crisisLevel == CrisisLevel::BLACK);
    REQUIRE(assessment.contextFactors["self_harm_risk"] == true);
    bool hasSelfHarmIndicator = false;
    for (const auto& ind : assessment.primaryIndicators) {
        if (ind == "SELF_HARM_RISK") hasSelfHarmIndicator = true;
    }
    REQUIRE(hasSelfHarmIndicator);
    REQUIRE(assessment.userSafetyScore < 0.3);
    bool hasCrisisHotline = false;
    for (const auto& i : assessment.recommendedInterventions) {
        if (i == "crisis_hotline") hasCrisisHotline = true;
    }
    REQUIRE(hasCrisisHotline);
}

TEST_CASE("RRT: CrisisEngine detect returns indicators", "[rrt]") {
    CrisisEngine engine("user-1");
    auto indicators = engine.detect("I'm feeling overwhelmed and can't cope");
    REQUIRE(indicators.layer1Confidence > 0.0);
    REQUIRE(indicators.aggregateConfidence >= 0.0);
}

TEST_CASE("RRT: CrisisEngine reset session clears state", "[rrt]") {
    CrisisEngine engine("user-1");
    engine.detect("I'm feeling overwhelmed");
    engine.resetSession();
    auto indicators = engine.detect("");
    REQUIRE(indicators.aggregateConfidence == 0.0);
}

TEST_CASE("RRT: multiple assessments track state", "[rrt]") {
    RRTAdvocate advocate("user-2");
    advocate.assessMessage("I'm fine today");
    advocate.assessMessage("Everything is terrible and I can't go on");
    auto status = advocate.getStatusReport();
    REQUIRE(status["user_id"] == "user-2");
}