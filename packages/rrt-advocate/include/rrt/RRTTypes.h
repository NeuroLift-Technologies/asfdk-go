#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <optional>
#include <nlohmann/json.hpp>

namespace rrt {

// CrisisLevel — crisis severity levels (mirrors Python CrisisLevel enum)
enum class CrisisLevel { GREEN, YELLOW, ORANGE, RED, BLACK };
const char* crisisLevelToString(CrisisLevel level);

// KeywordSemanticField — semantic fields for keyword analysis
enum class KeywordSemanticField {
    NEGATIVE_SELF_TALK, TASK_AVOIDANCE, OVERWHELM,
    MELTDOWN, SHUTDOWN, HYPERFOCUS_LOOP, SELF_HARM_RISK
};
const char* semanticFieldToString(KeywordSemanticField field);

// Layer 1: Keyword Analysis Types
struct KeywordMatch {
    KeywordSemanticField field;
    std::string pattern;
    std::string matchedText;
    std::size_t position;
};

struct KeywordAnalysisResult {
    std::vector<KeywordSemanticField> detectedFields;
    std::vector<KeywordMatch> matches;
    double confidenceScore = 0.0;
    bool selfHarmDetected = false;
    std::optional<KeywordSemanticField> primaryField;
};

// Layer 2: Sentiment Analysis Types
struct SentimentReading {
    double compound = 0.0;
    double positive = 0.0;
    double negative = 0.0;
    double neutral = 0.0;
    std::string textSnippet;
};

enum class SentimentTrend { STABLE, DECLINING, SHARPLY_DECLINING, RECOVERING };
const char* sentimentTrendToString(SentimentTrend trend);

struct SentimentAnalysisResult {
    SentimentReading currentReading;
    double polarityDrop = 0.0;
    double windowAverage = 0.0;
    SentimentTrend trend = SentimentTrend::STABLE;
    double confidenceScore = 0.0;
    std::vector<double> windowReadings;
};

// Layer 3: Behavioral Analysis Types
enum class ComplexityTrend { NORMAL, SIMPLIFYING, FRAGMENTING };
const char* complexityTrendToString(ComplexityTrend trend);

struct BehavioralAnalysisResult {
    std::optional<double> responseLatency;
    bool latencyAnomaly = false;
    double messageComplexity = 0.0;
    ComplexityTrend complexityTrend = ComplexityTrend::NORMAL;
    bool loopingDetected = false;
    double loopingSimilarity = 0.0;
    double confidenceScore = 0.0;
};

// CrisisIndicators — aggregated output from all three CDE layers
struct CrisisIndicators {
    std::chrono::system_clock::time_point timestamp;
    std::string rawText;
    KeywordAnalysisResult keywordResult;
    SentimentAnalysisResult sentimentResult;
    BehavioralAnalysisResult behavioralResult;
    bool selfHarmRisk = false;
    std::vector<std::string> detectedSemanticFields;
    std::string sentimentTrend = "stable";
    bool loopingDetected = false;
    double behavioralComplexity = 1.0;
    double layer1Confidence = 0.0;
    double layer2Confidence = 0.0;
    double layer3Confidence = 0.0;
    double aggregateConfidence = 0.0;
    static constexpr double LAYER1_WEIGHT = 0.45;
    static constexpr double LAYER2_WEIGHT = 0.35;
    static constexpr double LAYER3_WEIGHT = 0.20;
    void computeAggregate();
    std::vector<std::string> getPrimaryIndicators() const;
};

// CrisisAssessment — final crisis assessment output
struct CrisisAssessment {
    std::chrono::system_clock::time_point timestamp;
    CrisisLevel crisisLevel = CrisisLevel::GREEN;
    std::vector<std::string> primaryIndicators;
    std::vector<std::string> secondaryIndicators;
    double confidenceScore = 0.0;
    std::optional<std::chrono::seconds> estimatedDuration;
    std::vector<std::string> recommendedInterventions;
    double escalationThreshold = 0.0;
    double userSafetyScore = 1.0;
    nlohmann::json contextFactors;
};

// Burnout Types
enum class BurnoutLevel { NONE, MILD, MODERATE, SEVERE };
const char* burnoutLevelToString(BurnoutLevel level);

struct BurnoutIndicator {
    std::string name;
    double severity = 0.0;
    std::string description;
};

struct BurnoutAssessment {
    std::chrono::system_clock::time_point timestamp;
    BurnoutLevel level = BurnoutLevel::NONE;
    double overallScore = 0.0;
    std::vector<BurnoutIndicator> indicators;
    std::vector<std::string> recommendations;
    bool escalationRequired = false;
};

// RRT Error type (for dual error handling per DECISIONS.md §2)
struct RRTError {
    enum class Code { InvalidInput, DetectionFailure, AssessmentFailure, ConfigLoadFailure, UnknownError };
    Code code;
    std::string message;
};

} // namespace rrt