#pragma once

#include "ASGraph.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>

/**
 * Parser for CAIDA AS relationship data
 * Format: <provider-as>|<customer-as>|<relationship-type>
 * -1 = provider-to-customer
 *  0 = peer-to-peer
 */
class CAIDAParser {
public:
    // Parse CAIDA file into ASGraph
    static bool parseFile(const std::string& filename, ASGraph& graph) {
        // Handle .bz2 files - decompress first
        std::string file_to_parse = filename;
        bool is_compressed = (filename.size() > 4 && 
                             filename.substr(filename.size() - 4) == ".bz2");
        
        if (is_compressed) {
            std::string decompressed = filename.substr(0, filename.size() - 4);
            std::string cmd = "bunzip2 -k -f " + filename;
            int result = system(cmd.c_str());
            if (result != 0) {
                std::cerr << "Failed to decompress file" << std::endl;
                return false;
            }
            file_to_parse = decompressed;
        }
        
        // Parse the file
        std::ifstream file(file_to_parse);
        if (!file.is_open()) {
            std::cerr << "Cannot open file: " << file_to_parse << std::endl;
            return false;
        }
        
        std::string line;
        int line_count = 0;
        int rel_count = 0;
        
        while (std::getline(file, line)) {
            line_count++;
            if (parseLine(line, graph)) {
                rel_count++;
            }
        }
        
        file.close();
        
        std::cout << "Parsed " << line_count << " lines, " 
                  << rel_count << " relationships" << std::endl;
        std::cout << "Graph has " << graph.size() << " ASes" << std::endl;
        
        return true;
    }
    
private:
    // Parse a single line
    static bool parseLine(const std::string& line, ASGraph& graph) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            return false;
        }
        
        // Parse format: asn1|asn2|type
        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(ss, token, '|')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() < 3) {
            return false;
        }
        
        try {
            uint32_t asn1 = std::stoul(tokens[0]);
            uint32_t asn2 = std::stoul(tokens[1]);
            int rel_type = std::stoi(tokens[2]);
            
            if (rel_type == -1) {
                // Provider-customer: asn1 is provider, asn2 is customer
                graph.addRelationship(asn1, asn2);
            } else if (rel_type == 0) {
                // Peer-to-peer
                graph.addPeeringRelationship(asn1, asn2);
            }
            
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }
};