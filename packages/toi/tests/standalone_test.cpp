#include "toi/TermsOfInteraction.h"
#include <iostream>
#include <stdexcept>
#include <string>

int main() {
    int passed = 0, failed = 0;
    auto check = [&](bool cond, const char* name) {
        if (cond) { ++passed; std::cout << "PASS: " << name << "\n"; }
        else { ++failed; std::cout << "FAIL: " << name << "\n"; }
    };

    // Test 1: parse minimal document
    {
        std::string json = R"({"$toi":"1.0.0","$tier":"personal","identity":{"author":"anonymous"}})";
        auto doc = toi::parseTOI(nlohmann::json::parse(json));
        check(doc.$toi == "1.0.0", "parse minimal: $toi");
        check(doc.$tier == "personal", "parse minimal: $tier");
        check(doc.identity.author == "anonymous", "parse minimal: author");
    }

    // Test 2: parse full document
    {
        std::string json = R"({
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
        check(doc.$toi == "1.0.0", "parse full: $toi");
        check(doc.$tier == "personal", "parse full: $tier");
        check(doc.identity.author == "John Doe", "parse full: author");
        check(doc.identity.handle == "johndoe", "parse full: handle");
        check(doc.$created == "2024-01-15T10:30:00Z", "parse full: $created");
        check(doc.$id == "c2c496e4-f3e2-4a56-b33a-1234567890ab", "parse full: $id");
        check(doc.cognitive_profile.has_value() &&
              doc.cognitive_profile->self_described == "I like parallel work",
              "parse full: cognitive_profile");
        check(doc.privacy.has_value() && doc.privacy->retention == "user-controlled",
              "parse full: privacy retention");
        check(doc.agency.has_value() && doc.agency->task_initiation == "user-initiated",
              "parse full: agency");
        check(doc.communication.has_value() && doc.communication->tone == toi::Tone::Friendly,
              "parse full: communication tone");
        check(doc.ethical_pillars.size() == 1, "parse full: ethical_pillars size");
        check(doc.ethical_pillars[0] == "privacy-by-default", "parse full: ethical_pillars[0]");
    }

    // Test 3: validate minimal document
    {
        std::string json = R"({"$toi":"1.0.0","$tier":"personal","identity":{"author":"anonymous"}})";
        auto doc = toi::parseTOI(nlohmann::json::parse(json));
        check(toi::validateTOI(doc) == true, "validate minimal: valid");
    }

    // Test 4: invalid version throws
    {
        std::string json = R"({"$toi":"2.0.0","$tier":"personal","identity":{"author":"anonymous"}})";
        bool threw = false;
        try { toi::parseTOI(nlohmann::json::parse(json)); }
        catch (const std::invalid_argument&) { threw = true; }
        check(threw, "invalid version: throws");
    }

    // Test 5: invalid tier throws
    {
        std::string json = R"({"$toi":"1.0.0","$tier":"galactic","identity":{"author":"anonymous"}})";
        bool threw = false;
        try { toi::parseTOI(nlohmann::json::parse(json)); }
        catch (const std::invalid_argument&) { threw = true; }
        check(threw, "invalid tier: throws");
    }

    // Test 6: safeParseTOI success and error codes
    {
        std::string ok = R"({"$toi":"1.0.0","$tier":"personal","identity":{"author":"a"}})";
        auto okRes = toi::safeParseTOI(nlohmann::json::parse(ok));
        check(okRes.has_value(), "safeParseTOI: success has_value");

        std::string badVersion = R"({"$toi":"9.9.9","$tier":"personal","identity":{"author":"a"}})";
        auto vRes = toi::safeParseTOI(nlohmann::json::parse(badVersion));
        check(!vRes.has_value() &&
              vRes.error().code == toi::TOIError::Code::VersionMismatch,
              "safeParseTOI: VersionMismatch");

        std::string badTier = R"({"$toi":"1.0.0","$tier":"galactic","identity":{"author":"a"}})";
        auto tRes = toi::safeParseTOI(nlohmann::json::parse(badTier));
        check(!tRes.has_value() &&
              tRes.error().code == toi::TOIError::Code::InvalidTier,
              "safeParseTOI: InvalidTier");

        std::string noAuthor = R"({"$toi":"1.0.0","$tier":"personal","identity":{}})";
        auto aRes = toi::safeParseTOI(nlohmann::json::parse(noAuthor));
        check(!aRes.has_value() &&
              aRes.error().code == toi::TOIError::Code::MissingAuthor,
              "safeParseTOI: MissingAuthor");
    }

    // Test 7: tier precedence — highest-precedence doc first, personal wins
    {
        toi::TOIDocument personal{"1.0.0", "personal"};
        toi::TOIDocument community{"1.0.0", "community"};
        toi::TOIDocument project{"1.0.0", "project"};
        auto resolved = toi::resolveTOI({personal, community, project});
        check(resolved.$tier == "personal", "tier precedence: personal wins");
    }

    // Test 8: resolveTOI with empty stack falls back to project tier
    {
        auto resolved = toi::resolveTOI({});
        check(resolved.$toi == "1.0.0" && resolved.$tier == "project",
              "tier precedence: empty stack defaults to project");
    }

    // Test 9: canonicalize is currently the identity function (placeholder)
    {
        std::string json = R"({"$toi":"1.0.0","$tier":"personal"})";
        check(toi::canonicalize(json) == json, "canonicalize: identity placeholder");
    }

    std::cout << "\n=== TOI Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed > 0 ? 1 : 0;
}
