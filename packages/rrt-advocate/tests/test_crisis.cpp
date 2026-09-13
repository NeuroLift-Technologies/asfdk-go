#include <catch2/catch_test_macros.hpp>
#include "rrt/CrisisDetector.h"
#include "rrt/CrisisAssessor.h"
#include "rrt/CrisisEngine.h"
#include "rrt/RRTTypes.h"

using namespace rrt;

TEST_CASE("Crisis: detector aggregates three layers", "[crisis]") {
    CrisisDetector detector;
    auto indicators = detector.detectCrisisIndicators("I'm overwhelmed and worthless");
    REQUIRE(indicators.layer1Confidence > 0.0);
    REQUIRE(indicators.layer2Confidence >= 0.0);
    REQUIRE(indicators.layer3Confidence >= 0.0);
    REQUIRE(indicators.aggregateConfidence > 0.0);
    REQUIRE(indicators.aggregateConfidence <= 1.0);
}

TEST_CASE("Crisis: self-harm forces aggregate to 1.0", "[crisis]") {
    CrisisDetector detector;
    auto indicators = detector.detectCrisisIndicators("I want to kill myself");
    REQUIRE(indicators.selfHarmRisk == true);
    REQUIRE(indicators.aggregateConfidence == 1.0);
}

TEST_CASE("Crisis: assessor maps confidence to levels", "[crisis]") {
    CrisisAssessor assessor("user-1");
    CrisisIndicators indicators;
    indicators.aggregateConfidence = 0.0;
    auto a1 = assessor.assessCrisis(indicators);
    REQUIRE(a1.crisisLevel == CrisisLevel::GREEN);

    indicators.aggregateConfidence = 0.3;
    auto a2 = assessor.assessCrisis(indicators);
    REQUIRE(a2.crisisLevel == CrisisLevel::YELLOW);

    indicators.aggregateConfidence = 0.5;
    auto a3 = assessor.assessCrisis(indicators);
    REQUIRE(a3.crisisLevel == CrisisLevel::ORANGE);

    indicators.aggregateConfidence = 0.8;
    auto a4 = assessor.assessCrisis(indicators);
    REQUIRE(a4.crisisLevel == CrisisLevel::RED);

    indicators.aggregateConfidence = 0.95;
    auto a5 = assessor.assessCrisis(indicators);
    REQUIRE(a5.crisisLevel == CrisisLevel::BLACK);
}

TEST_CASE("Crisis: self-harm overrides to BLACK regardless of confidence", "[crisis]") {
    CrisisAssessor assessor("user-1");
    CrisisIndicators indicators;
    indicators.aggregateConfidence = 0.1;
    indicators.selfHarmRisk = true;
    auto assessment = assessor.assessCrisis(indicators);
    REQUIRE(assessment.crisisLevel == CrisisLevel::BLACK);
    REQUIRE(assessment.userSafetyScore < 0.3);
}

TEST_CASE("Crisis: safety score inversely related to confidence", "[crisis]") {
    CrisisAssessor assessor("user-1");
    CrisisIndicators low;
    low.aggregateConfidence = 0.1;
    auto aLow = assessor.assessCrisis(low);
    REQUIRE(aLow.userSafetyScore > 0.5);

    CrisisIndicators high;
    high.aggregateConfidence = 0.8;
    auto aHigh = assessor.assessCrisis(high);
    REQUIRE(aHigh.userSafetyScore < 0.5);
}

TEST_CASE("Crisis: interventions escalate with level", "[crisis]") {
    CrisisAssessor assessor("user-1");
    CrisisIndicators indicators;
    indicators.aggregateConfidence = 0.0;
    auto green = assessor.assessCrisis(indicators);
    REQUIRE(green.recommendedInterventions.empty());

    indicators.aggregateConfidence = 0.95;
    auto black = assessor.assessCrisis(indicators);
    REQUIRE(!black.recommendedInterventions.empty());
    REQUIRE(black.recommendedInterventions[0] == "emergency_stabilization");
}

TEST_CASE("Crisis: engine detect then assess pipeline", "[crisis]") {
    CrisisEngine engine("user-1");
    auto indicators = engine.detect("I can't cope with this anymore");
    REQUIRE(indicators.aggregateConfidence > 0.0);
    auto assessment = engine.assess("I can't cope with this anymore");
    REQUIRE(assessment.crisisLevel != CrisisLevel::GREEN);
    REQUIRE(assessment.confidenceScore > 0.0);
}

TEST_CASE("Crisis: reset session clears detector state", "[crisis]") {
    CrisisEngine engine("user-1");
    engine.detect("I'm overwhelmed");
    engine.resetSession();
    auto indicators = engine.detect("");
    REQUIRE(indicators.aggregateConfidence == 0.0);
}