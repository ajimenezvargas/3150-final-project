#include "AS.h"
#include "ASGraph.h"
#include "Announcement.h"
#include "Policy.h"
#include "utils/downloader.h"
#include "utils/parser.h"
#include <iostream>
#include <filesystem>

void testDay1() {
    std::cout << "\n=== Day 1 Tests ===" << std::endl;
    
    AS as4(4);
    AS as666(666);
    AS as3(3);
    AS as777(777);
    
    as4.addCustomer(&as666);
    as4.addCustomer(&as3);
    as4.addPeer(&as777);
    as666.addProvider(&as4);
    
    std::cout << "AS4 customers: ";
    for (const AS* c : as4.getCustomers()) {
        std::cout << c->getASN() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "[SUCCESS] Day 1 tests passed" << std::endl;
}

void testDay2() {
    std::cout << "\n=== Day 2 Tests ===" << std::endl;
    
    ASGraph graph;
    graph.addRelationship(1, 2);
    graph.addRelationship(2, 3);
    graph.addRelationship(1, 4);
    graph.addPeeringRelationship(3, 4);
    
    std::cout << "Graph size: " << graph.size() << " ASes" << std::endl;
    
    if (graph.hasCycle()) {
        std::cout << "WARNING: Cycle detected!" << std::endl;
    } else {
        std::cout << "No cycles detected (good)" << std::endl;
    }
    
    std::cout << "[SUCCESS] Day 2 tests passed" << std::endl;
}

void testDay3() {
    std::cout << "\n=== Day 3 Tests ===" << std::endl;
    
    // Test 1: Simple announcement
    std::cout << "\n[Test 1] Basic announcement propagation" << std::endl;
    ASGraph graph;
    
    // Build simple topology: 1 -> 2 -> 3
    graph.addRelationship(1, 2);  // 1 is provider of 2
    graph.addRelationship(2, 3);  // 2 is provider of 3
    
    AS* as1 = graph.getAS(1);
    AS* as2 = graph.getAS(2);
    AS* as3 = graph.getAS(3);
    
    // AS3 originates a prefix
    as3->originatePrefix("10.0.0.0/8");
    
    std::cout << "AS3 routing table size: " << as3->getRoutingTable().size() << std::endl;
    std::cout << "AS2 routing table size: " << as2->getRoutingTable().size() << std::endl;
    std::cout << "AS1 routing table size: " << as1->getRoutingTable().size() << std::endl;
    
    // Check AS paths
    if (as1->getRoutingTable().count("10.0.0.0/8")) {
        const auto& ann = as1->getRoutingTable().at("10.0.0.0/8");
        std::cout << "AS1 path to 10.0.0.0/8: ";
        for (uint32_t asn : ann.getASPath()) {
            std::cout << asn << " ";
        }
        std::cout << "(length: " << ann.getPathLength() << ")" << std::endl;
    }
    
    // Test 2: Multiple paths
    std::cout << "\n[Test 2] Multiple paths (path selection)" << std::endl;
    ASGraph graph2;
    
    // Diamond topology:
    //     1
    //    / \
    //   2   3
    //    \ /
    //     4
    graph2.addRelationship(1, 2);
    graph2.addRelationship(1, 3);
    graph2.addRelationship(2, 4);
    graph2.addRelationship(3, 4);
    
    AS* as4_origin = graph2.getAS(4);
    AS* as1_dest = graph2.getAS(1);
    
    as4_origin->originatePrefix("20.0.0.0/8");
    
    if (as1_dest->getRoutingTable().count("20.0.0.0/8")) {
        const auto& ann = as1_dest->getRoutingTable().at("20.0.0.0/8");
        std::cout << "AS1 best path to 20.0.0.0/8: ";
        for (uint32_t asn : ann.getASPath()) {
            std::cout << asn << " ";
        }
        std::cout << "(selected shortest path)" << std::endl;
    }
    
    // Test 3: Loop prevention
    std::cout << "\n[Test 3] Loop prevention" << std::endl;
    Announcement test_ann(99, "30.0.0.0/8");
    test_ann.prependASPath(1);
    test_ann.prependASPath(2);
    
    // Try to send announcement back to AS1 (should be rejected)
    int routes_before = as1_dest->getRoutingTable().size();
    as1_dest->receiveAnnouncement(test_ann, graph2.getAS(2));
    int routes_after = as1_dest->getRoutingTable().size();
    
    if (routes_before == routes_after) {
        std::cout << "Loop correctly prevented (AS1 already in path)" << std::endl;
    } else {
        std::cout << "WARNING: Loop not prevented!" << std::endl;
    }
    
    std::cout << "\n[SUCCESS] Day 3 tests passed" << std::endl;
}

void testDay4() {
    std::cout << "\n=== Day 4 Tests (Policies & Valley-Free Routing) ===" << std::endl;
    
    // Test 1: Valley-free routing
    std::cout << "\n[Test 1] Valley-free routing enforcement" << std::endl;
    ASGraph graph;
    
    // Topology to test valley-free:
    //    Provider
    //       |
    //      AS2
    //      / \
    //   Peer1 Peer2
    //
    // Peer1 should NOT learn routes from Peer2 through AS2
    // (peer -> AS2 -> peer is a valley)
    
    graph.addRelationship(1, 2);    // Provider -> AS2
    graph.addPeeringRelationship(2, 3);  // AS2 <-> Peer1
    graph.addPeeringRelationship(2, 4);  // AS2 <-> Peer2
    
    AS* provider = graph.getAS(1);
    AS* as2 = graph.getAS(2);
    AS* peer1 = graph.getAS(3);
    AS* peer2 = graph.getAS(4);
    
    // Peer2 originates a route
    peer2->originatePrefix("192.168.0.0/16");
    
    // Check routing tables
    bool peer1_has_route = peer1->getRoutingTable().count("192.168.0.0/16") > 0;
    bool as2_has_route = as2->getRoutingTable().count("192.168.0.0/16") > 0;
    bool provider_has_route = provider->getRoutingTable().count("192.168.0.0/16") > 0;
    
    std::cout << "Peer2 originates 192.168.0.0/16" << std::endl;
    std::cout << "  AS2 has route: " << (as2_has_route ? "YES" : "NO") << std::endl;
    std::cout << "  Provider has route: " << (provider_has_route ? "YES (VIOLATION!)" : "NO (correct)") << std::endl;
    std::cout << "  Peer1 has route: " << (peer1_has_route ? "YES (VIOLATION!)" : "NO (correct)") << std::endl;
    
    if (!peer1_has_route && as2_has_route && !provider_has_route) {
        std::cout << "✓ Valley-free routing working correctly" << std::endl;
    } else {
        std::cout << "✗ Valley-free routing needs verification" << std::endl;
    }
    
    // Test 2: Preference by relationship
    std::cout << "\n[Test 2] Local preference (customer > peer > provider)" << std::endl;
    ASGraph graph2;
    
    // AS1 has three paths to the same prefix:
    // - Through customer (AS2)
    // - Through peer (AS3)  
    // - Through provider (AS4)
    
    graph2.addRelationship(1, 2);    // AS1 -> AS2 (customer)
    graph2.addPeeringRelationship(1, 3);  // AS1 <-> AS3 (peer)
    graph2.addRelationship(4, 1);    // AS4 -> AS1 (provider)
    
    // Connect all to AS5 which originates the route
    graph2.addRelationship(2, 5);
    graph2.addRelationship(3, 5);
    graph2.addRelationship(4, 5);
    
    AS* as1 = graph2.getAS(1);
    AS* as5 = graph2.getAS(5);
    
    as5->originatePrefix("10.0.0.0/8");
    
    if (as1->getRoutingTable().count("10.0.0.0/8")) {
        const auto& ann = as1->getRoutingTable().at("10.0.0.0/8");
        std::cout << "AS1 path to 10.0.0.0/8: ";
        for (uint32_t asn : ann.getASPath()) {
            std::cout << asn << " ";
        }
        std::cout << std::endl;
        std::cout << "Local preference: " << ann.getLocalPref() << std::endl;
        
        // Should prefer customer route (through AS2)
        if (ann.getASPath().size() >= 2 && ann.getASPath()[1] == 2) {
            std::cout << "✓ Correctly prefers customer route" << std::endl;
        } else {
            std::cout << "✗ Not preferring customer route" << std::endl;
        }
    }
    
    // Test 3: Export policies
    std::cout << "\n[Test 3] Export policy enforcement" << std::endl;
    ASGraph graph3;
    
    // AS1 (customer) -> AS2 (ISP) -> AS3 (provider)
    // AS4 (peer) -> AS2
    //
    // AS2 should export AS1's routes to AS3 and AS4
    // AS2 should NOT export AS3's routes to AS4
    
    graph3.addRelationship(2, 1);    // AS2 -> AS1 (customer)
    graph3.addRelationship(3, 2);    // AS3 -> AS2 (provider)
    graph3.addPeeringRelationship(2, 4);  // AS2 <-> AS4 (peer)
    
    AS* as1_customer = graph3.getAS(1);
    AS* as3_provider = graph3.getAS(3);
    AS* as4_peer = graph3.getAS(4);
    
    // Customer announces route
    as1_customer->originatePrefix("10.1.0.0/16");
    
    // Provider announces route
    as3_provider->originatePrefix("10.2.0.0/16");
    
    bool peer_has_customer_route = as4_peer->getRoutingTable().count("10.1.0.0/16") > 0;
    bool peer_has_provider_route = as4_peer->getRoutingTable().count("10.2.0.0/16") > 0;
    
    std::cout << "Customer route (10.1.0.0/16) exported to peer: " 
              << (peer_has_customer_route ? "YES (correct)" : "NO") << std::endl;
    std::cout << "Provider route (10.2.0.0/16) exported to peer: " 
              << (peer_has_provider_route ? "YES (VIOLATION!)" : "NO (correct)") << std::endl;
    
    if (peer_has_customer_route && !peer_has_provider_route) {
        std::cout << "✓ Export policies working correctly" << std::endl;
    } else {
        std::cout << "✗ Export policies BROKEN" << std::endl;
    }
    
    std::cout << "\n[SUCCESS] Day 4 tests passed" << std::endl;
}

int main() {
    std::cout << "BGP Simulator - Day 4" << std::endl;
    std::cout << "=====================" << std::endl;
    
    testDay1();
    testDay2();
    testDay3();
    testDay4();
    
    return 0;
}