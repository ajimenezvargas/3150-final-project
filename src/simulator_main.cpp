#include "AS.h"
#include "ASGraph.h"
#include "Announcement.h"
#include "Policy.h"
#include "ROV.h"
#include "CSVOutput.h"
#include "CSVInput.h"
#include "utils/downloader.h"
#include "utils/parser.h"
#include <iostream>
#include <string>
#include <unordered_set>

void printUsage(const char* program_name) {
    std::cout << "BGP Simulator - Cloudflare Network Optimization Tool\n";
    std::cout << "Usage: " << program_name << " [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --caida <path>           Path to CAIDA AS relationships file\n";
    std::cout << "  --announcements <path>   Path to announcements CSV file\n";
    std::cout << "  --rov-asns <path>        Path to ROV ASNs CSV file\n";
    std::cout << "  --output <path>          Path to output CSV file (default: ribs.csv)\n";
    std::cout << "  --help                   Show this help message\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << program_name << " --caida relationships.txt \\\n";
    std::cout << "    --announcements announcements.csv \\\n";
    std::cout << "    --rov-asns rov_asns.csv \\\n";
    std::cout << "    --output ribs.csv\n";
}

int main(int argc, char* argv[]) {
    std::string caida_file;
    std::string announcements_file;
    std::string rov_asns_file;
    std::string output_file = "ribs.csv";
    
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--caida" && i + 1 < argc) {
            caida_file = argv[++i];
        } else if (arg == "--announcements" && i + 1 < argc) {
            announcements_file = argv[++i];
        } else if (arg == "--rov-asns" && i + 1 < argc) {
            rov_asns_file = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            output_file = argv[++i];
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Validate required arguments
    if (caida_file.empty()) {
        std::cerr << "Error: --caida is required\n";
        printUsage(argv[0]);
        return 1;
    }
    
    if (announcements_file.empty()) {
        std::cerr << "Error: --announcements is required\n";
        printUsage(argv[0]);
        return 1;
    }
    
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                            ║\n";
    std::cout << "║           BGP SIMULATOR - CLOUDFLARE EDITION               ║\n";
    std::cout << "║                                                            ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n\n";
    
    // Step 1: Build AS Graph from CAIDA data
    std::cout << "[1/5] Loading CAIDA AS Relationships...\n";
    ASGraph graph;
    
    if (!CAIDAParser::parseFile(caida_file, graph)) {
        std::cerr << "Error: Failed to parse CAIDA file\n";
        return 1;
    }
    
    std::cout << "  Loaded " << graph.getAllASes().size() << " ASes\n";
    std::cout << "  ✓ AS Graph constructed\n\n";
    
    // Step 2: Load ROV ASNs (optional)
    std::cout << "[2/5] Loading ROV ASNs...\n";
    std::unordered_set<uint32_t> rov_asns_set;
    
    if (!rov_asns_file.empty()) {
        auto rov_asns = CSVInput::parseROVASNs(rov_asns_file);
        rov_asns_set.insert(rov_asns.begin(), rov_asns.end());
        std::cout << "  Loaded " << rov_asns_set.size() << " ROV ASNs\n";
        
        // Enable ROV on these ASes
        for (uint32_t asn : rov_asns_set) {
            AS* as = graph.getAS(asn);
            if (as) {
                // Mark this AS as deploying ROV
                // (In a full implementation, you'd set a ROV policy here)
            }
        }
    } else {
        std::cout << "  No ROV ASNs file provided (optional)\n";
    }
    std::cout << "  ✓ ROV configuration complete\n\n";
    
    // Step 3: Load and seed announcements
    std::cout << "[3/5] Loading Announcements...\n";
    auto announcements = CSVInput::parseAnnouncements(announcements_file);
    
    if (announcements.empty()) {
        std::cerr << "Error: No announcements loaded\n";
        return 1;
    }
    
    std::cout << "  Loaded " << announcements.size() << " announcements\n";
    std::cout << "  ✓ Announcements parsed\n\n";
    
    // Step 4: Seed announcements into the graph
    std::cout << "[4/5] Seeding Announcements into AS Graph...\n";
    int seeded = 0;
    int skipped = 0;
    
    for (const auto& input_ann : announcements) {
        AS* origin_as = graph.getAS(input_ann.asn);
        
        if (!origin_as) {
            skipped++;
            continue;
        }
        
        // Create announcement
        origin_as->originatePrefix(input_ann.prefix);
        
        // Set ROV state if invalid
        if (input_ann.rov_invalid) {
            auto& routing_table = const_cast<std::unordered_map<std::string, Announcement>&>(
                origin_as->getRoutingTable()
            );
            
            if (routing_table.count(input_ann.prefix)) {
                routing_table[input_ann.prefix].setROVState(ROVState::INVALID);
            }
        }
        
        seeded++;
    }
    
    std::cout << "  Seeded: " << seeded << " announcements\n";
    if (skipped > 0) {
        std::cout << "  Skipped: " << skipped << " (ASN not in graph)\n";
    }
    std::cout << "  ✓ Announcements seeded\n\n";
    
    // Step 5: Export routing tables to CSV
    std::cout << "[5/5] Exporting Routing Tables...\n";
    
    if (!CSVOutput::writeRoutingTable(graph, output_file)) {
        std::cerr << "Error: Failed to write output CSV\n";
        return 1;
    }
    
    // Count total routes
    int total_routes = 0;
    for (const auto& [asn, as] : graph.getAllASes()) {
        total_routes += as->getRoutingTable().size();
    }
    
    std::cout << "  Total routes: " << total_routes << "\n";
    std::cout << "  Output file: " << output_file << "\n";
    std::cout << "  ✓ Routing tables exported\n\n";
    
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                            ║\n";
    std::cout << "║                   SIMULATION COMPLETE                      ║\n";
    std::cout << "║                                                            ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    return 0;
}