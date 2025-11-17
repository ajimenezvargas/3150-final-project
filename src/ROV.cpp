#include "ROV.h"
#include <algorithm>
#include <sstream>

void ROVValidator::addROA(const ROA& roa) {
    std::string norm_prefix = normalizePrefix(roa.prefix);
    roas_[norm_prefix].push_back(roa);
}

void ROVValidator::addROA(const std::string& prefix, uint32_t asn, int max_length) {
    addROA(ROA(prefix, asn, max_length));
}

ROVState ROVValidator::validate(const std::string& prefix, uint32_t origin_asn) const {
    std::string norm_prefix = normalizePrefix(prefix);
    int prefix_len = getPrefixLength(prefix);
    
    // Look for exact prefix match first
    auto it = roas_.find(norm_prefix);
    if (it != roas_.end()) {
        // Found ROAs for this exact prefix
        for (const ROA& roa : it->second) {
            // Check if origin ASN matches
            if (roa.authorized_as == origin_asn) {
                // Check prefix length constraint
                if (prefix_len <= roa.max_length) {
                    return ROVState::VALID;
                }
            }
        }
        // Found ROAs but none matched - INVALID
        return ROVState::INVALID;
    }
    
    // Check if any ROA covers this prefix (less specific)
    // For simplicity, we check all ROAs - real implementation would use trie
    for (const auto& [roa_prefix, roa_list] : roas_) {
        for (const ROA& roa : roa_list) {
            if (isCoveredBy(prefix, roa)) {
                // This ROA covers our prefix
                if (roa.authorized_as == origin_asn && prefix_len <= roa.max_length) {
                    return ROVState::VALID;
                }
                // Covered but doesn't match - INVALID
                return ROVState::INVALID;
            }
        }
    }
    
    // No covering ROA found
    return ROVState::UNKNOWN;
}

std::vector<ROA> ROVValidator::getROAsForPrefix(const std::string& prefix) const {
    std::string norm_prefix = normalizePrefix(prefix);
    auto it = roas_.find(norm_prefix);
    if (it != roas_.end()) {
        return it->second;
    }
    return {};
}

bool ROVValidator::isCoveredBy(const std::string& ann_prefix, const ROA& roa) const {
    // Simplified: check if prefixes match exactly or if ann_prefix is more specific
    // Real implementation would need proper IP prefix matching
    
    std::string norm_ann = normalizePrefix(ann_prefix);
    std::string norm_roa = normalizePrefix(roa.prefix);
    
    // Extract IP parts (simplified - assumes valid format)
    auto extract_ip = [](const std::string& pfx) -> std::string {
        size_t slash = pfx.find('/');
        if (slash != std::string::npos) {
            return pfx.substr(0, slash);
        }
        return pfx;
    };
    
    std::string ann_ip = extract_ip(norm_ann);
    std::string roa_ip = extract_ip(norm_roa);
    
    int ann_len = getPrefixLength(ann_prefix);
    int roa_len = getPrefixLength(roa.prefix);
    
    // Simplified: if IP addresses match and announcement is same or more specific
    if (ann_ip == roa_ip && ann_len >= roa_len) {
        return true;
    }
    
    // Exact match
    if (norm_ann == norm_roa) {
        return true;
    }
    
    return false;
}

int ROVValidator::getPrefixLength(const std::string& prefix) const {
    size_t slash = prefix.find('/');
    if (slash != std::string::npos) {
        return std::stoi(prefix.substr(slash + 1));
    }
    return 32; // Default to /32 if no length specified
}

std::string ROVValidator::normalizePrefix(const std::string& prefix) const {
    // Trim whitespace
    std::string result = prefix;
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    return result;
}