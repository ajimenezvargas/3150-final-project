#pragma once

#include <cstdint>
#include <set>
#include <string>

/**
 * BGP Community - 32-bit value for route tagging
 * Format: AS:Value (16 bits each)
 * 
 * Well-known communities (RFC 1997):
 * - NO_EXPORT: 0xFFFFFF01 (don't export outside confederation)
 * - NO_ADVERTISE: 0xFFFFFF02 (don't advertise to any peer)
 * - NO_EXPORT_SUBCONFED: 0xFFFFFF03 (don't export outside sub-confederation)
 */
class Community {
public:
    // Well-known communities
    static const uint32_t NO_EXPORT = 0xFFFFFF01;
    static const uint32_t NO_ADVERTISE = 0xFFFFFF02;
    static const uint32_t NO_EXPORT_SUBCONFED = 0xFFFFFF03;
    
    // Create community from AS:Value format
    static uint32_t make(uint16_t asn, uint16_t value) {
        return (static_cast<uint32_t>(asn) << 16) | value;
    }
    
    // Extract AS number from community
    static uint16_t getASN(uint32_t community) {
        return static_cast<uint16_t>(community >> 16);
    }
    
    // Extract value from community
    static uint16_t getValue(uint32_t community) {
        return static_cast<uint16_t>(community & 0xFFFF);
    }
    
    // Check if well-known community
    static bool isWellKnown(uint32_t community) {
        return (community & 0xFFFF0000) == 0xFFFF0000;
    }
    
    // Get string representation
    static std::string toString(uint32_t community);
    
    // Parse from string (e.g., "64512:100" or "NO_EXPORT")
    static uint32_t fromString(const std::string& str);
};

/**
 * Community Set - collection of communities on a route
 */
class CommunitySet {
public:
    CommunitySet() = default;
    
    // Add/remove communities
    void add(uint32_t community) { communities_.insert(community); }
    void remove(uint32_t community) { communities_.erase(community); }
    void clear() { communities_.clear(); }
    
    // Check for community
    bool has(uint32_t community) const { 
        return communities_.find(community) != communities_.end(); 
    }
    
    // Check for well-known communities
    bool hasNoExport() const { return has(Community::NO_EXPORT); }
    bool hasNoAdvertise() const { return has(Community::NO_ADVERTISE); }
    
    // Get all communities
    const std::set<uint32_t>& getCommunities() const { return communities_; }
    
    // Size
    size_t size() const { return communities_.size(); }
    bool empty() const { return communities_.empty(); }
    
    // String representation
    std::string toString() const;
    
private:
    std::set<uint32_t> communities_;
};