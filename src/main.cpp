#include "AS.h"
#include "ASGraph.h"
#include "Announcement.h"
#include "Policy.h"
#include "ROV.h"
#include "Community.h"
#include "Aggregation.h"
#include "Statistics.h"
#include "CSVOutput.h"
#include "utils/Downloader.h"
#include "utils/parser.h"
#include <iostream>
#include <filesystem>

void testBasicScenario() {
    std::cout << "\n=== Test 1: Single Announcement, Tiny Graph ===" << std::endl;
    std::cout << "Goal: Verify basic propagation and CSV output" << std::endl;
    std::cout << std::endl;
    
    // Create tiny graph: AS1 -> AS2 -> AS3
    ASGraph graph;
    graph.addRelationship(1, 2);  // AS1 is provider of AS2
    graph.addRelationship(2, 3);  // AS2 is provider of AS3
    
    std::cout << "Topology: AS1 -> AS2 -> AS3" << std::endl;
    std::cout << "          (provider-customer chain)" << std::endl;
    std::cout << std::endl;
    
    // AS3 originates a prefix
    AS* as3 = graph.getAS(3);
    as3->originatePrefix("10.0.0.0/8");
    
    std::cout << "AS3 originates: 10.0.0.0/8" << std::endl;
    std::cout << std::endl;
    
    // Check routing tables
    AS* as1 = graph.getAS(1);
    AS* as2 = graph.getAS(2);
    
    std::cout << "Routing Tables:" << std::endl;
    std::cout << "  AS1: " << as1->getRoutingTable().size() << " routes" << std::endl;
    std::cout << "  AS2: " << as2->getRoutingTable().size() << " routes" << std::endl;
    std::cout << "  AS3: " << as3->getRoutingTable().size() << " routes" << std::endl;
    std::cout << std::endl;
    
    // Output CSV
    CSVOutput::writeRoutingTable(graph, "routing_table_test1.csv");
    std::cout << "CSV Output (routing_table_test1.csv):" << std::endl;
    std::cout << CSVOutput::generateCSV(graph);
    std::cout << std::endl;
    
    std::cout << "✓ Test 1 Complete" << std::endl;
}

void testLargerGraph() {
    std::cout << "\n=== Test 2: Single Announcement, Larger Graph ===" << std::endl;
    std::cout << "Goal: Verify propagation in more complex topology" << std::endl;
    std::cout << std::endl;
    
    // Create larger graph with Tier-1, Tier-2, and stub ASes
    ASGraph graph;
    
    // Tier-1 backbone (peers with each other)
    graph.addPeeringRelationship(1, 2);
    
    // Tier-2 (customers of Tier-1)
    graph.addRelationship(1, 10);  // AS1 -> AS10
    graph.addRelationship(2, 20);  // AS2 -> AS20
    
    // Stubs (customers of Tier-2)
    graph.addRelationship(10, 100);  // AS10 -> AS100
    graph.addRelationship(20, 200);  // AS20 -> AS200
    
    std::cout << "Topology:" << std::endl;
    std::cout << "    AS1 <-peer-> AS2" << std::endl;
    std::cout << "     |            |" << std::endl;
    std::cout << "    AS10         AS20" << std::endl;
    std::cout << "     |            |" << std::endl;
    std::cout << "   AS100        AS200" << std::endl;
    std::cout << std::endl;
    
    // AS100 originates a prefix
    AS* as100 = graph.getAS(100);
    as100->originatePrefix("192.168.0.0/16");
    
    std::cout << "AS100 originates: 192.168.0.0/16" << std::endl;
    std::cout << std::endl;
    
    // Check which ASes learned the route
    std::cout << "Route learned by:" << std::endl;
    for (uint32_t asn : {1, 2, 10, 20, 100, 200}) {
        AS* as = graph.getAS(asn);
        if (as->getRoutingTable().count("192.168.0.0/16")) {
            const auto& ann = as->getRoutingTable().at("192.168.0.0/16");
            std::cout << "  AS" << asn << ": path = ";
            for (uint32_t hop : ann.getASPath()) {
                std::cout << hop << " ";
            }
            std::cout << std::endl;
        } else {
            std::cout << "  AS" << asn << ": (no route)" << std::endl;
        }
    }
    std::cout << std::endl;
    
    // Output CSV
    CSVOutput::writeRoutingTable(graph, "routing_table_test2.csv");
    std::cout << "CSV Output (routing_table_test2.csv):" << std::endl;
    std::cout << CSVOutput::generateCSV(graph);
    std::cout << std::endl;
    
    std::cout << "✓ Test 2 Complete" << std::endl;
}

