#pragma once

#include "rrt/RRTTypes.h"
#include "rrt/CrisisEngine.h"
#include <deque>

namespace rrt {

struct SessionMetrics {
    double duration = 0.0;        // Session duration in minutes
    int messageCount = 0;
    double avgResponseTime = 0.0; // Average response time in seconds
    int errorCount = 0;
    double taskCompletionRate = 1.0; // 0.0 to 1.0
};

class BurnoutDetector {
public:
    explicit BurnoutDetector(const std::string& userId);
    BurnoutAssessment assessBurnout(const SessionMetrics& metrics);
    void recordCrisisLevel(CrisisLevel level);
    void reset();

private:
    BurnoutLevel mapScoreToLevel(double score) const;
    std::vector<BurnoutIndicator> computeIndicators(const SessionMetrics& metrics);
    std::vector<std::string> generateRecommendations(BurnoutLevel level, const std::vector<BurnoutIndicator>& indicators) const;

    std::string userId_;
    std::deque<CrisisLevel> recentCrisisLevels_;
    static constexpr size_t MAX_HISTORY = 20;
};

} // namespace rrt