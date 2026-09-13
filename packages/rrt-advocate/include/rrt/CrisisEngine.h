#pragma once

#include "rrt/RRTTypes.h"
#include "rrt/CrisisDetector.h"
#include "rrt/CrisisAssessor.h"
#include <memory>

namespace rrt {

struct CrisisEngineOptions {
    std::string configPath;
};

class CrisisEngine {
public:
    explicit CrisisEngine(const std::string& userId, const CrisisEngineOptions& options = {});
    CrisisIndicators detect(const std::string& message = "");
    CrisisAssessment assess(const std::string& message = "");
    void resetSession();

private:
    std::unique_ptr<CrisisDetector> detector_;
    std::unique_ptr<CrisisAssessor> assessor_;
};

} // namespace rrt