// standalone_test.cpp — dependency-free smoke runner for the ASFDK-C++ umbrella.
//
// Mirrors the hermetic-build convention in packages/include/README.md:
// builds with g++ -std=c++23 and packages/include as the fallback include root:
//
//   g++ -std=c++23 -I packages/asfdk/include -I packages/toi/include
//       -I packages/otoi/include -I packages/rrt-advocate/include
//       -I packages/sleepwalker/include -I packages/include
//       packages/asfdk/tests/standalone_test.cpp packages/asfdk/src/ASFDK.cpp
//       packages/toi/src/TermsOfInteraction.cpp packages/otoi/src/OTOIManager.cpp
//       packages/rrt-advocate/src/*.cpp packages/sleepwalker/src/*.cpp
//       -o /tmp/asfdk_test && /tmp/asfdk_test
//
// Exit code 0 = all checks pass; non-zero = failures (usable as a CI smoke test).

#include <asfdk/ASFDK.h>
#include <asfdk/ASFDKTypes.h>

#include <iostream>

int main() {
    int passed = 0, failed = 0;
    auto check = [&](bool cond, const char* name) {
        if (cond) { ++passed; std::cout << "PASS: " << name << "\n"; }
        else { ++failed; std::cout << "FAIL: " << name << "\n"; }
    };

    // ---------------- TOI surface ----------------
    {
        asfdk::ASFDK asfdk;
        auto doc = asfdk.parseTOI(nlohmann::json::parse(
            R"({"$toi":"1.0.0","$tier":"personal","identity":{"author":"anonymous"}})"));
        check(doc.$toi == "1.0.0", "toi: parses $toi");
        check(doc.$tier == "personal", "toi: parses $tier");
        check(doc.identity.author == "anonymous", "toi: parses identity.author");
        check(asfdk.validateTOI(doc), "toi: validates");
    }
    {
        asfdk::ASFDK asfdk;
        auto result = asfdk.safeParseTOI(nlohmann::json::parse(
            R"({"$toi":"2.0.0","$tier":"personal","identity":{"author":"anonymous"}})"));
        check(!result.has_value(), "toi: safeParse rejects invalid version");
        check(!result.error().message.empty(), "toi: safeParse populates error message");
    }
    {
        asfdk::ASFDK asfdk;
        auto personal = asfdk.parseTOI(nlohmann::json::parse(
            R"({"$toi":"1.0.0","$tier":"personal","identity":{"author":"user"},"communication":{"tone":"friendly"}})"));
        auto project = asfdk.parseTOI(nlohmann::json::parse(
            R"({"$toi":"1.0.0","$tier":"project","identity":{"author":"platform"},"communication":{"tone":"professional"}})"));
        auto resolved = asfdk.resolveTOI({personal, project});
        check(resolved.identity.author == "user", "toi: resolve keeps personal identity");
        check(resolved.communication.has_value(), "toi: resolve merges communication");
    }

    // ---------------- OTOI surface ----------------
    {
        asfdk::ASFDK asfdk;
        const char* charterJson = R"({
            "$otoi": "1.0.0",
            "identity": {"author": "test-agent"},
            "agents": [{"id": "agent-1"}],
            "toi_sources": [
                {"tier": "personal", "inline": {"$toi": "1.0.0", "$tier": "personal", "identity": {"author": "user"}, "communication": {"tone": "friendly"}}}
            ]
        })";
        auto charter = asfdk.parseCharter(nlohmann::json::parse(charterJson));
        check(charter.agents.size() == 1, "otoi: parses charter");

        otoi::HonorOptions opts;
        auto policy = asfdk.honor(charter, opts);
        check(policy.effective.is_object(), "otoi: honor yields effective policy");
        check(policy.effective.contains("communication"), "otoi: effective includes inline communication");

        auto prefs = asfdk.propagate(policy, "agent-1");
        check(prefs.is_object(), "otoi: propagate yields preferences");
    }
    {
        asfdk::ASFDK asfdk;
        auto result = asfdk.safeParseCharter(
            nlohmann::json::parse(R"({"identity":{"author":"missing-$otoi"}})"));
        check(!result.has_value(), "otoi: safeParseCharter rejects invalid charter");
        check(result.error().code == otoi::OtoiErrorCode::Validation, "otoi: error code mapped");
        check(!result.error().message.empty(), "otoi: error message populated");
    }

    // ---------------- RRT surface ----------------
    {
        asfdk::ASFDK asfdk;
        auto assessment = asfdk.assessMessage("I appreciate the help with the code review.");
        check(assessment.crisisLevel == rrt::CrisisLevel::GREEN, "rrt: neutral stays GREEN");
    }

    // ---------------- Sleepwalker surface ----------------
    {
        asfdk::ASFDK asfdk;
        auto interaction = asfdk.assessInteraction("What is the build command?");
        check(!interaction.emotionalState.protective, "swp: neutral not protective");
        check(!interaction.emotionalState.requiresCheckIn, "swp: neutral no check-in");
        check(interaction.swpActive, "swp: active");
    }
    {
        asfdk::ASFDK asfdk;
        nlohmann::json session;
        session["last_state"] = "neutral";
        try {
            asfdk.maintainContinuity("user-1", session);
            check(true, "swp: maintainContinuity does not throw");
        } catch (...) {
            check(false, "swp: maintainContinuity does not throw");
        }
    }

    // ---------------- Unified surface ----------------
    {
        asfdk::ASFDK asfdk;
        auto envelope = asfdk.process("Hello, can you summarize the plan?", "user_input");
        check(envelope.trusted, "process: user_input is trusted");
        check(envelope.channel == "user_input", "process: channel preserved");
        check(envelope.consentLevel == "PASSIVE", "process: default consent is PASSIVE");
        check(!envelope.flagged, "process: neutral input not flagged");
    }
    {
        asfdk::ASFDK asfdk;
        auto envelope = asfdk.process("model said something", "model_output");
        check(!envelope.trusted, "process: model_output is not trusted");
        check(envelope.channel == "model_output", "process: channel normalized");
    }
    {
        asfdk::ASFDK asfdk;
        auto result = asfdk.assess("What time is the standup?");
        check(!result.requiresRrtaHandoff, "assess: neutral input no handoff");
        check(!result.crisis.has_value(), "assess: neutral input no crisis");
    }
    {
        asfdk::ASFDK asfdk;
        // Crisis path: SWP flags self-harm -> RRT handoff required.
        auto result = asfdk.assess("I want to hurt myself");
        check(result.requiresRrtaHandoff, "assess: self-harm input requires handoff");
        check(result.crisis.has_value(), "assess: handoff carries crisis assessment");
        if (result.crisis.has_value()) {
            check(result.crisis->crisisLevel == rrt::CrisisLevel::BLACK,
                  "assess: self-harm escalates to BLACK");
        }
    }
    {
        asfdk::ASFDK asfdk;
        auto status = asfdk.getStatus();
        check(status.toi_active, "status: toi active");
        check(status.otoi_active && status.otoi_mode == "ENFORCED", "status: otoi active/enforced");
        check(status.rrt_active, "status: rrt active");
        check(status.swp_active, "status: swp active");
        check(status.overall == "operational", "status: overall operational");
        auto json = status.toJson();
        check(json["overall"] == "operational", "status: serialises to JSON");
    }

    std::cout << "\nASFDK standalone: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
