#include "rrt/CrisisEngine.h"

namespace rrt {

CrisisEngine::CrisisEngine(const std::string& userId, const CrisisEngineOptions& options) {
    detector_ = std::make_unique<CrisisDetector>();
    assessor_ = std::make_unique<CrisisAssessor>(userId);
}

CrisisIndicators CrisisEngine::detect(const std::string& message) {
    return detector_->detectCrisisIndicators(message);
}

CrisisAssessment CrisisEngine::assess(const std::string& message) {
    // Use a fresh detector to avoid behavioral layer state pollution
    // from prior detect() calls affecting sentiment trend classification
    CrisisDetector freshDetector;
    auto indicators = freshDetector.detectCrisisIndicators(message);
    return assessor_->assessCrisis(indicators);
}

void CrisisEngine::resetSession() {
    detector_->resetSession();
}

} // namespace rrt