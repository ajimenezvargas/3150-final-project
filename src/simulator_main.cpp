#include "AS.h"
#include "ASGraph.h"
#include "Announcement.h"
#include "Policy.h"
#include "ROV.h"
#include "CSVOutput.h"
#include "CSVInput.h"
#include "utils/Downloader.h"
#include "utils/parser.h"
#include <iostream>
#include <string>
#include <unordered_set>

void printUsage(const char* program_name) {
    std::cout << "BGP Simulator - Cloudflare Network Optimization Tool\n";
    std::cout << "Usage: " << program_name << " [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --relationships <path>   Path to CAIDA AS relationships file\n";
    std::cout << "  --announcements <path>   Path to announcements CSV file\n";
    std::cout << "  --rov-asns <path>        Path to ROV ASNs CSV file\n";
    std::cout << "  --output <path>          Path to output CSV file (default: ribs.csv)\n";
    std::cout << "  --help                   Show this help message\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << program_name << " --relationships relationships.txt \\\n";
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
        } else if (arg == "--relationships" && i + 1 < argc) {
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
        std::cerr << "Error: --relationships is required\n";
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

    if (graph.hasCycle()) {
        std::cerr << "Error: Cycle detected in CAIDA relationships (provider/customer loop)\n";
        auto cycle = graph.findCycle();
        if (!cycle.empty()) {
            std::cerr << "  Cycle path: ";
            for (size_t i = 0; i < cycle.size(); ++i) {
                std::cerr << cycle[i];
                if (i + 1 < cycle.size()) {
                    std::cerr << " -> ";
                }
            }
            std::cerr << "\n";
        }
        return 2;
    }

    // Compute propagation ranks for hierarchical propagation
    graph.computePropagationRanks();
    std::cout << "  Computed " << graph.getPropagationRanks().size() << " propagation ranks\n";
    std::cout << "  ✓ AS Graph constructed\n\n";
    
    // Step 2: Load ROV ASNs (optional)
    std::cout << "[2/5] Loading ROV ASNs...\n";
    std::unordered_set<uint32_t> rov_asns_set;

    // Set ROV validator for ALL ASes so they can check ROV state
    for (const auto& [asn, as_ptr] : graph.getAllASes()) {
        as_ptr->setROVValidator(&graph.getROVValidator());
    }

    if (!rov_asns_file.empty()) {
        auto rov_asns = CSVInput::parseROVASNs(rov_asns_file);
        rov_asns_set.insert(rov_asns.begin(), rov_asns.end());
        std::cout << "  Loaded " << rov_asns_set.size() << " ROV ASNs\n";

        // Enable ROV filtering ONLY on ROV ASes - drop INVALID routes
        for (uint32_t asn : rov_asns_set) {
            AS* as = graph.getAS(asn);
            if (as) {
                as->setDropInvalid(true);
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
    std::cout << "[4/5] Seeding Announcements and Simulating Propagation...\n";
    int seeded = 0;
    int skipped = 0;

    // First, create ROAs for valid announcements
    for (const auto& input_ann : announcements) {
        if (!input_ann.rov_invalid) {
            // Add ROA for valid announcement
            graph.getROVValidator().addROA(input_ann.prefix, input_ann.asn);
        }
    }

    for (const auto& input_ann : announcements) {
        AS* origin_as = graph.getAS(input_ann.asn);

        if (!origin_as) {
            skipped++;
            continue;
        }

        // Originate all announcements (including invalid ones)
        // ROV-enabled ASes will drop invalid routes during propagation
        origin_as->originatePrefix(input_ann.prefix);
        seeded++;
    }

    std::cout << "  Seeded: " << seeded << " announcements\n";
    if (skipped > 0) {
        std::cout << "  Skipped: " << skipped << " (ASN not in graph)\n";
    }

    // Run BGPy-style hierarchical propagation until convergence
    std::cout << "  Running hierarchical propagation...\n";
    const auto& ranks = graph.getPropagationRanks();
    int round = 0;
    bool changed = true;

    while (changed) {
        round++;
        changed = false;

        // Phase 1: Propagate to providers (bottom-up through ranks)
        for (size_t i = 0; i < ranks.size(); i++) {
            // Process incoming from lower ranks (customers)
            if (i > 0) {
                for (AS* as_ptr : ranks[i]) {
                    if (as_ptr->processIncomingQueue()) {
                        changed = true;
                    }
                }
            }
            // Propagate ONLY to providers (higher ranks)
            for (AS* as_ptr : ranks[i]) {
                as_ptr->propagateToProviders();
            }
        }

        // Phase 2: Propagate to peers (all ranks)
        for (const auto& rank : ranks) {
            for (AS* as_ptr : rank) {
                as_ptr->propagateToPeers();
            }
        }
        for (const auto& rank : ranks) {
            for (AS* as_ptr : rank) {
                if (as_ptr->processIncomingQueue()) {
                    changed = true;
                }
            }
        }

        // Phase 3: Propagate to customers (top-down through ranks)
        for (int i = ranks.size() - 1; i >= 0; i--) {
            // Process incoming from higher ranks (providers)
            if (i < (int)ranks.size() - 1) {
                for (AS* as_ptr : ranks[i]) {
                    if (as_ptr->processIncomingQueue()) {
                        changed = true;
                    }
                }
            }
            // Propagate ONLY to customers (lower ranks)
            for (AS* as_ptr : ranks[i]) {
                as_ptr->propagateToCustomers();
            }
        }
    }

    std::cout << "  Converged after " << round << " rounds\n";
    std::cout << "  ✓ Propagation complete\n\n";
    
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
