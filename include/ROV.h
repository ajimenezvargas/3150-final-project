#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>

/**
 * ROV (Route Origin Validation) states
 * Based on RFC 6811
 */
enum class ROVState {
    VALID,      // Matches ROA exactly
    INVALID,    // Conflicts with ROA (wrong origin or too specific)
    UNKNOWN     // No ROA found (NotFound)
};

/**
 * Route Origin Authorization (ROA)
 * Represents RPKI data
 */
struct ROA {
    std::string prefix;      // e.g., "8.8.8.0/24"
    uint32_t authorized_as;  // ASN authorized to originate
    int max_length;          // Maximum prefix length allowed
    
    ROA(const std::string& pfx, uint32_t asn, int max_len = -1)
        : prefix(pfx), authorized_as(asn), max_length(max_len) {
        // If max_length not specified, use prefix length
        if (max_length == -1) {
            size_t slash = prefix.find('/');
            if (slash != std::string::npos) {
                max_length = std::stoi(prefix.substr(slash + 1));
            }
        }
    }
};

/**
 * ROV Validator
 * Validates announcements against ROAs
 */
class ROVValidator {
public:
    ROVValidator() = default;
    
    // Add a ROA to the validator
    void addROA(const ROA& roa);
    void addROA(const std::string& prefix, uint32_t asn, int max_length = -1);
    
    // Validate an announcement
    ROVState validate(const std::string& prefix, uint32_t origin_asn) const;
    
    // Get all ROAs for a prefix (for debugging)
    std::vector<ROA> getROAsForPrefix(const std::string& prefix) const;
    
    // Statistics
    size_t getROACount() const { return roas_.size(); }
    
    // Clear all ROAs
    void clear() { roas_.clear(); }
    
private:
    // Map prefix -> list of ROAs
    std::unordered_map<std::string, std::vector<ROA>> roas_;
    
    // Helper: check if announcement prefix is covered by ROA
    bool isCoveredBy(const std::string& ann_prefix, const ROA& roa) const;
    
    // Helper: extract prefix length from string like "1.0.0.0/24"
    int getPrefixLength(const std::string& prefix) const;
    
    // Helper: normalize prefix for comparison
    std::string normalizePrefix(const std::string& prefix) const;
};