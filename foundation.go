// Package asfdk — foundation orchestration. Mirrors
// src/Asfdk/NeuroLiftFoundation.cs from the C# port, including override
// semantics: an explicit FoundationComponents override always wins over the
// mode default.
package asfdk

import (
	"encoding/json"
	"fmt"
	"os"
	"strings"
	"time"
)

func BoolPtr(b bool) *bool { return &b }

// activeComponentNames is the canonical alphabetical order of the framework
// components, used consistently by HealthCheck and StatusSummary.
var activeComponentNames = []string{"rrt_advocate", "sleepwalker_protocol", "toi_otoi_framework"}

// defaultComponents returns the mode default component set.
func defaultComponents(mode FoundationMode) FoundationComponents {
	switch mode {
	case ModeCrisisOnly:
		return FoundationComponents{ToiOtoiFramework: BoolPtr(false), SleepwalkerProtocol: BoolPtr(false), RrtAdvocate: BoolPtr(true)}
	case ModeContinuityOnly:
		return FoundationComponents{ToiOtoiFramework: BoolPtr(false), SleepwalkerProtocol: BoolPtr(true), RrtAdvocate: BoolPtr(false)}
	case ModeFrameworkOnly:
		return FoundationComponents{ToiOtoiFramework: BoolPtr(true), SleepwalkerProtocol: BoolPtr(false), RrtAdvocate: BoolPtr(false)}
	case ModeDevelopment:
		return FoundationComponents{ToiOtoiFramework: BoolPtr(true), SleepwalkerProtocol: BoolPtr(true), RrtAdvocate: BoolPtr(false)}
	default: // ModeUnified
		return FoundationComponents{ToiOtoiFramework: BoolPtr(true), SleepwalkerProtocol: BoolPtr(true), RrtAdvocate: BoolPtr(true)}
	}
}

// pick resolves override ?? fallback, matching the C# port's Pick helper.
func pick(override *bool, fallback bool) bool {
	if override != nil {
		return *override
	}
	return fallback
}

// ComponentsForMode returns the active component set: mode defaults with any
// explicit config overrides applied. An explicit override always wins.
func ComponentsForMode(mode FoundationMode, config FoundationConfig) FoundationComponents {
	base := defaultComponents(mode)
	if config.Components == nil {
		return base
	}
	o := config.Components
	return FoundationComponents{
		ToiOtoiFramework:    BoolPtr(pick(o.ToiOtoiFramework, *base.ToiOtoiFramework)),
		SleepwalkerProtocol: BoolPtr(pick(o.SleepwalkerProtocol, *base.SleepwalkerProtocol)),
		RrtAdvocate:         BoolPtr(pick(o.RrtAdvocate, *base.RrtAdvocate)),
	}
}

// ResolveMode returns the effective mode for a config (default ModeUnified).
func ResolveMode(config FoundationConfig) FoundationMode {
	if config.Mode == "" {
		return ModeUnified
	}
	return ParseMode(string(config.Mode))
}

// modeName returns the canonical name of a mode.
func modeName(mode FoundationMode) string {
	if mode == "" {
		return string(ModeUnified)
	}
	return string(mode)
}

// NeuroLiftFoundation orchestrates the ASFDK components for one user.
type NeuroLiftFoundation struct {
	userId      string
	mode        FoundationMode
	components  FoundationComponents
	toi         map[string]any
	initialized bool
}

// NewNeuroLiftFoundation creates a foundation from config. If config.Toi is a
// file path, it is loaded from disk: a missing, unreadable, or malformed file
// returns an error rather than silently falling back to the default TOI. An
// explicitly configured TOI must never be quietly replaced. Unsupported Toi
// value types also produce an error.
func NewNeuroLiftFoundation(config FoundationConfig) (*NeuroLiftFoundation, error) {
	mode := ResolveMode(config)
	userId := config.UserId
	if userId == "" {
		userId = "anonymous"
	}
	f := &NeuroLiftFoundation{userId: userId, mode: mode, components: ComponentsForMode(mode, config)}
	switch t := config.Toi.(type) {
	case nil:
		f.init(nil)
	case string:
		data, err := os.ReadFile(t)
		if err != nil {
			return nil, fmt.Errorf("load TOI file: %w", err)
		}
		var doc map[string]any
		if err := json.Unmarshal(data, &doc); err != nil {
			return nil, fmt.Errorf("parse TOI file: %w", err)
		}
		f.init(sanitizeTOIDocument(doc))
	case map[string]any:
		f.init(sanitizeTOIDocument(t))
	default:
		return nil, fmt.Errorf("unsupported TOI value type %T", config.Toi)
	}
	// Strict validation at construction: a foundation must never run with an
	// invalid TOI document (Bugbot HIGH — presence-only checks previously let
	// {"version":1,"respect_autonomy":"yes","no_harm":null} through).
	if res := ValidateTOI(f.toi); !res.Valid {
		return nil, fmt.Errorf("invalid TOI document: %s", res.Errors[0].Message)
	}
	return f, nil
}

// init sets the TOI document (nil selects the default TOI) and marks the
// foundation initialized.
func (f *NeuroLiftFoundation) init(toi map[string]any) {
	if toi == nil {
		toi = defaultTOI()
	}
	f.toi = toi
	f.initialized = f.toi != nil
}

// IsComponentActive reports whether a named component is enabled.
func (f *NeuroLiftFoundation) IsComponentActive(name string) bool {
	switch strings.ToLower(name) {
	case "toi_otoi_framework":
		return pick(f.components.ToiOtoiFramework, true)
	case "sleepwalker_protocol":
		return pick(f.components.SleepwalkerProtocol, true)
	case "rrt_advocate":
		return pick(f.components.RrtAdvocate, true)
	default:
		return false
	}
}

