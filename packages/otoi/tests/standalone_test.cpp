#include "otoi/OTOIManager.h"
#include "otoi/OTOITypes.h"
#include <iostream>

int main() {
    int passed = 0, failed = 0;
    auto check = [&](bool cond, const char* name) {
        if (cond) { ++passed; std::cout << "PASS: " << name << "\n"; }
        else { ++failed; std::cout << "FAIL: " << name << "\n"; }
    };

    // Test 1: parse valid charter
    {
        std::string json = R"({
            "$otoi": "1.0.0",
            "$id": "test-charter",
            "identity": {"author": "John Doe"},
            "agents": [{"id": "agent-1"}, {"id": "agent-2"}],
            "toi_sources": [
                {"tier": "personal", "inline": {"$toi": "1.0.0", "$tier": "personal", "communication": {"tone": "friendly"}}},
                {"tier": "project", "inline": {"$toi": "1.0.0", "$tier": "project", "communication": {"tone": "formal"}}}
            ]
        })";
        otoi::OTOIManager mgr;
        auto result = mgr.safeParseCharter(nlohmann::json::parse(json));
        check(result.has_value(), "parse charter: succeeds");
        if (result.has_value()) {
            check(result.value().$otoi == "1.0.0", "parse charter: $otoi");
            check(result.value().$id.has_value() && result.value().$id == "test-charter", "parse charter: $id");
            check(result.value().agents.size() == 2, "parse charter: agents");
        }
    }

    // Test 2: safeHonor with inline docs
    {
        std::string json = R"({
            "$otoi": "1.0.0",
            "identity": {"author": "John Doe"},
            "agents": [{"id": "agent-1"}],
            "toi_sources": [
                {"tier": "personal", "inline": {"$toi": "1.0.0", "$tier": "personal", "communication": {"tone": "friendly"}}},
                {"tier": "project", "inline": {"$toi": "1.0.0", "$tier": "project", "communication": {"tone": "formal"}}}
            ]
        })";
        otoi::OTOIManager mgr;
        auto charter = mgr.safeParseCharter(nlohmann::json::parse(json));
        check(charter.has_value(), "honor: parse charter");
        if (charter.has_value()) {
            otoi::HonorOptions opts;
            auto policy = mgr.safeHonor(*charter, opts);
            check(policy.has_value(), "honor: safeHonor succeeds");
            if (policy.has_value()) {
                check(policy.value().effective.contains("communication"), "honor: effective has communication");
            }
        }
    }

    // Test 3: invalid version fails
    {
        std::string json = R"({"$otoi": "2.0.0", "identity": {"author": "x"}, "agents": []})";
        otoi::OTOIManager mgr;
        auto result = mgr.safeParseCharter(nlohmann::json::parse(json));
        check(!result.has_value(), "invalid version: fails");
    }

    std::cout << "\n=== OTOI Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed > 0 ? 1 : 0;
}