#include <catch2/catch_test_macros.hpp>
#include "rrt/BurnoutDetector.h"
#include "rrt/RRTAdvocate.h"
#include "rrt/RRTTypes.h"

using namespace rrt;

TEST_CASE("Burnout: no burnout for healthy metrics", "[burnout]") {
    BurnoutDetector detector("user-1");
    SessionMetrics metrics;
    metrics.duration = 30.0;
    metrics.messageCount = 10;
    metrics.avgResponseTime = 5.0;
    metrics.errorCount = 0;
    metrics.taskCompletionRate = 0.9;
    auto assessment = detector.assessBurnout(metrics);
    REQUIRE(assessment.level == BurnoutLevel::NONE);
    REQUIRE(assessment.overallScore < 0.25);
    REQUIRE(assessment.escalationRequired == false);
}

TEST_CASE("Burnout: severe burnout for extreme metrics", "[burnout]") {
    BurnoutDetector detector("user-1");
    SessionMetrics metrics;
    metrics.duration = 300.0;
    metrics.messageCount = 100;
    metrics.avgResponseTime = 200.0;
    metrics.errorCount = 10;
    metrics.taskCompletionRate = 0.2;
    auto assessment = detector.assessBurnout(metrics);
    REQUIRE(assessment.level == BurnoutLevel::SEVERE);
    REQUIRE(assessment.escalationRequired == true);
    REQUIRE(!assessment.recommendations.empty());
}

TEST_CASE("Burnout: moderate burnout for concerning metrics", "[burnout]") {
    BurnoutDetector detector("user-1");
    SessionMetrics metrics;
    metrics.duration = 150.0;
    metrics.messageCount = 60;
    metrics.avgResponseTime = 80.0;
    metrics.errorCount = 3;
    metrics.taskCompletionRate = 0.5;
    auto assessment = detector.assessBurnout(metrics);
    REQUIRE((assessment.level == BurnoutLevel::MODERATE || assessment.level == BurnoutLevel::SEVERE));
}

TEST_CASE("Burnout: indicators computed correctly", "[burnout]") {
    BurnoutDetector detector("user-1");
    SessionMetrics metrics;
    metrics.duration = 300.0;
    metrics.messageCount = 5;
    metrics.avgResponseTime = 5.0;
    metrics.errorCount = 0;
    metrics.taskCompletionRate = 0.9;
    auto assessment = detector.assessBurnout(metrics);
    bool hasExtendedSession = false;
    for (const auto& ind : assessment.indicators) {
        if (ind.name == "extended_session") hasExtendedSession = true;
    }
    REQUIRE(hasExtendedSession);
}

TEST_CASE("Burnout: crisis level history affects assessment", "[burnout]") {
    BurnoutDetector detector("user-1");
    for (int i = 0; i < 10; ++i) {
        detector.recordCrisisLevel(CrisisLevel::RED);
    }
    SessionMetrics metrics;
    metrics.duration = 30.0;
    metrics.messageCount = 10;
    metrics.avgResponseTime = 5.0;
    metrics.errorCount = 0;
    metrics.taskCompletionRate = 0.9;
    auto assessment = detector.assessBurnout(metrics);
    bool hasElevatedCrisis = false;
    for (const auto& ind : assessment.indicators) {
        if (ind.name == "elevated_crisis_frequency") hasElevatedCrisis = true;
    }
    REQUIRE(hasElevatedCrisis);
}

TEST_CASE("Burnout: reset clears history", "[burnout]") {
    BurnoutDetector detector("user-1");
    for (int i = 0; i < 10; ++i) {
        detector.recordCrisisLevel(CrisisLevel::RED);
    }
    detector.reset();
    SessionMetrics metrics;
    metrics.duration = 30.0;
    metrics.messageCount = 10;
    metrics.avgResponseTime = 5.0;
    metrics.errorCount = 0;
    metrics.taskCompletionRate = 0.9;
    auto assessment = detector.assessBurnout(metrics);
    REQUIRE(assessment.level == BurnoutLevel::NONE);
}

TEST_CASE("Burnout: advocate integrates burnout detection", "[burnout]") {
    RRTAdvocate advocate("user-1");
    SessionMetrics metrics;
    metrics.duration = 300.0;
    metrics.messageCount = 100;
    metrics.avgResponseTime = 200.0;
    metrics.errorCount = 10;
    metrics.taskCompletionRate = 0.2;
    auto assessment = advocate.assessBurnout(metrics);
    REQUIRE(assessment.level == BurnoutLevel::SEVERE);
}