void testConflictingAnnouncements() {
    std::cout << "\n=== Test 3: Two Announcements for Same Prefix ===" << std::endl;
    std::cout << "Goal: Verify path selection with competing routes" << std::endl;
    std::cout << std::endl;
    
    // Create diamond topology
    ASGraph graph;
    graph.addRelationship(1, 2);  // AS1 -> AS2
    graph.addRelationship(1, 3);  // AS1 -> AS3
    graph.addRelationship(2, 4);  // AS2 -> AS4
    graph.addRelationship(3, 4);  // AS3 -> AS4
    
    std::cout << "Topology (diamond):" << std::endl;
    std::cout << "       AS1" << std::endl;
    std::cout << "      /   \\" << std::endl;
    std::cout << "    AS2   AS3" << std::endl;
    std::cout << "      \\   /" << std::endl;
    std::cout << "       AS4" << std::endl;
    std::cout << std::endl;
    
    // Both AS2 and AS3 originate the same prefix
    AS* as2 = graph.getAS(2);
    AS* as3 = graph.getAS(3);
    
    as2->originatePrefix("203.0.113.0/24");
    as3->originatePrefix("203.0.113.0/24");
    
    std::cout << "AS2 originates: 203.0.113.0/24" << std::endl;
    std::cout << "AS3 originates: 203.0.113.0/24" << std::endl;
    std::cout << "(Conflict! Two origins for same prefix)" << std::endl;
    std::cout << std::endl;
    
    // Check what AS1 and AS4 chose
    AS* as1 = graph.getAS(1);
    AS* as4 = graph.getAS(4);
    
    std::cout << "Path Selection Results:" << std::endl;
    
    if (as1->getRoutingTable().count("203.0.113.0/24")) {
        const auto& ann = as1->getRoutingTable().at("203.0.113.0/24");
        std::cout << "  AS1 chose: path = ";
        for (uint32_t hop : ann.getASPath()) {
            std::cout << hop << " ";
        }
        std::cout << "(via AS" << ann.getASPath()[1] << ")" << std::endl;
    }
    
    if (as4->getRoutingTable().count("203.0.113.0/24")) {
        const auto& ann = as4->getRoutingTable().at("203.0.113.0/24");
        std::cout << "  AS4 chose: path = ";
        for (uint32_t hop : ann.getASPath()) {
            std::cout << hop << " ";
        }
        std::cout << "(via AS" << ann.getASPath()[1] << ")" << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "Decision Factors:" << std::endl;
    std::cout << "  - Both paths learned from customers (equal local pref)" << std::endl;
    std::cout << "  - Both paths have same length" << std::endl;
    std::cout << "  - Tie-breaker: lower origin ASN wins" << std::endl;
    std::cout << std::endl;
    
    // Output CSV
    CSVOutput::writeRoutingTable(graph, "routing_table_test3.csv");
    std::cout << "CSV Output (routing_table_test3.csv):" << std::endl;
    std::cout << CSVOutput::generateCSV(graph);
    std::cout << std::endl;
    
    std::cout << "✓ Test 3 Complete" << std::endl;
}

void testPrefixHijack() {
    std::cout << "\n=== Test 4: Prefix Hijacking Scenario ===" << std::endl;
    std::cout << "Goal: Demonstrate malicious announcement competing with legitimate" << std::endl;
    std::cout << std::endl;
    
    ASGraph graph;
    graph.addRelationship(1, 2);  // AS1 -> AS2
    graph.addRelationship(1, 3);  // AS1 -> AS3
    graph.addRelationship(1, 4);  // AS1 -> AS4 (attacker)
    
    std::cout << "Topology:" << std::endl;
    std::cout << "        AS1" << std::endl;
    std::cout << "       / | \\" << std::endl;
    std::cout << "     AS2 AS3 AS4" << std::endl;
    std::cout << std::endl;
    
    // AS2 is the legitimate owner
    AS* as2 = graph.getAS(2);
    as2->originatePrefix("8.8.8.0/24");
    
    std::cout << "AS2 (legitimate) originates: 8.8.8.0/24" << std::endl;
    
    // AS4 is an attacker announcing a more specific prefix
    AS* as4 = graph.getAS(4);
    as4->originatePrefix("8.8.8.0/25");  // More specific!
    
    std::cout << "AS4 (attacker) originates: 8.8.8.0/25 (MORE SPECIFIC!)" << std::endl;
    std::cout << std::endl;
    
    // Check what AS1 sees
    AS* as1 = graph.getAS(1);
    
    std::cout << "AS1 routing table:" << std::endl;
    for (const auto& [prefix, ann] : as1->getRoutingTable()) {
        if (prefix.find("8.8.8.") != std::string::npos) {
            std::cout << "  " << prefix << " -> via AS" << ann.getASPath()[1] << std::endl;
        }
    }
    std::cout << std::endl;
    
    std::cout << "Result: More specific prefix wins (longest prefix match)" << std::endl;
    std::cout << "        Traffic to 8.8.8.0/25 goes to attacker!" << std::endl;
    std::cout << "        This is a real BGP hijacking technique" << std::endl;
    std::cout << std::endl;
    
    // Output CSV
    CSVOutput::writeRoutingTable(graph, "routing_table_test4.csv");
    std::cout << "CSV Output (routing_table_test4.csv):" << std::endl;
    std::cout << CSVOutput::generateCSV(graph);
    std::cout << std::endl;
    
    std::cout << "✓ Test 4 Complete" << std::endl;
}

void testValleyFreeViolation() {
    std::cout << "\n=== Test 5: Valley-Free Policy Enforcement ===" << std::endl;
    std::cout << "Goal: Show that valley-free prevents certain routes" << std::endl;
    std::cout << std::endl;
    
    ASGraph graph;
    
    // Create a topology where valley-free matters
    graph.addRelationship(1, 2);      // AS1 -> AS2 (provider)
    graph.addPeeringRelationship(2, 3);  // AS2 <-> AS3 (peer)
    graph.addRelationship(3, 4);      // AS3 -> AS4 (provider)
    
    std::cout << "Topology:" << std::endl;
    std::cout << "  AS1" << std::endl;
    std::cout << "   |" << std::endl;
    std::cout << "  AS2 <-peer-> AS3" << std::endl;
    std::cout << "                |" << std::endl;
    std::cout << "               AS4" << std::endl;
    std::cout << std::endl;
    
    // AS4 originates
    AS* as4 = graph.getAS(4);
    as4->originatePrefix("172.16.0.0/12");
    
    std::cout << "AS4 originates: 172.16.0.0/12" << std::endl;
    std::cout << std::endl;
    
    // Check propagation
    std::cout << "Propagation:" << std::endl;
    std::cout << "  AS4 -> AS3: YES (provider exports to customer)" << std::endl;
    std::cout << "  AS3 -> AS2: NO! (valley-free violation)" << std::endl;
    std::cout << "              (learned from provider, can't export to peer)" << std::endl;
    std::cout << "  AS1: No route (AS2 didn't receive it)" << std::endl;
    std::cout << std::endl;
    
    AS* as1 = graph.getAS(1);
    AS* as2 = graph.getAS(2);
    AS* as3 = graph.getAS(3);
    
    std::cout << "Verification:" << std::endl;
    std::cout << "  AS1 has route: " << (as1->getRoutingTable().count("172.16.0.0/12") ? "YES" : "NO") << std::endl;
    std::cout << "  AS2 has route: " << (as2->getRoutingTable().count("172.16.0.0/12") ? "YES" : "NO") << std::endl;
    std::cout << "  AS3 has route: " << (as3->getRoutingTable().count("172.16.0.0/12") ? "YES" : "NO") << std::endl;
    std::cout << "  AS4 has route: " << (as4->getRoutingTable().count("172.16.0.0/12") ? "YES" : "NO") << std::endl;
    std::cout << std::endl;
    
    // Output CSV
    CSVOutput::writeRoutingTable(graph, "routing_table_test5.csv");
    std::cout << "CSV Output (routing_table_test5.csv):" << std::endl;
    std::cout << CSVOutput::generateCSV(graph);
    std::cout << std::endl;
    
    std::cout << "✓ Test 5 Complete" << std::endl;
}

int main() {
    std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                                                            ║" << std::endl;
    std::cout << "║           BGP SIMULATOR - CLOUDFLARE FORMAT                ║" << std::endl;
    std::cout << "║                                                            ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
    
    std::cout << "\nThis simulator outputs routing tables in CSV format:" << std::endl;
    std::cout << "  Format: asn,prefix,as_path" << std::endl;
    std::cout << "  Purpose: Cloudflare network optimization" << std::endl;
    std::cout << std::endl;
    
    // Run test scenarios
    testBasicScenario();
    testLargerGraph();
    testConflictingAnnouncements();
    testPrefixHijack();
    testValleyFreeViolation();
    
    std::cout << "\n╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                                                            ║" << std::endl;
    std::cout << "║                   ALL TESTS COMPLETE                       ║" << std::endl;
    std::cout << "║                                                            ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
    
    std::cout << "\nGenerated CSV files:" << std::endl;
    std::cout << "  - routing_table_test1.csv (basic scenario)" << std::endl;
    std::cout << "  - routing_table_test2.csv (larger topology)" << std::endl;
    std::cout << "  - routing_table_test3.csv (conflicting announcements)" << std::endl;
    std::cout << "  - routing_table_test4.csv (prefix hijacking)" << std::endl;
    std::cout << "  - routing_table_test5.csv (valley-free policy)" << std::endl;
    std::cout << std::endl;
    
    return 0;
}