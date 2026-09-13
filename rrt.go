package asfdk

import (
	"sort"
	"strings"
	"time"
)

// Crisis thresholds for RRT. Weights are additive per matched indicator
// category, so boundaries sit low: a single explicit self-harm ideation
// phrase (weight 0.4) must reach CrisisBlack.
const (
	// GreenThreshold is the boundary between CrisisGreen and CrisisYellow.
	GreenThreshold = 0.1
	// YellowThreshold is the boundary between CrisisYellow and CrisisOrange.
	YellowThreshold = 0.2
	// OrangeThreshold is the boundary between CrisisOrange and CrisisRed.
	OrangeThreshold = 0.3
	// RedThreshold is the boundary between CrisisRed and CrisisBlack.
	RedThreshold = 0.4
)

// crisisPatterns maps indicator names to lowercase substrings.
var crisisPatterns = map[string][]string{
	"self_harm_ideation":      {"kill myself", "end my life", "suicidal", "want to die", "better off dead", "hurt myself", "harm myself"},
	"hopelessness":            {"no point", "no reason to live", "hopeless", "nothing matters", "never get better"},
	"burden_statements":       {"burden on everyone", "everyone would be better off without me", "people would be happier if i was gone"},
	"goodbye_statements":      {"this is goodbye", "final message", "last time talking", "goodbye forever"},
	"giving_away_possessions": {"giving away my things", "giving away my stuff", "gave away everything"},
	"substance_concern":       {"overdose", "took too many", "drinking to forget", "pills to sleep"},
	"acute_agitation":         {"can't calm down", "can’t calm down", "shaking with anger", "about to explode"},
}

// crisisWeights scores each indicator's contribution to the crisis score.
var crisisWeights = map[string]float64{
	"self_harm_ideation": 0.4, "hopelessness": 0.15, "burden_statements": 0.15,
	"goodbye_statements": 0.12, "giving_away_possessions": 0.08,
	"substance_concern": 0.1, "acute_agitation": 0.08,
}

// crisisPatternNames returns the indicator names in sorted order so crisis
// scoring is deterministic regardless of Go map iteration order.
func crisisPatternNames() []string {
	names := make([]string, 0, len(crisisPatterns))
	for name := range crisisPatterns {
		names = append(names, name)
	}
	sort.Strings(names)
	return names
}

// CrisisAdvisor performs crisis detection and response planning (RRT).
type CrisisAdvisor struct{}

// NewCrisisAdvisor creates a CrisisAdvisor.
func NewCrisisAdvisor() *CrisisAdvisor { return &CrisisAdvisor{} }

// scoreCrisis computes the aggregate crisis score and matched indicators.
func scoreCrisis(text string) (float64, []string, []string) {
	lower := strings.ToLower(text)
	var primary, secondary []string
	score := 0.0
	for _, name := range crisisPatternNames() {
		for _, kw := range crisisPatterns[name] {
			if strings.Contains(lower, kw) {
				score += crisisWeights[name]
				primary = append(primary, name)
				break
			}
		}
	}
	return clamp01(score), primary, secondary
}

// AssessCrisis performs crisis assessment on interaction data.
func AssessCrisis(data map[string]any) CrisisAssessment {
	text := textFromData(data)
	score, primary, _ := scoreCrisis(text)
	level := CrisisGreen
	switch {
	case score >= RedThreshold:
		level = CrisisBlack
	case score >= OrangeThreshold:
		level = CrisisRed
	case score >= YellowThreshold:
		level = CrisisOrange
	case score >= GreenThreshold:
		level = CrisisYellow
	}
	interventions := []string{}
	switch {
	case level >= CrisisRed:
		interventions = append(interventions, "immediate human escalation", "provide crisis hotline resources", "encourage contact with emergency services")
	case level == CrisisOrange:
		interventions = append(interventions, "active listening", "provide crisis hotline resources", "increase monitoring frequency")
	case level == CrisisYellow:
		interventions = append(interventions, "supportive engagement", "offer coping strategies")
	default:
		interventions = append(interventions, "routine monitoring")
	}
	confidence := clamp01(score)
	if len(primary) == 0 {
		confidence = 0
	} else if confidence > 0.95 {
		confidence = 0.95
	}
	return CrisisAssessment{
		Timestamp: time.Now(), CrisisLevel: level, PrimaryIndicators: primary,
		SecondaryIndicators: []string{}, ConfidenceScore: confidence,
		RecommendedInterventions: interventions, EscalationThreshold: OrangeThreshold,
		UserSafetyScore: 1 - confidence, ContextFactors: map[string]any{},
	}
}

// AssessCrisisWithProvenance adds channel-aware trust analysis.
func AssessCrisisWithProvenance(data map[string]any, channel Channel) CrisisAssessmentWithProvenance {
	assessment := AssessCrisis(data)
	trusted := channelTrusted(channel)
	res := CrisisAssessmentWithProvenance{CrisisAssessment: assessment, Channel: channel, Trusted: trusted}
	if !trusted {
		res.Flagged = true
		res.FlagReason = "untrusted channel: " + string(channel)
	}
	return res
}

// GenerateCrisisResponse produces a safe response script for the level.
func (a *CrisisAdvisor) GenerateCrisisResponse(level CrisisLevel, assessment CrisisAssessment) string {
	switch {
	case level >= CrisisRed:
		return "I'm concerned about your safety. Please reach out to emergency services or a crisis hotline right now. You are not alone, and help is available."
	case level == CrisisOrange:
		return "It sounds like you're going through something really difficult. I'm here with you. Would you like help finding crisis support resources?"
	case level == CrisisYellow:
		return "I hear that things feel heavy right now. Let's take this one step at a time — would some coping strategies help?"
	default:
		return "Thanks for sharing how you're feeling. I'm here whenever you want to talk."
	}
}
