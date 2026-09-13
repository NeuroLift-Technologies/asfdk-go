#include "rrt/KeywordLayer.h"
#include <algorithm>

namespace rrt {

KeywordLayer::KeywordLayer() { compilePatterns(); }

void KeywordLayer::compilePatterns() {
    fieldPatterns_[KeywordSemanticField::NEGATIVE_SELF_TALK] = {
        R"(i hate myself)",
        R"((i('m| am) (so |really )?(worthless|useless|stupid|pathetic|awful|terrible|broken)))",
        R"(i('m| am) a failure)",
        R"(i always (mess|screw|f+uck) (up|everything))",
        R"(i('m| am) the worst)",
        R"(i ruin everything)",
        R"((nobody|no one) cares (about me|anymore)?)",
        R"(i (can't|cannot) do anything right)",
        R"(why am i (so |like this|like that))",
        R"(i don't deserve)",
        R"(i('m| am) so (dumb|stupid|bad))",
        R"(everything('s| is) my fault)",
        R"(i('m| am) not good enough)",
        R"(i('ve| have) ruined (it|everything|this))",
    };
    fieldWeights_[KeywordSemanticField::NEGATIVE_SELF_TALK] = 0.15;

    fieldPatterns_[KeywordSemanticField::TASK_AVOIDANCE] = {
        R"((can'?t|cannot) start)",
        R"((can'?t|cannot) (do|finish|begin|complete|get started))",
        R"(don'?t know (how to |where to )?begin)",
        R"((can'?t|cannot) make myself)",
        R"((i'?ve? been|been) (procrastin|putting it off|avoiding))",
        R"((too hard|too difficult|too much))",
        R"((giving up|give up|gave up))",
        R"(what'?s the point)",
        R"(can'?t (be|get) (motivated|started|going))",
        R"((paralyzed|paralysed))",
        R"((stuck|frozen) (on|with|at))",
    };
    fieldWeights_[KeywordSemanticField::TASK_AVOIDANCE] = 0.10;

    fieldPatterns_[KeywordSemanticField::OVERWHELM] = {
        R"(too much (going on|happening|at once))",
        R"((can'?t|cannot) cope)",
        R"((can'?t|cannot) handle (this|it|everything|anymore))",
        R"((drowning|overwhelm(ed|ing)))",
        R"(everything (is |feels )?(too much|overwhelming|impossible))",
        R"((spinning|spiraling|spiral(l)?ing))",
        R"((can'?t|cannot) breathe)",
        R"((shutting down|shutdown|shut(ting)? down))",
        R"(brain (is |feels )?(fried|overloaded|full|done))",
        R"((so many|too many) (things|tasks|thoughts))",
        R"((can'?t|cannot) think (straight|clearly))",
    };
    fieldWeights_[KeywordSemanticField::OVERWHELM] = 0.15;

    fieldPatterns_[KeywordSemanticField::MELTDOWN] = {
        R"((melt(ing|ed|down)|meltdown))",
        R"((fall(ing|en) apart|falling to pieces))",
        R"((break(ing|down)|breakdown))",
        R"((can'?t|cannot) stop (crying|shaking|panicking))",
        R"((losing|lost) (it|control|my mind))",
        R"((freaking|flipping|losing) out)",
        R"(everything'?s (falling|crumbling|coming) apart)",
        R"((complete|total|full) (meltdown|breakdown|collapse))",
    };
    fieldWeights_[KeywordSemanticField::MELTDOWN] = 0.25;

    fieldPatterns_[KeywordSemanticField::SHUTDOWN] = {
        R"((shut(ting)? down|shutdown))",
        R"((completely |totally )?(blank|empty|numb|gone|void))",
        R"((can'?t|cannot) (feel|think|move|respond))",
        R"((dissociat(ing|ed)|dissociation))",
        R"((frozen|froze(n)?|freezing))",
        R"((checked out|checked off|not (here|present|real)))",
        R"((going |feel(ing)? )(numb|blank|empty))",
        R"((words|thoughts) (won'?t|don'?t) come)",
        R"((can'?t|cannot) find (words|the words))",
        R"((just|totally|completely) (done|gone|empty|blank))",
    };
    fieldWeights_[KeywordSemanticField::SHUTDOWN] = 0.20;

    fieldPatterns_[KeywordSemanticField::HYPERFOCUS_LOOP] = {
        R"((can'?t|cannot) stop (thinking|going back|focusing))",
        R"((stuck|looping) (in|on) (a loop|it|this))",
        R"((keep|keep on|keeps) (thinking|going back|looping|fixating))",
        R"((hyperfocus(ing|ed)?|hyperfixat(ing|ion|ed)))",
        R"(rabbit hole|down a rabbit)",
        R"((obsess(ing|ed|ion)|obsessive))",
        R"((can'?t|cannot) (let it go|move on|stop|switch off))",
        R"((loop(ing|ed)?|spiral(l)?ing) (back|on|about))",
        R"((intrusive|unwanted) (thought|loop|fixation))",
    };
    fieldWeights_[KeywordSemanticField::HYPERFOCUS_LOOP] = 0.10;

    fieldPatterns_[KeywordSemanticField::SELF_HARM_RISK] = {
        R"((want to |going to |going to )(hurt|harm) (myself|me))",
        R"((self[- ]?harm|self[- ]?hurt|self[- ]?injur))",
        R"(don'?t want to (be here|live|exist|continue))",
        R"((better off|world (is|would be) better) (without me|if i (was|were) gone))",
        R"((thinking about|thought about) (ending|stopping) (it|everything|my life))",
        R"((suicid(e|al|ity)|want to die))",
        R"((kill|end) myself)",
    };
    fieldWeights_[KeywordSemanticField::SELF_HARM_RISK] = 1.00;

    for (const auto& [field, patterns] : fieldPatterns_) {
        std::vector<std::regex> compiled;
        for (const auto& pattern : patterns)
            compiled.emplace_back(pattern, std::regex_constants::icase);
        compiledPatterns_[field] = std::move(compiled);
    }
}

KeywordAnalysisResult KeywordLayer::analyze(const std::string& text) const {
    if (text.empty()) return KeywordAnalysisResult{};
    std::vector<KeywordSemanticField> detectedFields;
    std::vector<KeywordMatch> allMatches;
    std::unordered_map<KeywordSemanticField, int> fieldMatchCounts;
    for (const auto& [field, patterns] : compiledPatterns_) {
        int matchCount = 0;
        const auto& sourcePatterns = fieldPatterns_.at(field);
        for (size_t i = 0; i < patterns.size(); ++i) {
            std::smatch match;
            if (std::regex_search(text, match, patterns[i])) {
                allMatches.push_back({field, sourcePatterns[i], match[0].str(),
                    static_cast<std::size_t>(static_cast<std::streamoff>(match.position()))});
                ++matchCount;
            }
        }
        if (matchCount > 0) {
            detectedFields.push_back(field);
            fieldMatchCounts[field] = matchCount;
        }
    }
    double confidence = computeConfidence(detectedFields, fieldMatchCounts);
    bool selfHarm = std::find(detectedFields.begin(), detectedFields.end(),
        KeywordSemanticField::SELF_HARM_RISK) != detectedFields.end();
    std::optional<KeywordSemanticField> primary;
    if (!detectedFields.empty()) {
        primary = *std::max_element(detectedFields.begin(), detectedFields.end(),
            [this, &fieldMatchCounts](KeywordSemanticField a, KeywordSemanticField b) {
                return fieldWeights_.at(a) * fieldMatchCounts[a] < fieldWeights_.at(b) * fieldMatchCounts[b];
            });
    }
    return {detectedFields, allMatches, confidence, selfHarm, primary};
}

double KeywordLayer::computeConfidence(
    const std::vector<KeywordSemanticField>& detectedFields,
    const std::unordered_map<KeywordSemanticField, int>& fieldMatchCounts) const {
    if (std::find(detectedFields.begin(), detectedFields.end(), KeywordSemanticField::SELF_HARM_RISK) != detectedFields.end())
        return 1.0;
    if (detectedFields.empty()) return 0.0;
    double total = 0.0;
    for (const auto& f : detectedFields) {
        double weight = fieldWeights_.at(f);
        double countBonus = std::min(0.05 * (fieldMatchCounts.at(f) - 1), 0.10);
        total += weight + countBonus;
    }
    return std::min(1.0, total);
}

} // namespace rrt