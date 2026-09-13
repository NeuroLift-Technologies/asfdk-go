#include "rrt/CrisisDetector.h"

namespace rrt {

CrisisDetector::CrisisDetector() = default;

CrisisIndicators CrisisDetector::detectCrisisIndicators(const std::string& message) {
    auto keywordResult = keywordLayer_.analyze(message);
    auto sentimentResult = sentimentLayer_.analyze(message);
    auto behavioralResult = behavioralLayer_.analyze(message);

    CrisisIndicators indicators;
    indicators.timestamp = std::chrono::system_clock::now();
    indicators.rawText = message;
    indicators.keywordResult = keywordResult;
    indicators.sentimentResult = sentimentResult;
    indicators.behavioralResult = behavioralResult;
    indicators.selfHarmRisk = keywordResult.selfHarmDetected;
    indicators.detectedSemanticFields.reserve(keywordResult.detectedFields.size());
    for (const auto& f : keywordResult.detectedFields)
        indicators.detectedSemanticFields.push_back(semanticFieldToString(f));
    indicators.sentimentTrend = sentimentTrendToString(sentimentResult.trend);
    indicators.loopingDetected = behavioralResult.loopingDetected;
    indicators.behavioralComplexity = behavioralResult.messageComplexity;
    indicators.layer1Confidence = keywordResult.confidenceScore;
    indicators.layer2Confidence = sentimentResult.confidenceScore;
    indicators.layer3Confidence = behavioralResult.confidenceScore;
    indicators.computeAggregate();
    return indicators;
}

void CrisisDetector::resetSession() {
    sentimentLayer_.resetWindow();
    behavioralLayer_.reset();
}

} // namespace rrt