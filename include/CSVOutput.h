#pragma once

#include "ASGraph.h"
#include <string>
#include <fstream>

/**
 * CSV Output for routing tables
 * Format: asn,prefix,as_path
 */
class CSVOutput {
public:
    // Write routing table to CSV
    static bool writeRoutingTable(const ASGraph& graph, const std::string& filename);
    
    // Write single AS routing table to CSV
    static bool writeASRoutingTable(const AS& as, const std::string& filename);
    
    // Generate CSV string (for testing)
    static std::string generateCSV(const ASGraph& graph);

    private: //test
    // Format AS path as space-separated string
    static std::string formatASPath(const std::vector<uint32_t>& path);
};