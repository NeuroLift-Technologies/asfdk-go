#pragma once

#include "rrt/RRTTypes.h"
#include <regex>
#include <unordered_map>

namespace rrt {

class KeywordLayer {
public:
    KeywordLayer();
    KeywordAnalysisResult analyze(const std::string& text) const;

private:
    void compilePatterns();
    double computeConfidence(
        const std::vector<KeywordSemanticField>& detectedFields,
        const std::unordered_map<KeywordSemanticField, int>& fieldMatchCounts) const;

    std::unordered_map<KeywordSemanticField, std::vector<std::regex>> compiledPatterns_;
    std::unordered_map<KeywordSemanticField, std::vector<std::string>> fieldPatterns_;
    std::unordered_map<KeywordSemanticField, double> fieldWeights_;
};

} // namespace rrt