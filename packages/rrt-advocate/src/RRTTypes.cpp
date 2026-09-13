#include "rrt/RRTTypes.h"

namespace rrt {

const char* crisisLevelToString(CrisisLevel level) {
    switch (level) {
        case CrisisLevel::GREEN: return "stable";
        case CrisisLevel::YELLOW: return "elevated";
        case CrisisLevel::ORANGE: return "high";
        case CrisisLevel::RED: return "critical";
        case CrisisLevel::BLACK: return "emergency";
    }
    return "unknown";
}

const char* semanticFieldToString(KeywordSemanticField field) {
    switch (field) {
        case KeywordSemanticField::NEGATIVE_SELF_TALK: return "negative_self_talk";
        case KeywordSemanticField::TASK_AVOIDANCE: return "task_avoidance";
        case KeywordSemanticField::OVERWHELM: return "overwhelm";
        case KeywordSemanticField::MELTDOWN: return "meltdown";
        case KeywordSemanticField::SHUTDOWN: return "shutdown";
        case KeywordSemanticField::HYPERFOCUS_LOOP: return "hyperfocus_loop";
        case KeywordSemanticField::SELF_HARM_RISK: return "self_harm_risk";
    }
    return "unknown";
}

const char* sentimentTrendToString(SentimentTrend trend) {
    switch (trend) {
        case SentimentTrend::STABLE: return "stable";
        case SentimentTrend::DECLINING: return "declining";
        case SentimentTrend::SHARPLY_DECLINING: return "sharply_declining";
        case SentimentTrend::RECOVERING: return "recovering";
    }
    return "unknown";
}

const char* complexityTrendToString(ComplexityTrend trend) {
    switch (trend) {
        case ComplexityTrend::NORMAL: return "normal";
        case ComplexityTrend::SIMPLIFYING: return "simplifying";
        case ComplexityTrend::FRAGMENTING: return "fragmenting";
    }
    return "unknown";
}

const char* burnoutLevelToString(BurnoutLevel level) {
    switch (level) {
        case BurnoutLevel::NONE: return "none";
        case BurnoutLevel::MILD: return "mild";
        case BurnoutLevel::MODERATE: return "moderate";
        case BurnoutLevel::SEVERE: return "severe";
    }
    return "unknown";
}

void CrisisIndicators::computeAggregate() {
    if (selfHarmRisk) {
        aggregateConfidence = 1.0;
        return;
    }
    aggregateConfidence = std::min(1.0,
        layer1Confidence * LAYER1_WEIGHT +
        layer2Confidence * LAYER2_WEIGHT +
        layer3Confidence * LAYER3_WEIGHT);
}

std::vector<std::string> CrisisIndicators::getPrimaryIndicators() const {
    std::vector<std::string> indicators;
    if (selfHarmRisk) indicators.push_back("SELF_HARM_RISK");
    for (const auto& f : detectedSemanticFields) indicators.push_back(f);
    if (sentimentTrend == "declining" || sentimentTrend == "sharply_declining")
        indicators.push_back("sentiment_trend:" + sentimentTrend);
    if (loopingDetected) indicators.push_back("behavioral_looping");
    if (behavioralComplexity < 0.15) indicators.push_back("behavioral_shutdown_signal");
    return indicators;
}

} // namespace rrt