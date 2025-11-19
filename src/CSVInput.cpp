#include "CSVInput.h"
#include <algorithm>
#include <cstdint>

std::string CSVInput::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

bool CSVInput::parseBool(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    lower = trim(lower);
    
    if (lower == "true" || lower == "1" || lower == "yes") {
        return true;
    }
    return false;
}

std::vector<InputAnnouncement> CSVInput::parseAnnouncements(const std::string& filename) {
    std::vector<InputAnnouncement> announcements;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open announcements file: " << filename << std::endl;
        return announcements;
    }
    
    std::string line;
    bool first_line = true;
    int line_num = 0;
    
    while (std::getline(file, line)) {
        line_num++;
        line = trim(line);
        
        // Skip empty lines
        if (line.empty()) continue;
        
        // Skip header line (if it contains "asn" or "prefix")
        if (first_line) {
            first_line = false;
            std::string lower = line;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower.find("asn") != std::string::npos || 
                lower.find("prefix") != std::string::npos) {
                continue;
            }
        }
        
        // Parse CSV line: asn,prefix,rov_invalid
        std::istringstream iss(line);
        std::string asn_str, prefix, rov_str;
        
        if (std::getline(iss, asn_str, ',') &&
            std::getline(iss, prefix, ',') &&
            std::getline(iss, rov_str, ',')) {
            
            try {
                uint32_t asn = std::stoul(trim(asn_str));
                prefix = trim(prefix);
                bool rov_invalid = parseBool(rov_str);
                
                announcements.emplace_back(asn, prefix, rov_invalid);
            } catch (const std::exception& e) {
                std::cerr << "Warning: Could not parse line " << line_num 
                          << ": " << line << std::endl;
                std::cerr << "  Error: " << e.what() << std::endl;
            }
        } else {
            std::cerr << "Warning: Malformed line " << line_num 
                      << ": " << line << std::endl;
        }
    }
    
    file.close();
    std::cout << "Loaded " << announcements.size() << " announcements from " 
              << filename << std::endl;
    
    return announcements;
}

std::vector<uint32_t> CSVInput::parseROVASNs(const std::string& filename) {
    std::vector<uint32_t> rov_asns;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open ROV ASNs file: " << filename << std::endl;
        return rov_asns;
    }
    
    std::string line;
    bool first_line = true;
    int line_num = 0;
    
    while (std::getline(file, line)) {
        line_num++;
        line = trim(line);
        
        // Skip empty lines
        if (line.empty()) continue;
        
        // Skip header line (if it contains "asn")
        if (first_line) {
            first_line = false;
            std::string lower = line;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower.find("asn") != std::string::npos) {
                continue;
            }
        }
        
        // Parse ASN
        try {
            uint32_t asn = std::stoul(line);
            rov_asns.push_back(asn);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not parse ASN on line " << line_num 
                      << ": " << line << std::endl;
        }
    }
    
    file.close();
    std::cout << "Loaded " << rov_asns.size() << " ROV ASNs from " 
              << filename << std::endl;
    
    return rov_asns;
}