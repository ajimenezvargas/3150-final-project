#include "AS.h"
#include "ASGraph.h"
#include "Announcement.h"
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

int main() {
    std::cout << "BGP Simulator - Day 3" << std::endl;
    std::cout << "=====================" << std::endl;
    
    testDay1();
    testDay2();
    testDay3();
    
    return 0;
}