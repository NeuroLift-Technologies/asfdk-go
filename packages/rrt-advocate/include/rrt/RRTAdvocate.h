#pragma once

#include "rrt/RRTTypes.h"
#include "rrt/CrisisEngine.h"
#include "rrt/BurnoutDetector.h"
#include <memory>

namespace rrt {

class RRTAdvocate {
public:
    RRTAdvocate(const std::string& userId = "anonymous");
    ~RRTAdvocate();

    // Crisis assessment
    CrisisAssessment assessMessage(const std::string& message);
    CrisisAssessment assessCurrentState();
    CrisisIndicators detectIndicators(const std::string& message);

    // Burnout detection
    BurnoutAssessment assessBurnout(const SessionMetrics& metrics);

    // Session management
    void resetSession();
    void reset();
    nlohmann::json getStatusReport() const;

private:
    std::unique_ptr<CrisisEngine> crisisEngine_;
    std::unique_ptr<BurnoutDetector> burnoutDetector_;
    std::string userId_;
    bool monitoringActive_ = false;
    CrisisAssessment currentCrisis_;
};

} // namespace rrt