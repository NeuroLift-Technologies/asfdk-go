#pragma once

#include <string>
#include <vector>
#include <memory>

namespace asfdk {

/**
 * @brief Core types used across the ASFDK umbrella for 
 * cross-pillar communication and unified API responses.
 */
struct GovernanceResult {
    bool success;
    std::string message;
    int errorCode;
};

enum class GovernanceLevel {
    Informational,
    Warning,
    Critical,
    Crisis
};

} // namespace asfdk