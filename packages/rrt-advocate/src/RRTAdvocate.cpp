#include "rrt/RRTAdvocate.h"

namespace rrt {

RRTAdvocate::RRTAdvocate(const std::string& userId) : userId_(userId) {
    crisisEngine_ = std::make_unique<CrisisEngine>(userId);
    burnoutDetector_ = std::make_unique<BurnoutDetector>(userId);
}

RRTAdvocate::~RRTAdvocate() = default;

CrisisAssessment RRTAdvocate::assessMessage(const std::string& message) {
    auto assessment = crisisEngine_->assess(message);
    currentCrisis_ = assessment;
    burnoutDetector_->recordCrisisLevel(assessment.crisisLevel);
    return assessment;
}

CrisisAssessment RRTAdvocate::assessCurrentState() {
    return assessMessage("");
}

CrisisIndicators RRTAdvocate::detectIndicators(const std::string& message) {
    return crisisEngine_->detect(message);
}

BurnoutAssessment RRTAdvocate::assessBurnout(const SessionMetrics& metrics) {
    return burnoutDetector_->assessBurnout(metrics);
}

void RRTAdvocate::resetSession() {
    crisisEngine_->resetSession();
}

void RRTAdvocate::reset() {
    crisisEngine_->resetSession();
    burnoutDetector_->reset();
    currentCrisis_ = CrisisAssessment{};
    monitoringActive_ = false;
}

nlohmann::json RRTAdvocate::getStatusReport() const {
    nlohmann::json report;
    report["user_id"] = userId_;
    report["monitoring_active"] = monitoringActive_;
    report["current_crisis"] = {
        {"level", crisisLevelToString(currentCrisis_.crisisLevel)},
        {"confidence", currentCrisis_.confidenceScore},
    };
    report["active_interventions"] = 0;
    report["crisis_history_count"] = 0;
    report["performance"] = {
        {"avg_response_time", 0.0},
        {"success_rate", 0.0},
        {"last_assessment", nullptr},
    };
    return report;
}

} // namespace rrt