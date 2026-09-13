#pragma once

#include "rrt/RRTTypes.h"
#include "rrt/KeywordLayer.h"
#include "rrt/SentimentLayer.h"
#include "rrt/BehavioralLayer.h"

namespace rrt {

class CrisisDetector {
public:
    CrisisDetector();
    CrisisIndicators detectCrisisIndicators(const std::string& message);
    void resetSession();

private:
    KeywordLayer keywordLayer_;
    SentimentLayer sentimentLayer_;
    BehavioralLayer behavioralLayer_;
};

} // namespace rrt