/**
 * standalone_test.cpp — Dependency-free test runner for the Sleepwalker package.
 *
 * The CMake tests use Catch2, which is unavailable in this environment; this
 * runner exercises the same core behaviors (state detection, graduated
 * consent, continuity persistence, and protocol orchestration) with plain
 * assertions so the package can be verified locally.
 *
 * Build:
 *   g++ -std=c++20 -I include -I ../include tests/standalone_test.cpp src/*.cpp -o /tmp/sw_test
 *
 * Governed by: ORG-DEV-OTOI-1.0.3
 */

#include "sleepwalker/SleepwalkerProtocol.h"


// Local process-id helper — the ports have no cross-platform PID utility, and
// tests only need a unique suffix for temp directories. POSIX unistd.h and
// Windows process.h provide the platform's own unique-directory API
// (::getpid / ::getpid), which is unique across processes. Other non-POSIX
// targets fall back to a non-throwing seed (random_device, with a
// high-resolution-clock fallback) plus a process-local atomic counter, so the
// runner compiles without relying on any transitive header and no two
// directories ever collide within a process.
#if defined(_WIN32)
#include <process.h>
static std::string localProcessId() { return std::to_string(static_cast<long>(::_getpid())); }
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__)
#include <unistd.h>
static std::string localProcessId() { return std::to_string(static_cast<long>(::getpid())); }
#else
#include <atomic>
#include <chrono>
#include <random>
static std::string localProcessId() {
    static std::atomic<long> counter{0};
    long seed;
    try {
        std::random_device rd;
        seed = static_cast<long>(rd());
    } catch (...) {
        seed = static_cast<long>(
            std::chrono::steady_clock::now().time_since_epoch().count() & 0x7FFFFFFFL);
    }
    return std::to_string(seed + counter.fetch_add(1, std::memory_order_relaxed));
}
#endif
#include "sleepwalker/StateDetector.h"
#include "sleepwalker/ConsentManager.h"
#include "sleepwalker/ContinuityManager.h"
#include "sleepwalker/SleepwalkerTypes.h"

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

using namespace sleepwalker;

namespace {

int g_passed = 0;
int g_failed = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (cond) {                                                          \
            ++g_passed;                                                      \
        } else {                                                             \
            ++g_failed;                                                      \
            std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ << "  "     \
                      << #cond << "\n";                                      \
        }                                                                    \
    } while (false)

// ------------------------- StateDetector -------------------------

void testStateDetectorNeutral() {
    StateDetector d;
    EmotionalState s = d.detect("Can you help me write a parser in C++?");
    CHECK(s.stateType == StateType::Neutral);
    CHECK(!s.protective);
    CHECK(!s.requiresCheckIn);
    CHECK(s.confidence == 0.0);
}

void testStateDetectorDissociation() {
    StateDetector d;
    EmotionalState s = d.detect("I feel numb and totally disconnected today");
    // Dissociation has top priority when multiple states match.
    CHECK(s.stateType == StateType::Dissociation);
    CHECK(s.protective);
    CHECK(s.indicators.dissociation);
    CHECK(s.confidence > 0.0);
}

void testStateDetectorNumbing() {
    StateDetector d;
    EmotionalState s = d.detect("I am just emotionally flat lately");
    CHECK(s.stateType == StateType::Numbing);
    CHECK(s.protective);
    // Single active indicator → 0.5 confidence.
    CHECK(s.confidence == 0.5);
}

void testStateDetectorAvoidance() {
    StateDetector d;
    EmotionalState s = d.detect("maybe later, I am not ready to talk about it");
    CHECK(s.stateType == StateType::Avoidance);
    CHECK(s.protective);
}

void testStateDetectorDetachment() {
    StateDetector d;
    EmotionalState s = d.detect("I'm fine, it's whatever, it doesn't matter");
    CHECK(s.stateType == StateType::Detachment);
    CHECK(s.protective);
}

void testStateDetectorCrisis() {
    StateDetector d;
    EmotionalState sui = d.detect("sometimes I think about how to kill myself");
    CHECK(sui.requiresCheckIn);
    CHECK(sui.explicitSuicidalIdeation);
    CHECK(!sui.selfHarmIndicators);

    EmotionalState harm = d.detect("I have been hurting myself again");
    CHECK(harm.requiresCheckIn);
    CHECK(harm.selfHarmIndicators);

    EmotionalState safe = d.detect("I feel like I am about to lose control");
    CHECK(safe.requiresCheckIn);
    CHECK(safe.inabilityToEnsureSafety);
    CHECK(!safe.protective);
}

// ------------------------- ConsentManager -------------------------

