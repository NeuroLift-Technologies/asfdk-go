#include "rrt/RRTAdvocate.h"
#include "rrt/CrisisEngine.h"
#include "rrt/RRTTypes.h"
#include <cassert>
#include <iostream>

using namespace rrt;

int main() {
    int passed = 0, failed = 0;
    auto check = [&](bool cond, const char* name) {
        if (cond) { ++passed; std::cout << "PASS: " << name << "\n"; }
        else { ++failed; std::cout << "FAIL: " << name << "\n"; }
    };

    // Test 1: benign message is green
    {
        RRTAdvocate advocate("user-1");
        auto a = advocate.assessMessage("Thanks, the report looks great!");
        check(a.crisisLevel == CrisisLevel::GREEN, "benign message is green");
        check(a.recommendedInterventions.empty(), "benign: no interventions");
        check(a.contextFactors["self_harm_risk"] == false, "benign: no self-harm risk");
    }

    // Test 2: empty message is green
    {
        RRTAdvocate advocate("user-1");
        auto a = advocate.assessMessage("");
        check(a.crisisLevel == CrisisLevel::GREEN, "empty message is green");
        check(a.confidenceScore == 0.0, "empty: confidence is 0");
    }

    // Test 3: distress language elevates
    {
        RRTAdvocate advocate("user-1");
        auto a = advocate.assessMessage("I'm worthless and I can't cope. Everything is too much and I'm falling apart.");
        check(a.crisisLevel == CrisisLevel::YELLOW || a.crisisLevel == CrisisLevel::ORANGE, "distress elevates level");
        bool hasRelevant = false;
        for (const auto& f : a.secondaryIndicators)
            if (f == "overwhelm" || f == "negative_self_talk") hasRelevant = true;
        check(hasRelevant, "distress: has overwhelm/negative_self_talk");
        check(a.contextFactors["self_harm_risk"] == false, "distress: no self-harm risk");
        check(!a.recommendedInterventions.empty(), "distress: has interventions");
    }

    // Test 4: self-harm escalates to emergency
    {
        RRTAdvocate advocate("user-1");
        auto a = advocate.assessMessage("I can't do this anymore, I want to kill myself.");
        check(a.crisisLevel == CrisisLevel::BLACK, "self-harm: BLACK level");
        check(a.contextFactors["self_harm_risk"] == true, "self-harm: risk flagged");
        bool hasSelfHarm = false;
        for (const auto& ind : a.primaryIndicators)
            if (ind == "SELF_HARM_RISK") hasSelfHarm = true;
        check(hasSelfHarm, "self-harm: SELF_HARM_RISK indicator");
        check(a.userSafetyScore < 0.3, "self-harm: safety < 0.3");
        bool hasHotline = false;
        for (const auto& i : a.recommendedInterventions)
            if (i == "crisis_hotline") hasHotline = true;
        check(hasHotline, "self-harm: crisis_hotline recommended");
    }

    // Test 5: CrisisEngine detect
    {
        CrisisEngine engine("user-1");
        auto ind = engine.detect("I'm feeling overwhelmed and can't cope");
        check(ind.layer1Confidence > 0.0, "engine detect: keyword confidence > 0");
        check(ind.aggregateConfidence >= 0.0, "engine detect: aggregate >= 0");
    }

    // Test 6: reset session
    {
        CrisisEngine engine("user-1");
        engine.detect("I'm feeling overwhelmed");
        engine.resetSession();
        auto ind = engine.detect("");
        check(ind.aggregateConfidence == 0.0, "reset: empty after reset is 0");
    }

    // Test 7: Burnout detection
    {
        RRTAdvocate advocate("user-1");
        SessionMetrics metrics;
        metrics.duration = 300.0;
        metrics.messageCount = 100;
        metrics.avgResponseTime = 200.0;
        metrics.errorCount = 10;
        metrics.taskCompletionRate = 0.2;
        auto a = advocate.assessBurnout(metrics);
        check(a.level == BurnoutLevel::SEVERE, "burnout: severe for extreme metrics");
        check(a.escalationRequired == true, "burnout: escalation required");
    }

    std::cout << "\n=== Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed > 0 ? 1 : 0;
}