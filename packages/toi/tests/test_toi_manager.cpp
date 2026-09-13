#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "toi/TOITypes.h"
#include "toi/TermsOfInteraction.h"

TEST_CASE("TOI: parse minimal document", "[toi]") {
    const std::string json = R"({"$toi":"1.0.0","$tier":"personal","identity":{"author":"anonymous"}})";
    auto doc = toi::parseTOI(nlohmann::json::parse(json));
    REQUIRE(doc.$toi == "1.0.0");
    REQUIRE(doc.$tier == "personal");
    REQUIRE(doc.identity.author == "anonymous");
}

TEST_CASE("TOI: parse full document", "[toi]") {
    const std::string json = R"({
        "$toi":"1.0.0",
        "$tier":"personal",
        "$created":"2024-01-15T10:30:00Z",
        "$id":"c2c496e4-f3e2-4a56-b33a-1234567890ab",
        "identity":{"author":"John Doe","handle":"johndoe"},
        "cognitive_profile":{"self_described":"I like parallel work"},
        "privacy":{"retention":"user-controlled"},
        "agency":{"task_initiation":"user-initiated"},
        "communication":{"tone":"friendly"},
        "ethical_pillars":["privacy-by-default"]
    })";
    auto doc = toi::parseTOI(nlohmann::json::parse(json));
    REQUIRE(doc.$toi == "1.0.0");
    REQUIRE(doc.$tier == "personal");
    REQUIRE(doc.identity.author == "John Doe");
    REQUIRE(doc.identity.handle == "johndoe");
    REQUIRE(doc.$created == "2024-01-15T10:30:00Z");
    REQUIRE(doc.$id == "c2c496e4-f3e2-4a56-b33a-1234567890ab");
    REQUIRE(doc.cognitive_profile->self_described == "I like parallel work");
    REQUIRE(doc.cognitive_profile->thread_support == false); // default
    REQUIRE(doc.privacy->retention == "user-controlled");
    REQUIRE(doc.agency->task_initiation == "user-initiated");
    REQUIRE(doc.communication->tone == toi::Tone::Friendly);
    REQUIRE(doc.ethical_pillars.size() == 1);
    REQUIRE(doc.ethical_pillars[0] == "privacy-by-default");
}

TEST_CASE("TOI: validate minimal document", "[toi]") {
    const std::string json = R"({"$toi":"1.0.0","$tier":"personal","identity":{"author":"anonymous"}})";
    auto doc = toi::parseTOI(nlohmann::json::parse(json));
    REQUIRE(toi::validateTOI(doc) == true);
}

TEST_CASE("TOI: validate invalid $toi version", "[toi]") {
    const std::string json = R"({"$toi":"2.0.0","$tier":"personal","identity":{"author":"anonymous"}})";
    // parseTOI throws on invalid version (defensive: version is a hard gate)
    REQUIRE_THROWS_AS(toi::parseTOI(nlohmann::json::parse(json)), std::invalid_argument);
    // safeParseTOI returns expected error without throwing
    auto result = toi::safeParseTOI(nlohmann::json::parse(json));
    REQUIRE(!result.has_value());
    REQUIRE(result.error().code == toi::TOIError::Code::VersionMismatch);
}

TEST_CASE("TOI: validate missing identity.author", "[toi]") {
    const std::string json = R"({"$toi":"1.0.0","$tier":"personal})";
    REQUIRE_THROWS(toi::parseTOI(nlohmann::json::parse(json)));
}

TEST_CASE("TOI: tier precedence resolution", "[toi]") {
    toi::TOIDocument personal; personal.$toi = "1.0.0"; personal.$tier = "personal"; personal.identity.author = "anonymous";
    toi::TOIDocument community; community.$toi = "1.0.0"; community.$tier = "community"; community.identity.author = "bob";
    toi::TOIDocument project; project.$toi = "1.0.0"; project.$tier = "project"; project.identity.author = "charlie";

    // personal should win over community over project
    auto resolved = toi::resolveTOI({personal, community, project});
    REQUIRE(resolved.$tier == "personal");
}

TEST_CASE("TOI: ethical_pillars parsing", "[toi]") {
    const std::string json = R"({"$toi":"1.0.0","$tier":"personal","identity":{"author":"x"},"ethical_pillars":["privacy-by-default","user-agency"]})";
    auto doc = toi::parseTOI(nlohmann::json::parse(json));
    REQUIRE(doc.ethical_pillars.size() == 2);
    REQUIRE(doc.ethical_pillars[0] == "privacy-by-default");
    REQUIRE(doc.ethical_pillars[1] == "user-agency");
}