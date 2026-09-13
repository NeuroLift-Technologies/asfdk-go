#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "otoi/OTOITypes.h"
#include "otoi/OTOIManager.h"

using namespace otoi;

TEST_CASE("OTOI: parse minimal charter", "[otoi]") {
    const std::string json = R"({
        "$otoi": "1.0.0",
        "identity": {"author": "test-agent"},
        "agents": [{"id": "agent-1"}],
        "toi_sources": [
            {"tier": "personal", "inline": {"$toi": "1.0.0", "$tier": "personal", "identity": {"author": "user"}}}
        ]
    })";

    OTOIManager manager;
    auto result = manager.safeParseCharter(nlohmann::json::parse(json));
    REQUIRE(result.has_value());
    REQUIRE(result.value().$otoi == "1.0.0");
    REQUIRE(result.value().identity->at("author") == "test-agent");
    REQUIRE(result.value().agents.size() == 1);
    REQUIRE(result.value().agents[0].id == "agent-1");
    REQUIRE(result.value().toi_sources.size() == 1);
    REQUIRE(result.value().toi_sources[0].tier == Tier::Personal);
}

TEST_CASE("OTOI: parse charter with enforcement", "[otoi]") {
    const std::string json = R"({
        "$otoi": "1.0.0",
        "identity": {"author": "test-agent"},
        "agents": [{"id": "agent-1", "role": "assistant"}],
        "enforcement": {
            "mode": "strict",
            "on_conflict": "escalate",
            "on_unsupported": "reject",
            "audit": true
        },
        "toi_sources": [
            {"tier": "personal", "inline": {"$toi": "1.0.0", "$tier": "personal", "identity": {"author": "user"}}}
        ]
    })";

    OTOIManager manager;
    auto result = manager.safeParseCharter(nlohmann::json::parse(json));
    REQUIRE(result.has_value());
    REQUIRE(result.value().enforcement.has_value());
    REQUIRE(result.value().enforcement->mode == EnforcementMode::Strict);
    REQUIRE(result.value().enforcement->on_conflict == ConflictStrategy::Escalate);
    REQUIRE(result.value().enforcement->on_unsupported == UnsupportedStrategy::Reject);
    REQUIRE(result.value().enforcement->audit == true);
}

TEST_CASE("OTOI: parse charter missing required $otoi", "[otoi]") {
    const std::string json = R"({
        "identity": {"author": "test-agent"}
    })";

    OTOIManager manager;
    auto result = manager.safeParseCharter(nlohmann::json::parse(json));
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == OtoiErrorCode::Validation);
}

TEST_CASE("OTOI: parse invalid JSON", "[otoi]") {
    const std::string json = "not valid json";

    OTOIManager manager;
    auto result = manager.safeParseCharter(nlohmann::json::parse(json));
    // json::parse will throw on invalid JSON, so this test validates that path
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("OTOI: honor charter with single source", "[otoi]") {
    const std::string charterJson = R"({
        "$otoi": "1.0.0",
        "identity": {"author": "test-agent"},
        "agents": [{"id": "agent-1"}],
        "toi_sources": [
            {"tier": "personal", "inline": {"$toi": "1.0.0", "$tier": "personal", "identity": {"author": "user"}}}
        ]
    })";

    OTOIManager manager;
    auto charter = manager.parseCharter(nlohmann::json::parse(charterJson));

    HonorOptions options;
    options.documents = std::vector<nlohmann::json>();
    options.documents->push_back(nlohmann::json::parse(R"({"$toi":"1.0.0","$tier":"personal","identity":{"author":"user"}})");

    auto policy = manager.honor(charter, options);
    REQUIRE(policy.effective.contains("identity"));
    REQUIRE(policy.effective["identity"]["author"] == "user");
    REQUIRE(policy.tiers.size() == 1);
    REQUIRE(policy.tiers[0] == Tier::Personal);
    REQUIRE(policy.agents.size() == 1);
    REQUIRE(policy.agents[0].id == "agent-1");
}

