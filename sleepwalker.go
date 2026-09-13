package asfdk

import (
	"regexp"
	"sort"
	"strings"
)

// reWord matches word characters for confidence dilution.
var reWord = regexp.MustCompile(`\w`)

// emotionalIndicator describes one emotional signal category.
type emotionalIndicator struct {
	Name        string
	Description string
	Keywords    []string
	Patterns    []*regexp.Regexp
}

// emotionalIndicators is the Sleepwalker lexicon, mirroring the TS source.
var emotionalIndicators = []emotionalIndicator{
	{"distress", "User is experiencing acute distress", []string{"can't take", "can’t take", "falling apart", "breaking down", "losing it", "can't cope", "can’t cope", "overwhelmed", "at my limit", "breaking point", "drowning", "suffocating", "trapped", "no way out", "helpless", "hopeless", "desperate"}, nil},
	{"anxiety", "User is experiencing elevated anxiety", []string{"anxious", "worried", "panic", "panicking", "nervous", "stressed out", "on edge", "freaking out", "scared", "afraid", "terrified", "dread", "uneasy", "restless"}, nil},
	{"sadness", "User is experiencing sadness or low mood", []string{"sad", "depressed", "empty", "numb", "cry", "crying", "tears", "miserable", "heartbroken", "grief", "loss", "lonely", "alone", "hopeless", "worthless"}, nil},
	{"anger", "User is experiencing anger or frustration", []string{"angry", "furious", "rage", "frustrated", "fed up", "sick of", "hate", "unfair", "betrayed", "disrespected", "annoyed", "irritated", "livid", "mad"}, nil},
	{"crisis_speak", "User language suggests crisis state", []string{"end it all", "not worth living", "better off dead", "hurt myself", "kill myself", "suicidal", "want to die", "no reason to live", "goodbye forever", "final message", "self harm", "overdose"}, nil},
	{"confusion", "User is confused or disoriented", []string{"confused", "don't understand", "don’t understand", "lost", "unclear", "make sense", "what does this mean", "explain again", "bewildered", "mixed up", "disoriented"}, nil},
	{"excitement", "User is excited or energized", []string{"amazing", "incredible", "can't wait", "can’t wait", "so excited", "thrilled", "ecstatic", "pumped", "stoked", "fantastic", "wonderful", "love this", "best ever"}, nil},
	{"gratitude", "User is expressing gratitude", []string{"thank you", "thanks", "grateful", "appreciate", "so kind", "means a lot", "helped me", "bless you", "gratitude"}, nil},
}

// analyzeIndicators matches text against the indicator lexicon, returning
// matched indicator names and raw scores.
func analyzeIndicators(text string) ([]string, map[string]float64) {
	lower := strings.ToLower(text)
	var matched []string
	scores := map[string]float64{}
	for _, ind := range emotionalIndicators {
		hits := 0
		for _, kw := range ind.Keywords {
			if strings.Contains(lower, kw) {
				hits++
			}
		}
		for _, re := range ind.Patterns {
			if re.MatchString(lower) {
				hits++
			}
		}
		if hits > 0 {
			matched = append(matched, ind.Name)
			scores[ind.Name] = 1.0 - (1.0 / (1.0 + float64(hits)))
		}
	}
	sort.Strings(matched)
	return matched, scores
}

// AnalyzeEmotionalState runs the full emotional analysis pipeline.
func AnalyzeEmotionalState(text string) EmotionalState {
	matched, scores := analyzeIndicators(text)
	state := "neutral"
	confidence := 0.5

	if crisisIdx := indexOf(matched, "crisis_speak"); crisisIdx >= 0 {
		state = "crisis"
		confidence = scores["crisis_speak"]
	} else if len(matched) > 0 {
		state = matched[0]
		confidence = scores[matched[0]]
	}

	if raw := reWord.FindAllString(text, -1); raw != nil {
		if n := float64(len(raw)); n < 5 {
			confidence *= 0.6
		} else if n < 15 {
			confidence *= 0.85
		}
	}

	return EmotionalState{State: state, Confidence: clamp01(confidence), Indicators: matched, RawScores: scores}
}

// AnalyzeEmotionalStateWithProvenance adds channel-aware trust analysis.
func AnalyzeEmotionalStateWithProvenance(text string, channel Channel) EmotionalStateWithProvenance {
	state := AnalyzeEmotionalState(text)
	trusted := channelTrusted(channel)
	res := EmotionalStateWithProvenance{EmotionalState: state, Channel: channel, Trusted: trusted}
	if !trusted {
		res.Flagged = true
		res.FlagReason = "untrusted channel: " + string(channel)
	}
	return res
}

// clamp01 constrains v to [0, 1].
func clamp01(v float64) float64 {
	if v < 0 {
		return 0
	}
	if v > 1 {
		return 1
	}
	return v
}

// indexOf returns the first index of s in list, or -1.
func indexOf(list []string, s string) int {
	for i, v := range list {
		if v == s {
			return i
		}
	}
	return -1
}

// SleepwalkerProtocol performs emotional-state analysis (Sleepwalker).
type SleepwalkerProtocol struct{}

// NewSleepwalkerProtocol creates a SleepwalkerProtocol.
func NewSleepwalkerProtocol() *SleepwalkerProtocol { return &SleepwalkerProtocol{} }

// Analyze analyzes text for emotional state.
func (s *SleepwalkerProtocol) Analyze(text string) EmotionalState {
	return AnalyzeEmotionalState(text)
}

// AnalyzeWithProvenance analyzes text with channel trust awareness.
func (s *SleepwalkerProtocol) AnalyzeWithProvenance(text string, channel Channel) EmotionalStateWithProvenance {
	return AnalyzeEmotionalStateWithProvenance(text, channel)
}

// channelTrusted reports whether a channel is trusted for emotional input.
// Only direct user input and system events are trusted.
func channelTrusted(channel Channel) bool {
	return channel == ChannelUserInput || channel == ChannelSystem
}