void testConsentManagerLevels() {
    nlohmann::json toi = nlohmann::json::object();
    ConsentManager cm(toi);

    EmotionalState neutral;
    CHECK(cm.determineLevel(neutral) == ConsentLevel::Passive);
    CHECK(!cm.shouldIntervene(neutral));

    EmotionalState crisis;
    crisis.explicitSuicidalIdeation = true;
    CHECK(cm.determineLevel(crisis) == ConsentLevel::RRTAHandoff);
    CHECK(cm.shouldIntervene(crisis));

    EmotionalState checkIn;
    checkIn.requiresCheckIn = true;
    CHECK(cm.determineLevel(checkIn) == ConsentLevel::SafetyCheck);
    CHECK(cm.shouldIntervene(checkIn));

    EmotionalState protective;
    protective.protective = true;
    // Default threshold user_initiated_only → PASSIVE for protective states.
    CHECK(cm.determineLevel(protective) == ConsentLevel::Passive);
    CHECK(!cm.shouldIntervene(protective));

    // With offer_support_without_pressure → LOW_PRESSURE.
    nlohmann::json toi2 = {{"swp", {{"intervention_threshold", "offer_support_without_pressure"}}}};
    ConsentManager cm2(toi2);
    CHECK(cm2.determineLevel(protective) == ConsentLevel::LowPressure);
    CHECK(!cm2.shouldIntervene(protective));
}

void testConsentManagerMessages() {
    nlohmann::json toi = nlohmann::json::object();
    ConsentManager cm(toi);
    CHECK(!cm.getConsentMessage(ConsentLevel::Passive).empty());
    CHECK(!cm.getConsentMessage(ConsentLevel::LowPressure).empty());
    CHECK(!cm.getConsentMessage(ConsentLevel::SafetyCheck).empty());
    CHECK(!cm.getConsentMessage(ConsentLevel::RRTAHandoff).empty());

    EmotionalState crisis;
    crisis.selfHarmIndicators = true;
    CHECK(cm.getAppropriateLevel(crisis) == ConsentLevel::RRTAHandoff);
}

// ------------------------- ContinuityManager -------------------------

void testContinuityManager() {
    namespace fs = std::filesystem;
    fs::path storage = fs::temp_directory_path() /
                       ("swp_test_storage_" + localProcessId());
    std::error_code ec;
    fs::remove_all(storage, ec);

    {
        ContinuityManager cm(storage.string());
        CHECK(fs::exists(storage));

        ContinuityContext fresh = cm.getContext("alice");
        CHECK(!fresh.hasHistory);
        CHECK(fresh.sessionCount == 0);
        CHECK(fresh.lastSessionState == std::nullopt);

        cm.saveSession("alice", {{"emotional_state", "dissociation"},
                                 {"protective_state_active", true}});

        ContinuityContext ctx = cm.getContext("alice");
        CHECK(ctx.hasHistory);
        CHECK(ctx.sessionCount == 1);
        CHECK(ctx.lastSessionState == std::optional<std::string>("dissociation"));
        CHECK(ctx.protectiveStateActive);
        CHECK(ctx.daysSinceLastSession == std::optional<int>(0));

        cm.saveSession("alice", {{"emotional_state", "neutral"}});
        CHECK(cm.getContext("alice").sessionCount == 2);

        cm.updateBoundary("alice", "protected_topics",
                          nlohmann::json::array({"work", "family"}));
        ContinuityContext withBoundary = cm.getContext("alice");
        CHECK(withBoundary.declaredBoundaries.is_object());
        CHECK(withBoundary.declaredBoundaries.contains("protected_topics"));

        nlohmann::json last = cm.retrieveLastSessionState("alice");
        CHECK(last.contains("timestamp"));
        CHECK(last.value("emotional_state", std::string()) == "neutral");
    }

    fs::remove_all(storage, ec);
}

// ------------------------- SleepwalkerProtocol -------------------------

void testProtocolBasics() {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    opts.storagePath = (std::filesystem::temp_directory_path() /
                        ("swp_proto_" + localProcessId())).string();
    SleepwalkerProtocol swp(opts);

    nlohmann::json status = SleepwalkerProtocol::getStatus();
    CHECK(status.value("active", false) == true);
    CHECK(status.value("mode", std::string()) == "emotional-continuity");
    swp.reset(); // must be safe to call

    std::error_code ec;
    std::filesystem::remove_all(opts.storagePath, ec);
}

