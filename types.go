// Package asfdk is a Go port of the ASFDK (Agent Solidarity Framework Dev
// Kit): governance-aware AI safety primitives — prompt defense, TOI/OTOI
// validation, emotional-state detection (Sleepwalker), and crisis assessment
// (RRT Advocate) — orchestrated by [NeuroLiftFoundation].
//
// Behavior mirrors the canonical TypeScript/Python source and the C# port
// (NeuroLift-Technologies/asfdk-csharp), including canonical enum JSON names
// and FoundationComponents override semantics: an explicit per-component
// override always wins over the mode default.
package asfdk

import "strings"

// FoundationMode selects the default active component set of a foundation.
type FoundationMode string

// Foundation mode values (canonical JSON names).
const (
	ModeUnified        FoundationMode = "unified"
	ModeCrisisOnly     FoundationMode = "crisis_only"
	ModeContinuityOnly FoundationMode = "continuity"
	ModeFrameworkOnly  FoundationMode = "framework"
	ModeDevelopment    FoundationMode = "development"
)

// ParseMode maps a canonical mode name to a FoundationMode. Unknown or empty
// input resolves to ModeUnified.
func ParseMode(s string) FoundationMode {
	switch FoundationMode(s) {
	case ModeUnified, ModeCrisisOnly, ModeContinuityOnly, ModeFrameworkOnly, ModeDevelopment:
		return FoundationMode(s)
	}
	return ModeUnified
}

// InteractionType classifies a governed interaction.
type InteractionType string

// Interaction type values (canonical JSON names).
const (
	InteractionEmotionalAssessment InteractionType = "emotional_assessment"
	InteractionCrisisAlert         InteractionType = "crisis_alert"
	InteractionPreferenceUpdate    InteractionType = "preference_update"
	InteractionOptimizationRequest InteractionType = "optimization_request"
	InteractionStatusInquiry       InteractionType = "status_inquiry"
	InteractionEmergencyEscalation InteractionType = "emergency_escalation"
)

// Channel is the provenance channel of an input or output. ChannelUnknown is
// the zero value.
type Channel string

// Channel values (canonical JSON names).
const (
	ChannelUnknown     Channel = ""
	ChannelUserInput   Channel = "user_input"
	ChannelModelOutput Channel = "model_output"
	ChannelToolResult  Channel = "tool_result"
	ChannelSystem      Channel = "system"
)

// ParseChannel maps a canonical channel name to a Channel. Unknown or empty
// input resolves to ChannelUnknown.
func ParseChannel(s string) Channel {
	switch Channel(s) {
	case ChannelUserInput, ChannelModelOutput, ChannelToolResult, ChannelSystem:
		return Channel(s)
	}
	return ChannelUnknown
}

// ChannelNormalize coerces an arbitrary value into a Channel: strings are
// parsed by canonical name, Channel values pass through, and anything else
// (including nil) resolves to ChannelUnknown.
func ChannelNormalize(value any) Channel {
	switch v := value.(type) {
	case Channel:
		return v
	case string:
		return ParseChannel(v)
	default:
		return ChannelUnknown
	}
}

// RiskLevel grades sanitization findings.
type RiskLevel string

// Risk level values.
const (
	RiskLevelLow    RiskLevel = "low"
	RiskLevelMedium RiskLevel = "medium"
	RiskLevelHigh   RiskLevel = "high"
)

// SecurityEventType classifies security audit events.
type SecurityEventType string

// Security event type values.
const (
	SecurityInjectionAttempt  SecurityEventType = "injection_attempt"
	SecurityValidationFailure SecurityEventType = "validation_failure"
	SecurityLengthExceeded    SecurityEventType = "length_exceeded"
)

// OutputSchemaType selects output validation behavior.
type OutputSchemaType string

// Output schema type values.
const (
	OutputSchemaJSON OutputSchemaType = "json"
	OutputSchemaText OutputSchemaType = "text"
)

// CrisisLevel is an ordered crisis severity; comparisons such as
// level >= CrisisRed are meaningful. CrisisGreen is the zero value.
type CrisisLevel int

// Crisis levels, ordered from least to most severe.
const (
	CrisisGreen CrisisLevel = iota
	CrisisYellow
	CrisisOrange
	CrisisRed
	CrisisBlack
)

var crisisNames = map[CrisisLevel]string{
	CrisisGreen:  "green",
	CrisisYellow: "yellow",
	CrisisOrange: "orange",
	CrisisRed:    "red",
	CrisisBlack:  "black",
}

// String returns the canonical lowercase JSON name of the level.
func (c CrisisLevel) String() string { return crisisNames[c] }

// MarshalJSON encodes the level as its canonical string name.
func (c CrisisLevel) MarshalJSON() ([]byte, error) {
	return []byte(`"` + c.String() + `"`), nil
}

// ParseCrisisLevel maps a canonical name back to a CrisisLevel. Unknown input
// resolves to CrisisGreen.
func ParseCrisisLevel(s string) CrisisLevel {
	for level, name := range crisisNames {
		if name == strings.ToLower(s) {
			return level
		}
	}
	return CrisisGreen
}
