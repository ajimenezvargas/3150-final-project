#include "CSVOutput.h"
#include <sstream>
#include <iostream>

bool CSVOutput::writeRoutingTable(const ASGraph& graph, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    // Write header
    file << "asn,prefix,as_path\n";
    
    // Get all ASes in the graph
    const auto& ases = graph.getAllASes();
    
    // For each AS, write its routing table
    for (const auto& [asn, as_ptr] : ases) {
        const auto& routing_table = as_ptr->getRoutingTable();

        for (const auto& [prefix, announcement] : routing_table) {
            file << asn << ",";
            file << prefix << ",\"";
            file << formatASPath(announcement.getASPath());
            file << "\"\n";
        }
    }
    
    file.close();
    return true;
}

bool CSVOutput::writeASRoutingTable(const AS& as, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    // Write header
    file << "asn,prefix,as_path\n";
    
    const auto& routing_table = as.getRoutingTable();

    for (const auto& [prefix, announcement] : routing_table) {
        file << as.getASN() << ",";
        file << prefix << ",\"";
        file << formatASPath(announcement.getASPath());
        file << "\"\n";
    }
    
    file.close();
    return true;
}

std::string CSVOutput::generateCSV(const ASGraph& graph) {
    std::ostringstream oss;
    
    // Write header
    oss << "asn,prefix,as_path\n";
    
    // Get all ASes in the graph
    const auto& ases = graph.getAllASes();
    
    // For each AS, write its routing table
    for (const auto& [asn, as_ptr] : ases) {
        const auto& routing_table = as_ptr->getRoutingTable();
        
        for (const auto& [prefix, announcement] : routing_table) {
            oss << asn << ",";
            oss << prefix << ",";
            oss << formatASPath(announcement.getASPath());
            oss << "\n";
        }
    }
    
    return oss.str();
}

std::string CSVOutput::formatASPath(const std::vector<uint32_t>& path) {
    if (path.empty()) {
        return "";
    }

    std::ostringstream oss;
    oss << "(";
    for (size_t i = 0; i < path.size(); i++) {
        oss << path[i];
        if (i < path.size() - 1) {
            oss << ", ";
        } else if (path.size() == 1) {
            oss << ",";  // Trailing comma for single-element paths
        }
    }
    oss << ")";
    return oss.str();
}