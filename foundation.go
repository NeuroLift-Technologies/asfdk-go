// Package asfdk — foundation orchestration. Mirrors
// src/Asfdk/NeuroLiftFoundation.cs from the C# port, including override
// semantics: an explicit FoundationComponents override always wins over the
// mode default.
package asfdk

import (
	"encoding/json"
	"fmt"
	"os"
	"sort"
	"strings"
	"time"
)

// jsonUnmarshal and jsonMarshal are thin indirections kept as functions so
// structured encoding behavior is hooked in one place.
func jsonUnmarshal(data []byte, v any) error { return json.Unmarshal(data, v) }
func jsonMarshal(v any) ([]byte, error)      { return json.Marshal(v) }

func BoolPtr(b bool) *bool { return &b }

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

// NewNeuroLiftFoundation creates a foundation from config.
func NewNeuroLiftFoundation(config FoundationConfig) *NeuroLiftFoundation {
	mode := ResolveMode(config)
	userId := config.UserId
	if userId == "" {
		userId = "anonymous"
	}
	f := &NeuroLiftFoundation{userId: userId, mode: mode, components: ComponentsForMode(mode, config), toi: defaultTOI()}
	switch t := config.Toi.(type) {
	case nil:
	case string:
		if data, err := os.ReadFile(t); err == nil {
			var doc map[string]any
			if json.Unmarshal(data, &doc) == nil && doc != nil {
				f.toi = sanitizeTOIDocument(doc)
			}
		}
	case map[string]any:
		f.toi = sanitizeTOIDocument(t)
	}
	f.initialized = f.toi != nil
	return f
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

// HealthCheck returns per-component statuses.
func (f *NeuroLiftFoundation) HealthCheck() HealthCheckResult {
	components := map[string]ComponentStatus{}
	healthy := true
	for _, name := range []string{"toi_otoi_framework", "sleepwalker_protocol", "rrt_advocate"} {
		active := f.IsComponentActive(name)
		status := ComponentStatus{Active: active, Mode: modeName(f.mode)}
		if active && !f.initialized {
			status.Error = "not initialized"
			healthy = false
		}
		components[name] = status
	}
	return HealthCheckResult{Healthy: healthy, Components: components, Timestamp: time.Now()}
}

// ProcessInteraction routes an interaction through the active components and
// returns the unified response.
func (f *NeuroLiftFoundation) ProcessInteraction(interaction UserInteraction) (FoundationResponse, error) {
	response := FoundationResponse{
		Timestamp:          time.Now(),
		ResponseType:       "foundation_response",
		Content:            map[string]any{},
		ComponentsInvolved: []string{},
		Success:            true,
	}
	channel := ChannelNormalize(interaction.Channel)
	if channel == ChannelUnknown {
		channel = ChannelUserInput
	}

	if f.IsComponentActive("toi_otoi_framework") {
		response.ComponentsInvolved = append(response.ComponentsInvolved, "toi_otoi_framework")
		result := ValidateTOI(f.toi)
		response.Content["toi_otoi"] = map[string]any{"valid": result.Valid, "errors": len(result.Errors)}
	}
	if f.IsComponentActive("sleepwalker_protocol") {
		response.ComponentsInvolved = append(response.ComponentsInvolved, "sleepwalker_protocol")
		state := AnalyzeEmotionalStateWithProvenance(fmt.Sprint(interaction.Data["text"]), channel)
		response.Content["sleepwalker"] = map[string]any{
			"state": state.State, "confidence": state.Confidence,
			"channel": string(channel), "trusted": state.Trusted,
		}
	}
	if f.IsComponentActive("rrt_advocate") {
		response.ComponentsInvolved = append(response.ComponentsInvolved, "rrt_advocate")
		assessment := AssessCrisisWithProvenance(interaction.Data, channel)
		response.Content["rrt"] = map[string]any{
			"crisis_level": assessment.CrisisLevel.String(), "confidence": assessment.ConfidenceScore,
			"channel": string(channel), "trusted": assessment.Trusted,
		}
	}

	if len(response.ComponentsInvolved) == 0 {
		response.Success = false
		response.Content["error"] = "no components active for mode " + modeName(f.mode)
	}
	return response, nil
}

// StatusSummary returns a human-readable one-line status.
func (f *NeuroLiftFoundation) StatusSummary() string {
	var names []string
	for _, name := range []string{"rrt_advocate", "sleepwalker_protocol", "toi_otoi_framework"} {
		if f.IsComponentActive(name) {
			names = append(names, name)
		}
	}
	sort.Strings(names)
	return fmt.Sprintf("user=%s mode=%s active=[%s]", f.userId, modeName(f.mode), strings.Join(names, ","))
}
