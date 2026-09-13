#pragma once

#include "rrt/RRTTypes.h"
#include <deque>

namespace rrt {

class SentimentLayer {
public:
    explicit SentimentLayer(size_t windowSize = 5);
    SentimentAnalysisResult analyze(const std::string& text);
    void resetWindow();

private:
    SentimentReading scoreText(const std::string& text);
    SentimentReading fallbackScore(const std::string& text);
    SentimentTrend classifyTrend(double current, double windowAvg, double polarityDrop);
    double computeConfidence(const SentimentReading& reading, double polarityDrop, SentimentTrend trend);

    size_t windowSize_;
    std::deque<double> window_;
    static constexpr double DECLINE_THRESHOLD = -0.15;
    static constexpr double SHARP_DECLINE_THRESHOLD = -0.30;
};

} // namespace rrt