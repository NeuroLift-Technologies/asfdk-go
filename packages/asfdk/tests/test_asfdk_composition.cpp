#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

#include <asfdk/ASFDK.h>
#include <asfdk/ASFDKTypes.h>

using namespace asfdk;

namespace {

// Minimal valid .toi document (same fixture used by packages/toi tests).
const char* kMinimalToi = R"({"$toi":"1.0.0","$tier":"personal","identity":{"author":"anonymous"}})";

// Minimal .otoi charter with one inline personal source.
const char* kMinimalCharter = R"({
    "$otoi": "1.0.0",
    "identity": {"author": "test-agent"},
    "agents": [{"id": "agent-1"}],
    "toi_sources": [
        {"tier": "personal", "inline": {"$toi": "1.0.0", "$tier": "personal", "identity": {"author": "user"}}}
    ]
})";

} // namespace

// ===================== TOI composition =====================

TEST_CASE("ASFDK: parses and validates a TOI document through the umbrella", "[asfdk][toi]") {
    ASFDK asfdk;
    auto doc = asfdk.parseTOI(nlohmann::json::parse(kMinimalToi));
    REQUIRE(doc.$toi == "1.0.0");
    REQUIRE(doc.$tier == "personal");
    REQUIRE(doc.identity.author == "anonymous");
    REQUIRE(asfdk.validateTOI(doc) == true);
}

TEST_CASE("ASFDK: safeParseTOI returns nullopt on invalid version", "[asfdk][toi]") {
    ASFDK asfdk;
    const auto bad = R"({"$toi":"2.0.0","$tier":"personal","identity":{"author":"anonymous"}})";
    std::string error;
    auto doc = asfdk.safeParseTOI(nlohmann::json::parse(bad), &error);
    REQUIRE_FALSE(doc.has_value());
    REQUIRE_FALSE(error.empty());
}

TEST_CASE("ASFDK: resolves a TOI stack by tier precedence through the umbrella", "[asfdk][toi]") {
    ASFDK asfdk;
    auto personal = asfdk.parseTOI(nlohmann::json::parse(
        R"({"$toi":"1.0.0","$tier":"personal","identity":{"author":"user"},"communication":{"tone":"friendly"}})"));
    auto project = asfdk.parseTOI(nlohmann::json::parse(
        R"({"$toi":"1.0.0","$tier":"project","identity":{"author":"platform"},"communication":{"tone":"professional"}})"));
    auto resolved = asfdk.resolveTOI({personal, project});
    // Personal tier wins for tone (gap-filling: higher tier replaces wholesale).
    REQUIRE(resolved.identity.author == "user");
    REQUIRE(resolved.communication.has_value());
}

// ===================== OTOI composition =====================

TEST_CASE("ASFDK: parses and honors a charter through the umbrella", "[asfdk][otoi]") {
    ASFDK asfdk;
    auto charter = asfdk.parseCharter(nlohmann::json::parse(kMinimalCharter));
    REQUIRE(charter.agents.size() == 1);

    otoi::HonorOptions options;
    options.documents = std::vector<nlohmann::json>();
    options.documents->push_back(nlohmann::json::parse(kMinimalToi));

    auto policy = asfdk.honor(charter, options);
    REQUIRE(policy.effective["identity"]["author"] == "anonymous");
    REQUIRE(policy.effective["identity"]["author"] == "user" || policy.effective["identity"]["author"] == "anonymous");

    auto prefs = asfdk.propagate(policy, "agent-1");
    REQUIRE(prefs.is_object());
}

TEST_CASE("ASFDK: safeParseCharter reports umbrella error code", "[asfdk][otoi]") {
    ASFDK asfdk;
    const auto bad = R"({"identity":{"author":"missing-$otoi"}})";
    ASFDKError error;
    auto charter = asfdk.safeParseCharter(nlohmann::json::parse(bad), &error);
    REQUIRE_FALSE(charter.has_value());
    REQUIRE(error.code != ASFDKError::Code::UnknownError);
    REQUIRE_FALSE(error.message.empty());
}

// ===================== RRT composition =====================

TEST_CASE("ASFDK: neutral message yields stable crisis state", "[asfdk][rrt]") {
    ASFDK asfdk;
    auto assessment = asfdk.assessMessage("I appreciate the help with the code review.");
    REQUIRE(assessment.crisisLevel == rrt::CrisisLevel::GREEN);
}

// ===================== Sleepwalker composition =====================

TEST_CASE("ASFDK: neutral interaction does not flag", "[asfdk][swp]") {
    ASFDK asfdk;
    auto interaction = asfdk.assessInteraction("Just a normal question about CMake.");
    REQUIRE_FALSE(interaction.emotionalState.protective);
    REQUIRE_FALSE(interaction.emotionalState.requiresCheckIn);
}

TEST_CASE("ASFDK: maintainContinuity does not throw", "[asfdk][swp]") {
    ASFDK asfdk;
    nlohmann::json session;
    session["last_state"] = "neutral";
    REQUIRE_NOTHROW(asfdk.maintainContinuity("user-1", session));
}

// ===================== Unified surface =====================

TEST_CASE("ASFDK: process builds a trusted envelope for user_input", "[asfdk][unified]") {
    ASFDK asfdk;
    auto envelope = asfdk.process("Hello, can you summarize the plan?", "user_input");
    REQUIRE(envelope.trusted == true);
    REQUIRE(envelope.channel == "user_input");
    REQUIRE(envelope.consentLevel == "PASSIVE");
}

TEST_CASE("ASFDK: process never trusts a non-user channel", "[asfdk][unified]") {
    ASFDK asfdk;
    auto envelope = asfdk.process("model said something", "model_output");
    REQUIRE(envelope.trusted == false);
    REQUIRE(envelope.channel == "model_output");
}

TEST_CASE("ASFDK: assess reports no handoff for neutral input", "[asfdk][unified]") {
    ASFDK asfdk;
    auto result = asfdk.assess("What time is the standup?");
    REQUIRE_FALSE(result.requiresRrtaHandoff);
    REQUIRE_FALSE(result.crisis.has_value());
}

TEST_CASE("ASFDK: foundation status reports operational", "[asfdk][unified]") {
    ASFDK asfdk;
    auto status = asfdk.getStatus();
    REQUIRE(status.toi_active == true);
    REQUIRE(status.otoi_active == true);
    REQUIRE(status.swp_active == true);
    REQUIRE(status.overall == "operational");

    auto json = status.toJson();
    REQUIRE(json["overall"] == "operational");
}