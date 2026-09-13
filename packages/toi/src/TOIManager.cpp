
#include "toi/TermsOfInteraction.h"
#include <string>
#include <vector>

namespace toi {

/// TOI Manager — entry point for TOI validation and operations.
/// Provides the public API surface for TOI document handling.

class TOIManager {
public:
    TOIManager() = default;

    /// Parse a TOI document from a JSON string.
    /// @param jsonStr The JSON string representing a .toi document
    /// @return Parsed TOIDocument on success
    /// @throws std::invalid_argument if the document is invalid
    TOIDocument parse(const std::string& jsonStr) const {
        auto j = nlohmann::json::parse(jsonStr);
        return parseTOI(j);
    }

    /// Validate a TOIDocument against the canonical schema.
    /// @param doc The TOI document to validate
    /// @return true if the document is valid, false otherwise
    bool validate(const TOIDocument& doc) const {
        return ::toi::validateTOI(doc);
    }

    /// Resolve a stack of TOI documents into one effective document.
    /// @param docs Vector of TOIDocument to resolve (highest precedence first)
    /// @return Effective TOIDocument
    TOIDocument resolve(std::vector<TOIDocument> docs) const {
        return ::toi::resolveTOI(docs);
    }

    /// Canonicalize a TOI document using JCS (RFC 8785).
    /// @param jsonStr The JSON string to canonicalize
    /// @return Canonical JSON string
    std::string canonicalize(const std::string& jsonStr) const {
        return ::toi::canonicalize(jsonStr);
    }

    /// Verify an Ed25519 signature on a TOI document.
    /// @param canonicalBytes The canonical byte representation
    /// @param signature The signature envelope as JSON {alg, public_key, value}
    /// @param publicKey The Ed25519 public key (32 bytes)
    /// @return true if the signature is valid, false otherwise
    bool verifySignature(const std::string& canonicalBytes,
                         const nlohmann::json& signature,
                         const std::vector<uint8_t>& publicKey) const {
        return ::toi::verifySignature(canonicalBytes, signature, publicKey);
    }

    /// Get the active TOI document from the foundation configuration.
    /// @param config The foundation configuration
    /// @return The active TOI document, or null if not initialized
    TOIDocument getActiveTOI(const nlohmann::json& config) const {
        if (config.contains("toi") && config["toi"].is_string()) {
            return parse(config["toi"].get<std::string>());
        }
        return TOIDocument{};
    }
};

} // namespace toi