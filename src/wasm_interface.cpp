#include "AS.h"
#include "ASGraph.h"
#include "Announcement.h"
#include "CSVInput.h"
#include "CSVOutput.h"
#include "utils/parser.h"
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace emscripten;

class BGPSimulatorWASM {
private:
    ASGraph graph;
    std::vector<InputAnnouncement> announcements;
    std::unordered_set<uint32_t> rov_asns_set;

public:
    BGPSimulatorWASM() = default;

    // Load CAIDA data from string
    bool loadCAIDAData(const std::string& caida_data) {
        std::istringstream stream(caida_data);
        std::string line;
        int line_count = 0;

        while (std::getline(stream, line)) {
            line_count++;
            if (line_count % 100000 == 0) {
                std::cerr << "Parsed " << line_count << " lines..." << std::endl;
            }

            // Skip comments
            if (line.empty() || line[0] == '#') continue;

            // Parse CAIDA format: asn1 | asn2 | relationship
            std::istringstream iss(line);
            uint32_t asn1, asn2, relationship;
            char pipe;

            if (iss >> asn1 >> pipe >> asn2 >> pipe >> relationship) {
                // Ensure nodes exist
                graph.getOrCreateAS(asn1);
                graph.getOrCreateAS(asn2);

                // Add relationship
                // -1 = customer, 0 = peer, 1 = provider
                if (relationship == -1) {
                    // asn1 is provider, asn2 is customer
                    graph.addRelationship(asn1, asn2);
                } else if (relationship == 0) {
                    // Peer relationship
                    graph.addPeeringRelationship(asn1, asn2);
                } else if (relationship == 1) {
                    // asn2 is provider, asn1 is customer
                    graph.addRelationship(asn2, asn1);
                }
            }
        }

        std::cerr << "Loaded " << graph.getAllASes().size() << " ASes" << std::endl;
        return true;
    }

    // Load announcements from CSV string
    bool loadAnnouncements(const std::string& csv_data) {
        std::istringstream stream(csv_data);
        std::string line;
        bool first_line = true;

        while (std::getline(stream, line)) {
            // Skip empty lines
            if (line.empty()) continue;

            // Skip header
            if (first_line) {
                first_line = false;
                std::string lower = line;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("asn") != std::string::npos) {
                    continue;
                }
            }

            // Parse: asn,prefix,rov_invalid
            std::istringstream iss(line);
            std::string asn_str, prefix, rov_str;

            if (std::getline(iss, asn_str, ',') &&
                std::getline(iss, prefix, ',') &&
                std::getline(iss, rov_str, ',')) {

                try {
                    uint32_t asn = std::stoul(asn_str);
                    // Trim whitespace
                    prefix.erase(0, prefix.find_first_not_of(" \t\r\n"));
                    prefix.erase(prefix.find_last_not_of(" \t\r\n") + 1);

                    bool rov_invalid = false;
                    std::string trimmed_rov = rov_str;
                    trimmed_rov.erase(0, trimmed_rov.find_first_not_of(" \t\r\n"));
                    trimmed_rov.erase(trimmed_rov.find_last_not_of(" \t\r\n") + 1);
                    std::transform(trimmed_rov.begin(), trimmed_rov.end(), trimmed_rov.begin(), ::tolower);

                    if (trimmed_rov == "true" || trimmed_rov == "1" || trimmed_rov == "yes") {
                        rov_invalid = true;
                    }

                    announcements.emplace_back(asn, prefix, rov_invalid);
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing announcement: " << e.what() << std::endl;
                }
            }
        }

        std::cerr << "Loaded " << announcements.size() << " announcements" << std::endl;
        return !announcements.empty();
    }

    // Load ROV ASNs from CSV string
    bool loadROVASNs(const std::string& csv_data) {
        std::istringstream stream(csv_data);
        std::string line;
        bool first_line = true;

        while (std::getline(stream, line)) {
            if (line.empty()) continue;

            if (first_line) {
                first_line = false;
                std::string lower = line;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find("asn") != std::string::npos) {
                    continue;
                }
            }

            try {
                uint32_t asn = std::stoul(line);
                rov_asns_set.insert(asn);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing ROV ASN: " << e.what() << std::endl;
            }
        }

        std::cerr << "Loaded " << rov_asns_set.size() << " ROV ASNs" << std::endl;
        return true;
    }

    // Run the simulation and return results
    std::string runSimulation() {
        // Seed announcements
        for (const auto& input_ann : announcements) {
            AS* origin_as = graph.getAS(input_ann.asn);
            if (!origin_as) continue;

            origin_as->originatePrefix(input_ann.prefix);

            if (input_ann.rov_invalid) {
                auto& routing_table = const_cast<std::unordered_map<std::string, Announcement>&>(
                    origin_as->getRoutingTable()
                );

                if (routing_table.count(input_ann.prefix)) {
                    routing_table[input_ann.prefix].setROVState(ROVState::INVALID);
                }
            }
        }

        // Build result JSON
        std::ostringstream result;
        result << "{";
        result << "\"status\": \"success\",";
        result << "\"total_routes\": " << getTotalRouteCount() << ",";
        result << "\"total_ases\": " << graph.getAllASes().size() << ",";
        result << "\"announcements_seeded\": " << announcements.size();
        result << "}";

        return result.str();
    }

    // Get routing information for a specific ASN
    std::string getRoutingInfo(uint32_t asn) {
        AS* as = graph.getAS(asn);
        if (!as) {
            return "{\"error\": \"ASN not found\"}";
        }

        std::ostringstream result;
        result << "{";
        result << "\"asn\": " << asn << ",";
        result << "\"routes\": [";

        const auto& routing_table = as->getRoutingTable();
        bool first = true;

        for (const auto& [prefix, announcement] : routing_table) {
            if (!first) result << ",";
            result << "{\"prefix\": \"" << prefix << "\"}";
            first = false;
        }

        result << "],";
        result << "\"route_count\": " << routing_table.size();
        result << "}";

        return result.str();
    }

    // Export all routing tables as CSV
    std::string exportRoutingTables() {
        std::ostringstream csv;
        csv << "ASN,Prefix,Path\n";

        for (const auto& [asn, as] : graph.getAllASes()) {
            const auto& routing_table = as->getRoutingTable();
            for (const auto& [prefix, announcement] : routing_table) {
                csv << asn << "," << prefix << "\n";
            }
        }

        return csv.str();
    }

    int getTotalRouteCount() {
        int total = 0;
        for (const auto& [asn, as] : graph.getAllASes()) {
            total += as->getRoutingTable().size();
        }
        return total;
    }

    void reset() {
        graph = ASGraph();
        announcements.clear();
        rov_asns_set.clear();
    }
};

// Global simulator instance
BGPSimulatorWASM simulator;

// Exposed C++ functions for JavaScript
EMSCRIPTEN_BINDINGS(bgp_simulator) {
    class_<BGPSimulatorWASM>("BGPSimulator")
        .constructor<>()
        .function("loadCAIDAData", &BGPSimulatorWASM::loadCAIDAData)
        .function("loadAnnouncements", &BGPSimulatorWASM::loadAnnouncements)
        .function("loadROVASNs", &BGPSimulatorWASM::loadROVASNs)
        .function("runSimulation", &BGPSimulatorWASM::runSimulation)
        .function("getRoutingInfo", &BGPSimulatorWASM::getRoutingInfo)
        .function("exportRoutingTables", &BGPSimulatorWASM::exportRoutingTables)
        .function("getTotalRouteCount", &BGPSimulatorWASM::getTotalRouteCount)
        .function("reset", &BGPSimulatorWASM::reset);
}
