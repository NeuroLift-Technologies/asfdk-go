#include "rrt/BurnoutDetector.h"
#include <algorithm>
#include <cmath>

namespace rrt {

BurnoutDetector::BurnoutDetector(const std::string& userId) : userId_(userId) {}

BurnoutAssessment BurnoutDetector::assessBurnout(const SessionMetrics& metrics) {
    auto indicators = computeIndicators(metrics);
    double totalScore = 0.0;
    for (const auto& ind : indicators) totalScore += ind.severity;
    double overallScore = indicators.empty() ? 0.0 : std::min(1.0, (totalScore + 0.1 * indicators.size()) / indicators.size());
    overallScore = std::round(overallScore * 1000.0) / 1000.0;
    auto level = mapScoreToLevel(overallScore);
    auto recommendations = generateRecommendations(level, indicators);

    BurnoutAssessment assessment;
    assessment.timestamp = std::chrono::system_clock::now();
    assessment.level = level;
    assessment.overallScore = overallScore;
    assessment.indicators = indicators;
    assessment.recommendations = recommendations;
    assessment.escalationRequired = (level == BurnoutLevel::SEVERE);
    return assessment;
}

void BurnoutDetector::recordCrisisLevel(CrisisLevel level) {
    recentCrisisLevels_.push_back(level);
    if (recentCrisisLevels_.size() > MAX_HISTORY) recentCrisisLevels_.pop_front();
}

void BurnoutDetector::reset() { recentCrisisLevels_.clear(); }

BurnoutLevel BurnoutDetector::mapScoreToLevel(double score) const {
    if (score < 0.35) return BurnoutLevel::NONE;
    if (score < 0.45) return BurnoutLevel::MILD;
    if (score < 0.75) return BurnoutLevel::MODERATE;
    return BurnoutLevel::SEVERE;
}

std::vector<BurnoutIndicator> BurnoutDetector::computeIndicators(const SessionMetrics& metrics) {
    std::vector<BurnoutIndicator> indicators;

    // Extended session duration indicator
    if (metrics.duration >= 120.0) {
        indicators.push_back({"extended_session", std::min(1.0, (metrics.duration - 120.0) / 120.0),
            "Session duration exceeds healthy limits"});
    }

    // High message volume indicator
    if (metrics.messageCount > 50) {
        indicators.push_back({"high_volume", std::min(1.0, (metrics.messageCount - 50.0) / 25.0),
            "High message volume detected"});
    }

    // Slow response time indicator
    if (metrics.avgResponseTime > 60.0) {
        indicators.push_back({"slow_response", std::min(1.0, (metrics.avgResponseTime - 60.0) / 60.0),
            "Response times are increasing"});
    }

    // Elevated crisis level frequency
    int elevatedCount = 0;
    for (const auto& level : recentCrisisLevels_) {
        if (level != CrisisLevel::GREEN) ++elevatedCount;
    }
    if (!recentCrisisLevels_.empty()) {
        double elevatedRatio = static_cast<double>(elevatedCount) / recentCrisisLevels_.size();
        if (elevatedRatio > 0.3) {
            indicators.push_back({"elevated_crisis_frequency", elevatedRatio,
                "Frequent elevated crisis levels detected"});
        }
    }

    // Low task completion rate
    if (metrics.taskCompletionRate <= 0.5) {
        indicators.push_back({"low_completion", (1.0 - metrics.taskCompletionRate) * 2.0,
            "Task completion rate is low"});
    }

    // Error frequency
    if (metrics.errorCount >= 3) {
        indicators.push_back({"high_error_rate", std::min(1.0, metrics.errorCount / 5.0),
            "High error rate detected"});
    }

    return indicators;
}

std::vector<std::string> BurnoutDetector::generateRecommendations(BurnoutLevel level, const std::vector<BurnoutIndicator>& indicators) const {
    std::vector<std::string> recs;
    switch (level) {
        case BurnoutLevel::NONE:
            recs.push_back("Continue current pace");
            break;
        case BurnoutLevel::MILD:
            recs.push_back("Take a short break");
            recs.push_back("Practice grounding techniques");
            break;
        case BurnoutLevel::MODERATE:
            recs.push_back("Take a 15-minute break");
            recs.push_back("Reduce task load");
            recs.push_back("Practice self-care activities");
            break;
        case BurnoutLevel::SEVERE:
            recs.push_back("Stop current activity");
            recs.push_back("Take an extended break");
            recs.push_back("Seek support from supervisor");
            recs.push_back("Consider ending session");
            break;
    }
    for (const auto& ind : indicators) {
        if (ind.name == "extended_session") recs.push_back("Consider taking a break from extended sessions");
        if (ind.name == "high_volume") recs.push_back("Reduce interaction frequency");
        if (ind.name == "slow_response") recs.push_back("Slow down and take time to respond");
        if (ind.name == "elevated_crisis_frequency") recs.push_back("Monitor stress levels closely");
        if (ind.name == "low_completion") recs.push_back("Break tasks into smaller steps");
        if (ind.name == "high_error_rate") recs.push_back("Review and reduce task complexity");
    }
    return recs;
}

} // namespace rrt