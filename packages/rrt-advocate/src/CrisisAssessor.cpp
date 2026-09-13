#include "rrt/CrisisAssessor.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

namespace rrt {

const std::vector<CrisisAssessor::LevelThreshold> CrisisAssessor::LEVEL_THRESHOLDS = {
    {0.0, 0.20, CrisisLevel::GREEN},
    {0.20, 0.40, CrisisLevel::YELLOW},
    {0.40, 0.70, CrisisLevel::ORANGE},
    {0.70, 0.90, CrisisLevel::RED},
    {0.90, 1.01, CrisisLevel::BLACK},
};

const std::unordered_map<CrisisLevel, std::vector<std::string>> CrisisAssessor::DEFAULT_INTERVENTIONS = {
    {CrisisLevel::GREEN, {}},
    {CrisisLevel::YELLOW, {"breathing_exercise", "grounding_technique"}},
    {CrisisLevel::ORANGE, {"guided_meditation", "cognitive_restructuring"}},
    {CrisisLevel::RED, {"intensive_grounding", "crisis_counseling"}},
    {CrisisLevel::BLACK, {"emergency_stabilization", "crisis_hotline"}},
};

const std::unordered_map<CrisisLevel, double> CrisisAssessor::ESCALATION_THRESHOLDS = {
    {CrisisLevel::GREEN, 0.4},
    {CrisisLevel::YELLOW, 0.6},
    {CrisisLevel::ORANGE, 0.75},
    {CrisisLevel::RED, 0.90},
    {CrisisLevel::BLACK, 1.0},
};

CrisisAssessor::CrisisAssessor(const std::string& userId) : userId_(userId) {}

CrisisAssessment CrisisAssessor::assessCrisis(const CrisisIndicators& indicators) const {
    double confidence = indicators.aggregateConfidence;
    CrisisLevel level = mapConfidenceToLevel(confidence);
    if (indicators.selfHarmRisk) level = CrisisLevel::BLACK;
    double safetyScore = computeSafetyScore(indicators, level);
    auto interventions = getRecommendedInterventions(level);

    CrisisAssessment assessment;
    assessment.timestamp = indicators.timestamp;
    assessment.crisisLevel = level;
    assessment.primaryIndicators = indicators.getPrimaryIndicators();
    assessment.secondaryIndicators = indicators.detectedSemanticFields;
    assessment.confidenceScore = std::round(confidence * 1000.0) / 1000.0;
    assessment.estimatedDuration = std::nullopt;
    assessment.recommendedInterventions = interventions;
    assessment.escalationThreshold = getEscalationThreshold(level);
    assessment.userSafetyScore = std::round(safetyScore * 1000.0) / 1000.0;
    assessment.contextFactors = {
        {"self_harm_risk", indicators.selfHarmRisk},
        {"sentiment_trend", indicators.sentimentTrend},
        {"looping_detected", indicators.loopingDetected},
        {"behavioral_complexity", indicators.behavioralComplexity},
        {"layer_scores", {
            {"keyword", indicators.layer1Confidence},
            {"sentiment", indicators.layer2Confidence},
            {"behavioral", indicators.layer3Confidence},
        }},
    };
    return assessment;
}

CrisisLevel CrisisAssessor::mapConfidenceToLevel(double confidence) const {
    for (const auto& t : LEVEL_THRESHOLDS)
        if (confidence >= t.low && confidence < t.high) return t.level;
    return CrisisLevel::BLACK;
}

double CrisisAssessor::computeSafetyScore(const CrisisIndicators& indicators, CrisisLevel level) const {
    if (indicators.selfHarmRisk) return 0.05;
    double base = 1.0 - indicators.aggregateConfidence;
    if (indicators.loopingDetected) base -= 0.10;
    if (indicators.behavioralComplexity < 0.10) base -= 0.15;
    if (indicators.sentimentTrend == "sharply_declining") base -= 0.10;
    return std::max(0.05, std::min(1.0, base));
}

std::vector<std::string> CrisisAssessor::getRecommendedInterventions(CrisisLevel level) const {
    auto it = DEFAULT_INTERVENTIONS.find(level);
    if (it != DEFAULT_INTERVENTIONS.end()) return it->second;
    return {};
}

double CrisisAssessor::getEscalationThreshold(CrisisLevel level) const {
    auto it = ESCALATION_THRESHOLDS.find(level);
    if (it != ESCALATION_THRESHOLDS.end()) return it->second;
    return 0.8;
}

} // namespace rrt