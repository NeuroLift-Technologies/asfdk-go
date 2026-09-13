#pragma once

#include "rrt/RRTTypes.h"
#include <deque>
#include <string>
#include <vector>

namespace rrt {

struct MessageRecord {
    double timestamp;
    int wordCount;
    int charCount;
    int sentenceCount;
    double punctuationDensity;
    std::vector<size_t> wordHashes;
};

class BehavioralLayer {
public:
    explicit BehavioralLayer(size_t windowSize = 5);
    BehavioralAnalysisResult analyze(const std::string& text);
    void reset();

private:
    MessageRecord recordMessage(const std::string& text);
    double computeComplexity(const MessageRecord& record);
    ComplexityTrend computeComplexityTrend();
    double computeLoopingSimilarity(const MessageRecord& current);
    double computeConfidence(bool latencyAnomaly, double complexity, ComplexityTrend trend, bool looping);

    size_t windowSize_;
    std::deque<MessageRecord> records_;
    double lastMessageTime_ = 0.0;
    bool hasLastTime_ = false;

    static constexpr double LATENCY_ANOMALY_THRESHOLD = 300.0;
    static constexpr double LOOPING_SIMILARITY_THRESHOLD = 0.55;
};

} // namespace rrt