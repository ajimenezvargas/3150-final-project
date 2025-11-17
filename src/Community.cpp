#include "Community.h"
#include <sstream>
#include <iomanip>

std::string Community::toString(uint32_t community) {
    // Check for well-known communities
    if (community == NO_EXPORT) {
        return "NO_EXPORT";
    }
    if (community == NO_ADVERTISE) {
        return "NO_ADVERTISE";
    }
    if (community == NO_EXPORT_SUBCONFED) {
        return "NO_EXPORT_SUBCONFED";
    }
    
    // Regular community: AS:Value
    uint16_t asn = getASN(community);
    uint16_t value = getValue(community);
    
    std::ostringstream oss;
    oss << asn << ":" << value;
    return oss.str();
}

uint32_t Community::fromString(const std::string& str) {
    // Check for well-known communities
    if (str == "NO_EXPORT") {
        return NO_EXPORT;
    }
    if (str == "NO_ADVERTISE") {
        return NO_ADVERTISE;
    }
    if (str == "NO_EXPORT_SUBCONFED") {
        return NO_EXPORT_SUBCONFED;
    }
    
    // Parse AS:Value format
    size_t colon = str.find(':');
    if (colon != std::string::npos) {
        uint16_t asn = static_cast<uint16_t>(std::stoul(str.substr(0, colon)));
        uint16_t value = static_cast<uint16_t>(std::stoul(str.substr(colon + 1)));
        return make(asn, value);
    }
    
    // Try parsing as raw number
    return static_cast<uint32_t>(std::stoul(str));
}

std::string CommunitySet::toString() const {
    if (communities_.empty()) {
        return "[]";
    }
    
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    for (uint32_t comm : communities_) {
        if (!first) oss << ", ";
        oss << Community::toString(comm);
        first = false;
    }
    oss << "]";
    return oss.str();
}