#include "rrt/SentimentLayer.h"
#include <algorithm>
#include <cctype>
#include <regex>
#include <set>
#include <cmath>

namespace rrt {

SentimentLayer::SentimentLayer(size_t windowSize) : windowSize_(windowSize) {}

SentimentReading SentimentLayer::scoreText(const std::string& text) {
    return fallbackScore(text);
}

SentimentReading SentimentLayer::fallbackScore(const std::string& text) {
    std::string snippet = text.substr(0, std::min<size_t>(text.size(), 60));

    static const std::set<std::string> positiveWords = {
        "good", "great", "okay", "fine", "better", "calm", "happy",
        "relieved", "hopeful", "grateful", "thank", "love", "safe",
    };
    static const std::set<std::string> negativeWords = {
        "bad", "terrible", "awful", "horrible", "hate", "depressed",
        "anxious", "scared", "hopeless", "worthless", "useless",
        "pain", "hurt", "suffering", "stuck", "broken", "lost", "cope",
        "fail", "can't", "cannot", "cant", "never", "worst", "empty",
    };

    std::string lower;
    lower.resize(text.size());
    std::transform(text.begin(), text.end(), lower.begin(), ::tolower);
    std::string cleaned = std::regex_replace(lower, std::regex("[^a-z\\s]"), "");
    auto words = std::set<std::string>{};
    size_t pos = 0;
    while (pos < cleaned.size()) {
        size_t end = cleaned.find(' ', pos);
        if (end == std::string::npos) end = cleaned.size();
        if (end > pos) words.insert(cleaned.substr(pos, end - pos));
        pos = end + 1;
    }

    int posCount = 0, negCount = 0;
    for (const auto& w : words) {
        if (positiveWords.count(w)) ++posCount;
        if (negativeWords.count(w)) ++negCount;
    }
    int total = posCount + negCount;
    double compound = 0.0;
    if (total > 0) compound = static_cast<double>(posCount - negCount) / total;
    compound = std::max(-1.0, std::min(1.0, compound));
    double negRatio = total > 0 ? static_cast<double>(negCount) / total : 0.0;

    return {compound, total > 0 ? static_cast<double>(posCount) / total : 0.0, negRatio,
            1.0 - std::abs(compound), snippet};
}

SentimentTrend SentimentLayer::classifyTrend(double current, double windowAvg, double polarityDrop) {
    if (window_.size() < 2) return current < -0.3 ? SentimentTrend::DECLINING : SentimentTrend::STABLE;
    if (polarityDrop <= SHARP_DECLINE_THRESHOLD) return SentimentTrend::SHARPLY_DECLINING;
    if (polarityDrop <= DECLINE_THRESHOLD) return SentimentTrend::DECLINING;
    if (polarityDrop >= 0.15) return SentimentTrend::RECOVERING;
    return SentimentTrend::STABLE;
}

double SentimentLayer::computeConfidence(const SentimentReading& reading, double polarityDrop, SentimentTrend trend) {
    double confidence = 0.0;
    if (reading.compound < -0.6) confidence += 0.30;
    else if (reading.compound < -0.3) confidence += 0.15;
    else if (reading.compound < 0.0) confidence += 0.10;
    if (trend == SentimentTrend::SHARPLY_DECLINING) confidence += 0.20;
    else if (trend == SentimentTrend::DECLINING) confidence += 0.10;
    return std::min(1.0, confidence);
}

SentimentAnalysisResult SentimentLayer::analyze(const std::string& text) {
    auto reading = scoreText(text);
    std::vector<double> windowValues(window_.begin(), window_.end());
    double windowAvg = windowValues.empty() ? reading.compound :
        std::accumulate(windowValues.begin(), windowValues.end(), 0.0) / windowValues.size();
    double polarityDrop = reading.compound - windowAvg;
    window_.push_back(reading.compound);
    if (window_.size() > windowSize_) window_.pop_front();
    auto trend = classifyTrend(reading.compound, windowAvg, polarityDrop);
    double confidence = computeConfidence(reading, polarityDrop, trend);
    return {reading, polarityDrop, windowAvg, trend, confidence, std::vector<double>(window_.begin(), window_.end())};
}

void SentimentLayer::resetWindow() { window_.clear(); }

} // namespace rrt