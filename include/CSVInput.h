#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdint>

/**
 * Input announcement from CSV
 * Format: asn,prefix,rov_invalid
 */
struct InputAnnouncement {
    uint32_t asn;
    std::string prefix;
    bool rov_invalid;
    
    InputAnnouncement(uint32_t a, const std::string& p, bool r) 
        : asn(a), prefix(p), rov_invalid(r) {}
};

/**
 * CSV Input Parser
 * Reads announcements.csv and rov_asns.csv files
 */
class CSVInput {
public:
    // Parse announcements CSV
    // Format: asn,prefix,rov_invalid
    static std::vector<InputAnnouncement> parseAnnouncements(const std::string& filename);
    
    // Parse ROV ASNs CSV
    // Format: asn (one per line, no header)
    static std::vector<uint32_t> parseROVASNs(const std::string& filename);
    
private:
    // Helper to trim whitespace
    static std::string trim(const std::string& str);
    
    // Helper to parse boolean
    static bool parseBool(const std::string& str);
};