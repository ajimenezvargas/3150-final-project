#include "Aggregation.h"
#include <sstream>
#include <algorithm>
#include <cmath>

IPPrefix::IPPrefix(const std::string& pfx) : prefix(pfx) {
    // Parse prefix like "10.0.0.0/8"
    size_t slash = pfx.find('/');
    if (slash != std::string::npos) {
        base_ip = pfx.substr(0, slash);
        prefix_length = std::stoi(pfx.substr(slash + 1));
    } else {
        base_ip = pfx;
        prefix_length = 32;
    }
}

bool IPPrefix::covers(const IPPrefix& other) const {
    // This prefix covers other if it's less specific and same network
    if (prefix_length >= other.prefix_length) {
        return false;  // Can't cover if more specific or equal
    }
    
    // Check if same network
    uint32_t this_ip = ipToInt(base_ip);
    uint32_t other_ip = ipToInt(other.base_ip);
    
    // Create mask for this prefix
    uint32_t mask = 0xFFFFFFFF << (32 - prefix_length);
    
    return (this_ip & mask) == (other_ip & mask);
}

bool IPPrefix::coveredBy(const IPPrefix& other) const {
    return other.covers(*this);
}

bool IPPrefix::canAggregate(const IPPrefix& other) const {
    // Can aggregate if:
    // 1. Same prefix length
    // 2. Differ by only one bit
    // 3. That bit is at the prefix length position
    
    if (prefix_length != other.prefix_length) {
        return false;
    }
    
    uint32_t ip1 = ipToInt(base_ip);
    uint32_t ip2 = ipToInt(other.base_ip);
    
    // Create mask for this prefix length
    uint32_t mask = 0xFFFFFFFF << (32 - prefix_length);
    
    // Get parent network (one bit less specific)
    uint32_t parent_mask = 0xFFFFFFFF << (32 - prefix_length + 1);
    
    // Both should be in same parent network
    if ((ip1 & parent_mask) != (ip2 & parent_mask)) {
        return false;
    }
    
    // Should differ in the bit at prefix_length position
    uint32_t diff_bit = 1 << (32 - prefix_length);
    return (ip1 ^ ip2) == diff_bit;
}

IPPrefix IPPrefix::aggregate(const IPPrefix& p1, const IPPrefix& p2) {
    // Aggregate into parent prefix (one bit less specific)
    uint32_t ip1 = ipToInt(p1.base_ip);
    uint32_t parent_mask = 0xFFFFFFFF << (32 - p1.prefix_length + 1);
    uint32_t parent_ip = ip1 & parent_mask;
    
    std::string parent_str = intToIp(parent_ip) + "/" + std::to_string(p1.prefix_length - 1);
    return IPPrefix(parent_str);
}

uint32_t IPPrefix::ipToInt(const std::string& ip) {
    std::istringstream iss(ip);
    std::string octet;
    uint32_t result = 0;
    int shift = 24;
    
    while (std::getline(iss, octet, '.')) {
        uint32_t val = std::stoul(octet);
        result |= (val << shift);
        shift -= 8;
    }
    
    return result;
}

std::string IPPrefix::intToIp(uint32_t ip) {
    std::ostringstream oss;
    oss << ((ip >> 24) & 0xFF) << "."
        << ((ip >> 16) & 0xFF) << "."
        << ((ip >> 8) & 0xFF) << "."
        << (ip & 0xFF);
    return oss.str();
}

bool IPPrefix::sameNetwork(const IPPrefix& other) const {
    int common_len = std::min(prefix_length, other.prefix_length);
    uint32_t mask = 0xFFFFFFFF << (32 - common_len);
    
    uint32_t ip1 = ipToInt(base_ip);
    uint32_t ip2 = ipToInt(other.base_ip);
    
    return (ip1 & mask) == (ip2 & mask);
}

IPPrefix IPPrefix::getParent() const {
    if (prefix_length <= 0) {
        return *this;  // Can't get more general
    }
    
    uint32_t ip = ipToInt(base_ip);
    uint32_t parent_mask = 0xFFFFFFFF << (32 - prefix_length + 1);
    uint32_t parent_ip = ip & parent_mask;
    
    std::string parent_str = intToIp(parent_ip) + "/" + std::to_string(prefix_length - 1);
    return IPPrefix(parent_str);
}

// RouteAggregator implementation

std::vector<std::pair<std::string, std::string>> 
RouteAggregator::findAggregatablePairs(const std::vector<std::string>& prefixes) {
    std::vector<std::pair<std::string, std::string>> pairs;
    
    for (size_t i = 0; i < prefixes.size(); i++) {
        for (size_t j = i + 1; j < prefixes.size(); j++) {
            IPPrefix p1(prefixes[i]);
            IPPrefix p2(prefixes[j]);
            
            if (p1.canAggregate(p2)) {
                pairs.push_back({prefixes[i], prefixes[j]});
            }
        }
    }
    
    return pairs;
}

std::vector<std::string> 
RouteAggregator::aggregate(const std::vector<std::string>& prefixes) {
    if (prefixes.size() <= 1) {
        return prefixes;
    }
    
    // Greedy aggregation: find pairs and aggregate
    std::vector<std::string> result = prefixes;
    bool changed = true;
    
    while (changed) {
        changed = false;
        auto pairs = findAggregatablePairs(result);
        
        if (!pairs.empty()) {
            // Take first pair and aggregate
            auto& pair = pairs[0];
            IPPrefix p1(pair.first);
            IPPrefix p2(pair.second);
            IPPrefix aggregated = IPPrefix::aggregate(p1, p2);
            
            // Remove the two prefixes and add aggregated
            result.erase(std::remove(result.begin(), result.end(), pair.first), result.end());
            result.erase(std::remove(result.begin(), result.end(), pair.second), result.end());
            result.push_back(aggregated.toString());
            
            changed = true;
        }
    }
    
    return result;
}

bool RouteAggregator::isSafeToAggregate(const std::string& p1, const std::string& p2) {
    IPPrefix prefix1(p1);
    IPPrefix prefix2(p2);
    return prefix1.canAggregate(prefix2);
}

bool RouteAggregator::areContiguous(const IPPrefix& p1, const IPPrefix& p2) {
    return p1.canAggregate(p2);
}