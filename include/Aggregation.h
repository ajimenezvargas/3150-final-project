#pragma once
#include <string>
#include <vector>
#include <cstdint>

/**
 * IP Prefix utilities for aggregation
 */
class IPPrefix {
public:
    std::string prefix;      // e.g., "10.0.0.0/8"
    std::string base_ip;     // e.g., "10.0.0.0"
    int prefix_length;       // e.g., 8
    
    IPPrefix(const std::string& pfx);
    
    // Check if this prefix covers another (is less specific)
    bool covers(const IPPrefix& other) const;
    
    // Check if this prefix is covered by another (is more specific)
    bool coveredBy(const IPPrefix& other) const;
    
    // Check if two prefixes can be aggregated (adjacent)
    bool canAggregate(const IPPrefix& other) const;
    
    // Aggregate two prefixes into a less specific one
    static IPPrefix aggregate(const IPPrefix& p1, const IPPrefix& p2);
    
    // Convert IP string to 32-bit integer
    static uint32_t ipToInt(const std::string& ip);
    
    // Convert 32-bit integer to IP string
    static std::string intToIp(uint32_t ip);
    
    // Check if two IPs are in same network
    bool sameNetwork(const IPPrefix& other) const;
    
    // Get parent prefix (one bit less specific)
    IPPrefix getParent() const;
    
    // String representation
    std::string toString() const { return prefix; }
};

/**
 * Route Aggregator
 * Combines multiple specific routes into less specific aggregates
 */
class RouteAggregator {
public:
    RouteAggregator() = default;
    
    // Find prefixes that can be aggregated
    static std::vector<std::pair<std::string, std::string>> 
        findAggregatablePairs(const std::vector<std::string>& prefixes);
    
    // Aggregate a set of prefixes (greedy algorithm)
    static std::vector<std::string> 
        aggregate(const std::vector<std::string>& prefixes);
    
    // Check if aggregation is safe (no de-aggregation of existing aggregates)
    static bool isSafeToAggregate(const std::string& p1, const std::string& p2);
    
private:
    // Helper: check if prefixes are contiguous
    static bool areContiguous(const IPPrefix& p1, const IPPrefix& p2);
};