#include "rrt/BehavioralLayer.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <iterator>
#include <numeric>
#include <set>
#include <string>
#include <vector>

namespace rrt {

static size_t hashToken(const std::string& word) {
    return std::hash<std::string>{}(word);
}

static std::vector<std::string> splitWords(const std::string& text) {
    std::vector<std::string> result;
    size_t pos = 0;
    while (pos < text.size()) {
        size_t end = text.find(' ', pos);
        if (end == std::string::npos) end = text.size();
        if (end > pos) result.push_back(text.substr(pos, end - pos));
        pos = end + 1;
    }
    return result;
}

BehavioralLayer::BehavioralLayer(size_t windowSize) : windowSize_(windowSize) {}

MessageRecord BehavioralLayer::recordMessage(const std::string& text) {
    auto now = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
    int charCount = static_cast<int>(text.size());
    auto words = splitWords(text);
    int wordCount = static_cast<int>(words.size());
    std::vector<size_t> wordHashes;
    for (auto& w : words) {
        std::transform(w.begin(), w.end(), w.begin(), ::tolower);
        w.erase(std::remove_if(w.begin(), w.end(), [](char c) { return c == '.' || c == ',' || c == '!' || c == '?' || c == ';' || c == ':'; }), w.end());
        if (w.size() > 2) wordHashes.push_back(hashToken(w));
    }
    int sentenceCount = 0;
    for (char c : text) if (c == '.' || c == '!' || c == '?') ++sentenceCount;
    sentenceCount = std::max(1, sentenceCount);
    int punctCount = 0;
    for (char c : text) if (std::string(".,!?;:()[]{}'\"").find(c) != std::string::npos) ++punctCount;
    double punctDensity = charCount > 0 ? static_cast<double>(punctCount) / charCount : 0.0;
    return {now, wordCount, charCount, sentenceCount, punctDensity, wordHashes};
}

double BehavioralLayer::computeComplexity(const MessageRecord& record) {
    double wordScore = std::min(record.wordCount / 30.0, 1.0);
    double avgWordsPerSentence = record.sentenceCount > 0 ? static_cast<double>(record.wordCount) / record.sentenceCount : 0.0;
    double sentenceScore = std::min(avgWordsPerSentence / 15.0, 1.0);
    return std::round((wordScore * 0.6 + sentenceScore * 0.4) * 1000.0) / 1000.0;
}

ComplexityTrend BehavioralLayer::computeComplexityTrend() {
    if (records_.size() < 3) return ComplexityTrend::NORMAL;
    std::vector<double> complexities;
    for (const auto& r : records_) complexities.push_back(computeComplexity(r));
    size_t n = complexities.size();
    double recent[3] = {complexities[n-3], complexities[n-2], complexities[n-1]};
    double delta = recent[2] - recent[0];
    if (delta < -0.3) return ComplexityTrend::FRAGMENTING;
    if (delta < -0.15) return ComplexityTrend::SIMPLIFYING;
    return ComplexityTrend::NORMAL;
}

double BehavioralLayer::computeLoopingSimilarity(const MessageRecord& current) {
    if (records_.size() < 2) return 0.0;
    double maxSim = 0.0;
    size_t checkCount = std::min<size_t>(3, records_.size() - 1);
    for (size_t i = records_.size() - checkCount; i < records_.size(); ++i) {
        const auto& prev = records_[i];
        if (current.wordHashes.empty() || prev.wordHashes.empty()) continue;
        std::set<size_t> currSet(current.wordHashes.begin(), current.wordHashes.end());
        std::set<size_t> prevSet(prev.wordHashes.begin(), prev.wordHashes.end());
        std::vector<size_t> intersection, unionSet;
        std::set_intersection(currSet.begin(), currSet.end(), prevSet.begin(), prevSet.end(), std::back_inserter(intersection));
        std::set_union(currSet.begin(), currSet.end(), prevSet.begin(), prevSet.end(), std::back_inserter(unionSet));
        if (!unionSet.empty()) maxSim = std::max(maxSim, static_cast<double>(intersection.size()) / unionSet.size());
    }
    return std::round(maxSim * 1000.0) / 1000.0;
}

double BehavioralLayer::computeConfidence(bool latencyAnomaly, double complexity, ComplexityTrend trend, bool looping) {
    double confidence = 0.0;
    if (latencyAnomaly) confidence += 0.10;
    if (complexity < 0.10) confidence += 0.20;
    else if (complexity < 0.20) confidence += 0.10;
    if (trend == ComplexityTrend::FRAGMENTING) confidence += 0.15;
    else if (trend == ComplexityTrend::SIMPLIFYING) confidence += 0.05;
    if (looping) confidence += 0.20;
    return std::min(1.0, confidence);
}

BehavioralAnalysisResult BehavioralLayer::analyze(const std::string& text) {
    if (text.empty()) return {};
    auto record = recordMessage(text);
    double latency = 0.0;
    bool latencyAnomaly = false;
    if (hasLastTime_) {
        latency = record.timestamp - lastMessageTime_;
        latencyAnomaly = latency > LATENCY_ANOMALY_THRESHOLD;
    }
    double complexity = computeComplexity(record);
    auto trend = computeComplexityTrend();
    double loopingSim = computeLoopingSimilarity(record);
    bool looping = loopingSim >= LOOPING_SIMILARITY_THRESHOLD;
    double confidence = computeConfidence(latencyAnomaly, complexity, trend, looping);
    lastMessageTime_ = record.timestamp;
    hasLastTime_ = true;
    records_.push_back(record);
    if (records_.size() > windowSize_) records_.pop_front();
    return {latency, latencyAnomaly, complexity, trend, looping, loopingSim, confidence};
}

void BehavioralLayer::reset() { records_.clear(); lastMessageTime_ = 0.0; hasLastTime_ = false; }

} // namespace rrt