void testProtocolAssessment() {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    opts.storagePath = (std::filesystem::temp_directory_path() /
                        ("swp_proto2_" + localProcessId())).string();
    SleepwalkerProtocol swp(opts);

    InteractionAssessment a = swp.assessInteraction("please review this pull request");
    CHECK(a.swpActive);
    CHECK(!a.protectiveStateActive);
    CHECK(a.consentLevel == ConsentLevel::Passive);
    CHECK(!a.continuityContext.has_value());

    InteractionAssessment crisis = swp.assessInteraction("I want to end it all");
    CHECK(crisis.consentLevel == ConsentLevel::RRTAHandoff);

    InteractionAssessment withCtx = swp.assessInteraction("hello", {}, "user42");
    CHECK(withCtx.continuityContext.has_value());
    CHECK(!withCtx.continuityContext->hasHistory);

    std::error_code ec;
    std::filesystem::remove_all(opts.storagePath, ec);
}

void testProtocolResponseGuidance() {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    opts.storagePath = (std::filesystem::temp_directory_path() /
                        ("swp_proto3_" + localProcessId())).string();
    SleepwalkerProtocol swp(opts);

    // Protective state → stable low-demand response.
    ResponseGuidance low = swp.generateResponse("I feel numb and spaced out");
    CHECK(low.responseType == "stable_low_demand");
    CHECK(low.intervention == "none");
    CHECK(low.emotionalDemands == std::optional<std::string>("minimal"));

    // Check-in/crisis state → graduated consent offer at RRTA handoff level
    // (determineLevel prioritizes crisis flags over requiresCheckIn).
    ResponseGuidance consent = swp.generateResponse("sometimes I think about suicide");
    CHECK(consent.responseType == "consent_offer");
    CHECK(consent.intervention == "consent_required");
    CHECK(consent.level == std::optional<ConsentLevel>(ConsentLevel::RRTAHandoff));
    EmotionalState crisisState = swp.detectEmotionalState("sometimes I think about suicide");
    CHECK(swp.requiresRrtaHandoff(crisisState));

    // Neutral → plain supportive response.
    ResponseGuidance neutral = swp.generateResponse("what time is the meeting?");
    CHECK(neutral.responseType == "neutral");
    CHECK(neutral.intervention == "none");

    // SWP disabled in TOI → protective input no longer yields low-demand mode.
    SleepwalkerProtocol::Options off = opts;
    off.userToi = {{"swp", {{"active", false}}}};
    SleepwalkerProtocol swpOff(off);
    ResponseGuidance offResp = swpOff.generateResponse("I feel numb and spaced out");
    CHECK(offResp.responseType == "neutral");

    std::error_code ec;
    std::filesystem::remove_all(opts.storagePath, ec);
}

void testProtocolContinuityEndToEnd() {
    SleepwalkerProtocol::Options opts;
    opts.loggingEnabled = false;
    opts.storagePath = (std::filesystem::temp_directory_path() /
                        ("swp_proto4_" + localProcessId())).string();
    SleepwalkerProtocol swp(opts);

    swp.maintainContinuity("bob", {{"emotional_state", "avoidance"},
                                   {"protective_state_active", true}});

    InteractionAssessment a = swp.assessInteraction("hello", {}, "bob");
    CHECK(a.continuityContext.has_value());
    CHECK(a.continuityContext->hasHistory);
    CHECK(a.continuityContext->sessionCount == 1);
    CHECK(a.continuityContext->lastSessionState == std::optional<std::string>("avoidance"));
    CHECK(a.continuityContext->protectiveStateActive);

    std::error_code ec;
    std::filesystem::remove_all(opts.storagePath, ec);
}

void testTypeHelpers() {
    CHECK(stateTypeToString(StateType::Dissociation) == "dissociation");
    CHECK(stateTypeToString(StateType::Neutral) == "neutral");
    CHECK(stringToStateType("avoidance") == StateType::Avoidance);

    CHECK(consentLevelToString(ConsentLevel::RRTAHandoff) == "RRTA_HANDOFF");
    CHECK(consentLevelToString(ConsentLevel::SafetyCheck) == "SAFETY_CHECK");

    CHECK(channelToString(Channel::UserInput) == "user_input");
    CHECK(stringToChannel("model_output") == Channel::ModelOutput);
    CHECK(stringToChannel("garbage") == Channel::Unknown);
}

} // namespace

int main() {
    testStateDetectorNeutral();
    testStateDetectorDissociation();
    testStateDetectorNumbing();
    testStateDetectorAvoidance();
    testStateDetectorDetachment();
    testStateDetectorCrisis();
    testConsentManagerLevels();
    testConsentManagerMessages();
    testContinuityManager();
    testProtocolBasics();
    testProtocolAssessment();
    testProtocolResponseGuidance();
    testProtocolContinuityEndToEnd();
    testTypeHelpers();

    std::cout << "=== Sleepwalker Results: " << g_passed << " passed, "
              << g_failed << " failed ===\n";
    return g_failed == 0 ? 0 : 1;
}