TEST_CASE("OTOI: honor charter with tier precedence", "[otoi]") {
    const std::string charterJson = R"({
        "$otoi": "1.0.0",
        "identity": {"author": "test-agent"},
        "agents": [{"id": "agent-1"}],
        "toi_sources": [
            {"tier": "personal", "inline": {"$toi": "1.0.0", "$tier": "personal", "identity": {"author": "personal-user"}}},
            {"tier": "community", "inline": {"$toi": "1.0.0", "$tier": "community", "identity": {"author": "community-user"}}}
        ]
    })";

    OTOIManager manager;
    auto charter = manager.parseCharter(nlohmann::json::parse(charterJson));

    HonorOptions options;
    auto policy = manager.honor(charter, options);
    REQUIRE(policy.effective["identity"]["author"] == "personal-user");
    REQUIRE(policy.tiers.size() == 2);
    REQUIRE(policy.tiers[0] == Tier::Personal);
    REQUIRE(policy.tiers[1] == Tier::Community);
}

TEST_CASE("OTOI: detect same-tier conflicts", "[otoi]") {
    const std::string json = R"({
        "$otoi": "1.0.0",
        "identity": {"author": "test-agent"},
        "agents": [{"id": "agent-1"}],
        "toi_sources": [
            {"tier": "personal", "inline": {"$toi": "1.0.0", "$tier": "personal", "communication": {"tone": "formal"}}},
            {"tier": "personal", "inline": {"$toi": "1.0.0", "$tier": "personal", "communication": {"tone": "casual"}}}
        ]
    })";

    OTOIManager manager;
    auto charter = manager.parseCharter(nlohmann::json::parse(json));

    OTOIValidator validator;
    std::vector<nlohmann::json> documents;
    for (const auto& source : charter.toi_sources) {
        if (source.inline) documents.push_back(*source.inline);
    }

    auto conflicts = validator.detectConflicts(documents);
    REQUIRE(conflicts.size() == 1);
    REQUIRE(conflicts[0].path == "communication.tone");
    REQUIRE(conflicts[0].values.size() == 2);
}

TEST_CASE("OTOI: propagate refuses undeclared agent in strict mode", "[otoi]") {
    const std::string charterJson = R"({
        "$otoi": "1.0.0",
        "identity": {"author": "test-agent"},
        "agents": [{"id": "agent-1"}],
        "enforcement": {"mode": "strict"},
        "toi_sources": [
            {"tier": "personal", "inline": {"$toi": "1.0.0", "$tier": "personal", "identity": {"author": "user"}}}
        ]
    })";

    OTOIManager manager;
    auto charter = manager.parseCharter(nlohmann::json::parse(charterJson));
    HonorOptions options;
    auto policy = manager.honor(charter, options);

    nlohmann::json result = manager.propagate(policy, "unknown-agent");
    REQUIRE(result.is_object());
    REQUIRE(result.empty());
}

TEST_CASE("OTOI: enum conversions", "[otoi]") {
    REQUIRE(to_string(Tier::Personal) == "personal");
    REQUIRE(to_string(Tier::Community) == "community");
    REQUIRE(to_string(Tier::Project) == "project");
    REQUIRE(tier_from_string("personal") == Tier::Personal);
    REQUIRE(tier_from_string("invalid") == std::nullopt);

    REQUIRE(to_string(EnforcementMode::Advisory) == "advisory");
    REQUIRE(to_string(EnforcementMode::Enforced) == "enforced");
    REQUIRE(to_string(EnforcementMode::Strict) == "strict");

    REQUIRE(to_string(ConflictStrategy::HighestTierWins) == "highest-tier-wins");
    REQUIRE(to_string(ConflictStrategy::Reject) == "reject");
    REQUIRE(to_string(ConflictStrategy::Escalate) == "escalate");

    REQUIRE(to_string(UnsupportedStrategy::Ignore) == "ignore");
    REQUIRE(to_string(UnsupportedStrategy::Degrade) == "degrade");
    REQUIRE(to_string(UnsupportedStrategy::Reject) == "reject");
}
