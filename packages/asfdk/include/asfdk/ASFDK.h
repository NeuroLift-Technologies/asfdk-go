#pragma once

#include <string>
#include <memory>
#include <optional>
#include <vector>

// Include actual pillar headers
#include "toi/TOITypes.h"
#include "toi/TermsOfInteraction.h"
#include "otoi/OTOIManager.h"
#include "rrt/RRTAdvocate.h"
#include "rrt/RRTTypes.h"
#include "sleepwalker/SleepwalkerProtocol.h"

#include <nlohmann/json.hpp>
#include "asfdk/ASFDKTypes.h"

namespace asfdk {

struct Envelope {
    bool trusted;
    std::string channel;
    std::string consentLevel;
    nlohmann::json payload;
    bool flagged = false;
    std::string flagReason;
};

struct AssessmentResult {
    bool requiresRrtaHandoff;
    std::optional<rrt::CrisisAssessment> crisis;
    sleepwalker::EmotionalState emotionalState;
};

struct FoundationStatus {
    bool toi_active;
    bool otoi_active;
    bool rrt_active;
    bool swp_active;
    std::string otoi_mode;
    std::string overall;

    nlohmann::json toJson() const;
};

/**
 * @brief The ASFDK Umbrella class.
 * Composes the four pillars of the Solidarity Framework into a
 * single unified interface for the application.
 */
class ASFDK {
public:
    ASFDK();
    ~ASFDK();

    ASFDK(const ASFDK&) = delete;
    ASFDK& operator=(const ASFDK&) = delete;

    // ===================== TOI surface (free functions) =====================
    
    toi::TOIDocument parseTOI(const nlohmann::json& json);
    tl::expected<toi::TOIDocument, toi::TOIError> safeParseTOI(const nlohmann::json& json);
    bool validateTOI(const toi::TOIDocument& doc);
    toi::TOIDocument resolveTOI(std::vector<toi::TOIDocument> docs);
    std::string canonicalize(const std::string& json);

    // ===================== OTOI surface (pass-through to otoi::OTOIManager) =====================
    
    otoi::OtoiCharter parseCharter(const nlohmann::json& json);
    tl::expected<otoi::OtoiCharter, otoi::OtoiError> safeParseCharter(const nlohmann::json& json);
    otoi::EffectivePolicy honor(const otoi::OtoiCharter& charter, const otoi::HonorOptions& options = {});
    nlohmann::json propagate(const otoi::EffectivePolicy& policy, const std::string& agentId);

    // ===================== RRT surface (pass-through to rrt::RRTAdvocate) =====================
    
    rrt::CrisisAssessment assessMessage(const std::string& message);

    // ===================== Sleepwalker surface (pass-through to sleepwalker::SleepwalkerProtocol) =====================
    
    sleepwalker::InteractionAssessment assessInteraction(
        const std::string& userInput,
        const std::vector<std::string>& sessionHistory = {},
        const std::optional<std::string>& userId = std::nullopt
    );
    void maintainContinuity(const std::string& userId, nlohmann::json sessionData);

    // ===================== Unified governance surface =====================
    
    Envelope process(const std::string& input, const std::string& channel);
    AssessmentResult assess(const std::string& input);
    FoundationStatus getStatus();

private:
    std::unique_ptr<otoi::OTOIManager> m_otoi;
    std::unique_ptr<rrt::RRTAdvocate> m_rrt;
    std::unique_ptr<sleepwalker::SleepwalkerProtocol> m_sleepwalker;
};

} // namespace asfdk
