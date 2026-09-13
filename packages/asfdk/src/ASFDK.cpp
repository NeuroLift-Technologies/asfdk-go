#include "asfdk/ASFDK.h"
#include "asfdk/ASFDKTypes.h"

#include <iostream>
#include <nlohmann/json.hpp>

namespace asfdk {

ASFDK::ASFDK()
    : m_otoi(std::make_unique<otoi::OTOIManager>())
    , m_rrt(std::make_unique<rrt::RRTAdvocate>())
    , m_sleepwalker(std::make_unique<sleepwalker::SleepwalkerProtocol>())
{
}

ASFDK::~ASFDK() = default;

// ===================== TOI surface (free functions) =====================

toi::TOIDocument ASFDK::parseTOI(const nlohmann::json& json) {
    return toi::parseTOI(json);
}

tl::expected<toi::TOIDocument, toi::TOIError> ASFDK::safeParseTOI(const nlohmann::json& json) {
    return toi::safeParseTOI(json);
}

bool ASFDK::validateTOI(const toi::TOIDocument& doc) {
    return toi::validateTOI(doc);
}

toi::TOIDocument ASFDK::resolveTOI(std::vector<toi::TOIDocument> docs) {
    return toi::resolveTOI(std::move(docs));
}

std::string ASFDK::canonicalize(const std::string& json) {
    return toi::canonicalize(json);
}

// ===================== OTOI surface =====================

otoi::OtoiCharter ASFDK::parseCharter(const nlohmann::json& json) {
    return m_otoi->parseCharter(json);
}

tl::expected<otoi::OtoiCharter, otoi::OtoiError> ASFDK::safeParseCharter(const nlohmann::json& json) {
    return m_otoi->safeParseCharter(json);
}

otoi::EffectivePolicy ASFDK::honor(const otoi::OtoiCharter& charter, const otoi::HonorOptions& options) {
    return m_otoi->honor(charter, options);
}

nlohmann::json ASFDK::propagate(const otoi::EffectivePolicy& policy, const std::string& agentId) {
    return m_otoi->propagate(policy, agentId);
}

// ===================== RRT surface =====================

rrt::CrisisAssessment ASFDK::assessMessage(const std::string& message) {
    return m_rrt->assessMessage(message);
}

// ===================== Sleepwalker surface =====================

sleepwalker::InteractionAssessment ASFDK::assessInteraction(
    const std::string& userInput,
    const std::vector<std::string>& sessionHistory,
    const std::optional<std::string>& userId)
{
    return m_sleepwalker->assessInteraction(userInput, sessionHistory, userId);
}

void ASFDK::maintainContinuity(const std::string& userId, nlohmann::json sessionData) {
    m_sleepwalker->maintainContinuity(userId, sessionData);
}

// ===================== Unified governance surface =====================

Envelope ASFDK::process(const std::string& input, const std::string& channel) {
    Envelope env;
    env.channel = channel;
    env.trusted = (channel == "user_input");
    env.consentLevel = "PASSIVE";
    env.payload = nlohmann::json{{"input", input}, {"channel", channel}};

    // Check emotional state for flagging
    auto assessment = m_sleepwalker->assessInteraction(input);
    if (assessment.emotionalState.protective || assessment.emotionalState.requiresCheckIn) {
        env.flagged = true;
        env.flagReason = assessment.emotionalState.protective ? "protective_state" : "check_in_required";
    }

    return env;
}

AssessmentResult ASFDK::assess(const std::string& input) {
    AssessmentResult result;
    result.emotionalState = m_sleepwalker->detectEmotionalState(input);
    result.requiresRrtaHandoff = m_sleepwalker->requiresRrtaHandoff(result.emotionalState);

    if (result.requiresRrtaHandoff) {
        result.crisis = m_rrt->assessMessage(input);
    }

    return result;
}

FoundationStatus ASFDK::getStatus() {
    FoundationStatus status;
    status.toi_active = true;  // TOI is stateless, always active

    auto otoiStatus = m_otoi->getStatus();
    status.otoi_active = otoiStatus.active;
    status.otoi_mode = (otoiStatus.mode == otoi::EnforcementMode::Strict) ? "STRICT" : "ENFORCED";

    status.rrt_active = true;  // RRT is always active once constructed
    status.swp_active = true;  // SWP is always active once constructed

    status.overall = (status.otoi_active) ? "operational" : "degraded";
    return status;
}

nlohmann::json FoundationStatus::toJson() const {
    return nlohmann::json{
        {"toi_active", toi_active},
        {"otoi_active", otoi_active},
        {"rrt_active", rrt_active},
        {"swp_active", swp_active},
        {"otoi_mode", otoi_mode},
        {"overall", overall}
    };
}

} // namespace asfdk
