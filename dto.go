package asfdk

import "time"

// FoundationComponents holds per-component overrides. A nil pointer defers to
// the mode default; an explicit true/false always wins.
type FoundationComponents struct {
	ToiOtoiFramework    *bool `json:"toi_otoi_framework,omitempty"`
	SleepwalkerProtocol *bool `json:"sleepwalker_protocol,omitempty"`
	RrtAdvocate         *bool `json:"rrt_advocate,omitempty"`
}

// FoundationConfig configures a foundation instance.
type FoundationConfig struct {
	UserId     string                `json:"user_id"`
	Mode       FoundationMode        `json:"mode"`
	Components *FoundationComponents `json:"components,omitempty"`
	// Toi is an optional TOI source: nil (default TOI), a file path (string),
	// or an inline TOI document (map[string]any).
	Toi any `json:"toi,omitempty"`
}

// UserInteraction is a single governed interaction.
type UserInteraction struct {
	Timestamp       time.Time       `json:"timestamp"`
	InteractionType InteractionType `json:"interaction_type"`
	Data            map[string]any  `json:"data"`
	UserId          string          `json:"user_id"`
	SessionId       string          `json:"session_id,omitempty"`
	Priority        *int            `json:"priority,omitempty"`
	Context         map[string]any  `json:"context,omitempty"`
	Channel         Channel         `json:"channel,omitempty"`
}

// FoundationResponse is the unified output of ProcessInteraction.
// ComponentsInvolved lists the components that processed the interaction.
// Trusted is the aggregate provenance verdict: false if any involved
// component flagged the input as coming from an untrusted channel.
type FoundationResponse struct {
	Timestamp          time.Time      `json:"timestamp"`
	ResponseType       string         `json:"response_type"`
	Content            map[string]any `json:"content"`
	ComponentsInvolved []string       `json:"components_involved"`
	Trusted            bool           `json:"trusted"`
	Success            bool           `json:"success"`
}

// ComponentStatus reports one framework component.
type ComponentStatus struct {
	Active bool   `json:"active"`
	Mode   string `json:"mode"`
	Error  string `json:"error,omitempty"`
}

// HealthCheckResult aggregates component statuses.
type HealthCheckResult struct {
	Healthy    bool                       `json:"healthy"`
	Components map[string]ComponentStatus `json:"components"`
	Timestamp  time.Time                  `json:"timestamp"`
}

// SanitizationResult is the outcome of SanitizeInput.
type SanitizationResult struct {
	Clean     bool      `json:"clean"`
	Content   string    `json:"content"`
	Reason    string    `json:"reason,omitempty"`
	RiskLevel RiskLevel `json:"risk_level"`
}

// ValidationResult reports output validation.
type ValidationResult struct {
	Valid  bool   `json:"valid"`
	Reason string `json:"reason,omitempty"`
}

// SecurityEvent is a security audit record.
type SecurityEvent struct {
	EventType SecurityEventType `json:"event_type"`
	UserId    string            `json:"user_id"`
	Details   string            `json:"details"`
	Timestamp int64             `json:"timestamp"`
}

// ValidationIssue is a single schema violation.
type ValidationIssue struct {
	Message string `json:"message"`
	Path    string `json:"path"`
	Code    string `json:"code"`
}

// TOIValidationResult is the outcome of ValidateTOI.
type TOIValidationResult struct {
	Valid  bool              `json:"valid"`
	Errors []ValidationIssue `json:"errors,omitempty"`
	Toi    map[string]any    `json:"toi,omitempty"`
}

// OTOIValidationResult is the outcome of ValidateCharter.
type OTOIValidationResult struct {
	Valid   bool              `json:"valid"`
	Errors  []ValidationIssue `json:"errors,omitempty"`
	Charter map[string]any    `json:"charter,omitempty"`
}

// EmotionalState is the Sleepwalker emotional analysis output.
type EmotionalState struct {
	State      string             `json:"state"`
	Confidence float64            `json:"confidence"`
	Indicators []string           `json:"indicators"`
	RawScores  map[string]float64 `json:"raw_scores"`
}

// EmotionalStateWithProvenance adds channel provenance to EmotionalState.
type EmotionalStateWithProvenance struct {
	EmotionalState
	Channel    Channel `json:"channel"`
	Trusted    bool    `json:"trusted"`
	Flagged    bool    `json:"flagged,omitempty"`
	FlagReason string  `json:"flag_reason,omitempty"`
}

// CrisisAssessment is the RRT crisis analysis output.
type CrisisAssessment struct {
	Timestamp                time.Time      `json:"timestamp"`
	CrisisLevel              CrisisLevel    `json:"crisis_level"`
	PrimaryIndicators        []string       `json:"primary_indicators"`
	SecondaryIndicators      []string       `json:"secondary_indicators"`
	ConfidenceScore          float64        `json:"confidence_score"`
	EstimatedDuration        *float64       `json:"estimated_duration,omitempty"`
	RecommendedInterventions []string       `json:"recommended_interventions"`
	EscalationThreshold      float64        `json:"escalation_threshold"`
	UserSafetyScore          float64        `json:"user_safety_score"`
	ContextFactors           map[string]any `json:"context_factors"`
}

// CrisisAssessmentWithProvenance adds channel provenance to CrisisAssessment.
type CrisisAssessmentWithProvenance struct {
	CrisisAssessment
	Channel    Channel `json:"channel"`
	Trusted    bool    `json:"trusted"`
	Flagged    bool    `json:"flagged,omitempty"`
	FlagReason string  `json:"flag_reason,omitempty"`
}
