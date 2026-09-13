#pragma once

#include "rrt/RRTTypes.h"

namespace rrt {

class CrisisAssessor {
public:
    explicit CrisisAssessor(const std::string& userId);
    CrisisAssessment assessCrisis(const CrisisIndicators& indicators) const;

private:
    CrisisLevel mapConfidenceToLevel(double confidence) const;
    double computeSafetyScore(const CrisisIndicators& indicators, CrisisLevel level) const;
    std::vector<std::string> getRecommendedInterventions(CrisisLevel level) const;
    double getEscalationThreshold(CrisisLevel level) const;

    std::string userId_;

    struct LevelThreshold { double low; double high; CrisisLevel level; };
    static const std::vector<LevelThreshold> LEVEL_THRESHOLDS;
    static const std::unordered_map<CrisisLevel, std::vector<std::string>> DEFAULT_INTERVENTIONS;
    static const std::unordered_map<CrisisLevel, double> ESCALATION_THRESHOLDS;
};

} // namespace rrt