// ValidateTOIDocument validates the foundation's resolved TOI.
func (f *NeuroLiftFoundation) ValidateTOIDocument() TOIValidationResult {
	return ValidateTOI(f.toi)
}

// HealthCheck returns per-component statuses. A component is healthy when it
// is active and initialized; the TOI/OTOI component additionally requires the
// foundation's resolved TOI document to pass validation.
func (f *NeuroLiftFoundation) HealthCheck() HealthCheckResult {
	components := map[string]ComponentStatus{}
	healthy := true
	toiValid := f.ValidateTOIDocument().Valid
	for _, name := range activeComponentNames {
		active := f.IsComponentActive(name)
		status := ComponentStatus{Active: active, Mode: modeName(f.mode)}
		if active && !f.initialized {
			status.Error = "not initialized"
			healthy = false
		}
		if active && name == "toi_otoi_framework" && !toiValid {
			status.Error = "TOI document invalid"
			healthy = false
		}
		components[name] = status
	}
	return HealthCheckResult{Healthy: healthy, Components: components, Timestamp: time.Now()}
}

// ProcessInteraction routes an interaction through the components its
// interaction type and the active component set select, mirroring the
// canonical foundation routing:
//   - emotional_assessment → Sleepwalker analysis, with an automatic RRT
//     handoff when the assessed state warrants it (RequiresRrtaHandoff)
//   - preference_update    → TOI validation of the interaction payload; an
//     invalid TOI fails the interaction (mirrors canonical update_preferences
//     raising on invalid preferences)
//   - crisis_alert, emergency_escalation → RRT assessment
//
// All analysis routes sanitize input first (flag, don't block). Interactions
// with unknown or missing channel provenance are rejected: they are never
// analyzed as if they were trusted user input.
func (f *NeuroLiftFoundation) ProcessInteraction(interaction UserInteraction) (FoundationResponse, error) {
	response := FoundationResponse{
		Timestamp:          time.Now(),
		ResponseType:       string(interaction.InteractionType),
		Content:            map[string]any{},
		ComponentsInvolved: []string{},
		Trusted:            true,
		Success:            true,
	}
	channel := ChannelNormalize(interaction.Channel)
	if channel == ChannelUnknown {
		// Security: never upgrade unknown or missing provenance to
		// user_input — that would silently grant user-input trust to
		// inputs whose origin cannot be verified. Fail closed instead.
		response.Success = false
		response.Trusted = false
		response.Content["error"] = "unknown channel provenance"
		return response, fmt.Errorf("unknown channel provenance: %q", interaction.Channel)
	}
	text := textFromData(interaction.Data)

	if f.IsComponentActive("toi_otoi_framework") && interaction.InteractionType == InteractionPreferenceUpdate {
		response.ComponentsInvolved = append(response.ComponentsInvolved, "toi_otoi_framework")
		payload, _ := interaction.Data["toi"].(map[string]any)
		result := ValidateTOI(payload)
		response.Content["toi_otoi"] = map[string]any{"valid": result.Valid, "errors": len(result.Errors)}
		if !result.Valid {
			response.Success = false
			response.Content["error"] = "TOI validation failed"
		}
	}
	if f.IsComponentActive("sleepwalker_protocol") && interaction.InteractionType == InteractionEmotionalAssessment {
		response.ComponentsInvolved = append(response.ComponentsInvolved, "sleepwalker_protocol")
		state := AssessEmotionalStateWithProvenance(text, channel)
		response.Trusted = response.Trusted && state.Trusted
		response.Content["sleepwalker"] = map[string]any{
			"state": state.State, "confidence": state.Confidence,
			"channel": string(channel), "trusted": state.Trusted, "flagged": state.Flagged,
		}
		if f.IsComponentActive("rrt_advocate") && RequiresRrtaHandoff(state.EmotionalState) {
			// Canonical handoff: Sleepwalker escalates to RRT when the
			// emotional state warrants it; rrt_advocate is listed whenever
			// the handoff was attempted.
			assessment := AssessCrisisWithProvenance(interaction.Data, channel)
			response.ComponentsInvolved = append(response.ComponentsInvolved, "rrt_advocate")
			response.Trusted = response.Trusted && assessment.Trusted
			response.Content["rrt"] = map[string]any{
				"crisis_level": assessment.CrisisLevel.String(), "confidence": assessment.ConfidenceScore,
				"channel": string(channel), "trusted": assessment.Trusted,
			}
		}
	}
	if f.IsComponentActive("rrt_advocate") &&
		(interaction.InteractionType == InteractionCrisisAlert || interaction.InteractionType == InteractionEmergencyEscalation) {
		response.ComponentsInvolved = append(response.ComponentsInvolved, "rrt_advocate")
		assessment := AssessCrisisWithProvenance(interaction.Data, channel)
		response.Trusted = response.Trusted && assessment.Trusted
		response.Content["rrt"] = map[string]any{
			"crisis_level": assessment.CrisisLevel.String(), "confidence": assessment.ConfidenceScore,
			"channel": string(channel), "trusted": assessment.Trusted,
		}
	}

	if len(response.ComponentsInvolved) == 0 {
		response.Success = false
		response.Content["error"] = "no components routed for interaction type " + string(interaction.InteractionType) +
			" under mode " + modeName(f.mode)
	}
	return response, nil
}

// StatusSummary returns a human-readable one-line status.
func (f *NeuroLiftFoundation) StatusSummary() string {
	var names []string
	for _, name := range activeComponentNames {
		if f.IsComponentActive(name) {
			names = append(names, name)
		}
	}
	return fmt.Sprintf("user=%s mode=%s active=[%s]", f.userId, modeName(f.mode), strings.Join(names, ","))
}
