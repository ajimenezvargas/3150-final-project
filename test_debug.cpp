#include "include/AS.h"
#include "include/ASGraph.h"
#include "include/Announcement.h"
#include "include/CSVInput.h"
#include "include/CSVOutput.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
    // Load CAIDA data
    std::ifstream caida_file("test_preset.txt");
    std::stringstream caida_buffer;
    caida_buffer << caida_file.rdbuf();
    std::string caida_data = caida_buffer.str();

    ASGraph graph;
    std::istringstream stream(caida_data);
    std::string line;

    std::cout << "Parsing CAIDA data..." << std::endl;
    std::cout << "Total buffer size: " << caida_data.size() << " bytes" << std::endl;
    int line_count = 0;
    while (std::getline(stream, line)) {
        line_count++;
        std::cout << "Raw line " << line_count << " (len=" << line.length() << "): [" << line << "]" << std::endl;
        if (line.empty() || line[0] == '#') {
            std::cout << "  Skipping (empty or comment)" << std::endl;
            continue;
        }

        std::cout << "Line " << line_count << ": " << line << std::endl;

        std::istringstream iss(line);
        uint32_t asn1, asn2, relationship;
        char pipe;

        if (iss >> asn1 >> pipe >> asn2 >> pipe >> relationship) {
            std::cout << "  Parsed: " << asn1 << " | " << asn2 << " | " << relationship << std::endl;
            graph.getOrCreateAS(asn1);
            graph.getOrCreateAS(asn2);

            if (relationship == -1) {
                graph.addRelationship(asn1, asn2);
            } else if (relationship == 0) {
                graph.addPeeringRelationship(asn1, asn2);
            } else if (relationship == 1) {
                graph.addRelationship(asn2, asn1);
            }
        } else {
            std::cout << "  FAILED TO PARSE" << std::endl;
        }
    }

    std::cout << "\nLoaded " << graph.getAllASes().size() << " ASes" << std::endl;

    // Load announcements
    std::ifstream anns_file("test_preset_anns.csv");
    std::stringstream anns_buffer;
    anns_buffer << anns_file.rdbuf();
    std::string anns_data = anns_buffer.str();

    std::cout << "\nParsing announcements..." << std::endl;
    std::istringstream ann_stream(anns_data);
    bool first = true;
    while (std::getline(ann_stream, line)) {
        if (line.empty()) continue;

        if (first) {
            first = false;
            if (line.find("asn") != std::string::npos || line.find("origin") != std::string::npos) {
                std::cout << "  Skipping header: " << line << std::endl;
                continue;
            }
        }

        std::cout << "Ann Line: " << line << std::endl;

        std::istringstream iss(line);
        std::string asn_str, prefix, rov_str;

        if (std::getline(iss, asn_str, ',') &&
            std::getline(iss, prefix, ',') &&
            std::getline(iss, rov_str, ',')) {

            std::cout << "  Parsed: asn=" << asn_str << ", prefix=" << prefix << ", rov=" << rov_str << std::endl;

            try {
                uint32_t asn = std::stoul(asn_str);
                AS* as = graph.getAS(asn);
                if (as) {
                    std::cout << "    Found AS" << asn << ", originating prefix..." << std::endl;
                    as->originatePrefix(prefix);
                } else {
                    std::cout << "    AS" << asn << " not found in graph!" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "  ERROR: " << e.what() << std::endl;
            }
        }
    }

    // Check routing tables
    std::cout << "\nRouting Tables:" << std::endl;
    for (const auto& [asn, as_ptr] : graph.getAllASes()) {
        std::cout << "AS" << asn << ": " << as_ptr->getRoutingTable().size() << " routes" << std::endl;
        for (const auto& [prefix, ann] : as_ptr->getRoutingTable()) {
            std::cout << "  - " << prefix << std::endl;
        }
    }

    return 0;
}
