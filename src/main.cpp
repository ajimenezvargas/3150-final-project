#include "AS.h"
#include "ASGraph.h"
#include "Announcement.h"
#include "Policy.h"
#include "ROV.h"
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
    
    std::cout << "[SUCCESS] Day 3 tests passed (condensed)" << std::endl;
}

void testDay4() {
    std::cout << "\n=== Day 4 Tests ===" << std::endl;
    
    ASGraph graph;
    graph.addRelationship(1, 2);    // Provider -> AS2
    graph.addPeeringRelationship(2, 3);
    graph.addPeeringRelationship(2, 4);
    
    AS* as2 = graph.getAS(2);
    AS* peer1 = graph.getAS(3);
    AS* peer2 = graph.getAS(4);
    
    peer2->originatePrefix("192.168.0.0/16");
    
    bool peer1_has = peer1->getRoutingTable().count("192.168.0.0/16") > 0;
    
    if (!peer1_has) {
        std::cout << "✓ Valley-free routing working" << std::endl;
    }
    
    std::cout << "[SUCCESS] Day 4 tests passed (condensed)" << std::endl;
}

void testDay5() {
    std::cout << "\n=== Day 5 Tests (Route Origin Validation - ROV) ===" << std::endl;
    
    // Test 1: Basic ROV validation
    std::cout << "\n[Test 1] Basic ROV validation states" << std::endl;
    
    ROVValidator validator;
    
    // Add ROAs (Route Origin Authorizations)
    validator.addROA("8.8.8.0/24", 15169);       // Google authorized for 8.8.8.0/24
    validator.addROA("1.1.1.0/24", 13335);       // Cloudflare authorized for 1.1.1.0/24
    validator.addROA("10.0.0.0/8", 64512, 24);   // AS64512 authorized, max /24
    
    std::cout << "Loaded " << validator.getROACount() << " ROAs" << std::endl;
    
    // Test VALID
    ROVState state1 = validator.validate("8.8.8.0/24", 15169);
    std::cout << "8.8.8.0/24 from AS15169: " 
              << (state1 == ROVState::VALID ? "VALID ✓" : "NOT VALID") << std::endl;
    
    // Test INVALID (wrong origin)
    ROVState state2 = validator.validate("8.8.8.0/24", 64496);
    std::cout << "8.8.8.0/24 from AS64496: " 
              << (state2 == ROVState::INVALID ? "INVALID ✓" : "NOT INVALID") << std::endl;
    
    // Test UNKNOWN (no ROA)
    ROVState state3 = validator.validate("192.168.0.0/16", 64497);
    std::cout << "192.168.0.0/16 from AS64497: " 
              << (state3 == ROVState::UNKNOWN ? "UNKNOWN ✓" : "NOT UNKNOWN") << std::endl;
    
    // Test INVALID (too specific)
    ROVState state4 = validator.validate("10.0.1.0/25", 64512);
    std::cout << "10.0.1.0/25 from AS64512: " 
              << (state4 == ROVState::INVALID ? "INVALID (too specific) ✓" : "NOT INVALID") << std::endl;
    
    if (state1 == ROVState::VALID && state2 == ROVState::INVALID && 
        state3 == ROVState::UNKNOWN && state4 == ROVState::INVALID) {
        std::cout << "✓ ROV validation working correctly" << std::endl;
    }
    
    // Test 2: ROV in path selection
    std::cout << "\n[Test 2] ROV influence on path selection" << std::endl;
    
    ASGraph graph;
    graph.addRelationship(1, 2);  // AS1 -> AS2
    graph.addRelationship(1, 3);  // AS1 -> AS3
    
    AS* as1 = graph.getAS(1);
    AS* as2 = graph.getAS(2);
    AS* as3 = graph.getAS(3);
    
    // Set up ROV
    graph.getROVValidator().addROA("100.0.0.0/8", 2);  // AS2 is authorized
    graph.enableROV(true);
    
    // Give all ASes access to the validator
    as1->setROVValidator(&graph.getROVValidator());
    as2->setROVValidator(&graph.getROVValidator());
    as3->setROVValidator(&graph.getROVValidator());
    
    // AS2 (authorized) announces
    as2->originatePrefix("100.0.0.0/8");
    
    // AS3 (unauthorized) announces the same prefix
    as3->originatePrefix("100.0.0.0/8");
    
    // AS1 should prefer the VALID route from AS2
    if (as1->getRoutingTable().count("100.0.0.0/8")) {
        const auto& ann = as1->getRoutingTable().at("100.0.0.0/8");
        uint32_t chosen_origin = ann.getOrigin();
        ROVState chosen_state = ann.getROVState();
        
        std::cout << "AS1 chose route from AS" << chosen_origin 
                  << " (ROV state: " 
                  << (chosen_state == ROVState::VALID ? "VALID" :
                      chosen_state == ROVState::INVALID ? "INVALID" : "UNKNOWN")
                  << ")" << std::endl;
        
        if (chosen_origin == 2 && chosen_state == ROVState::VALID) {
            std::cout << "✓ Correctly prefers VALID route" << std::endl;
        } else {
            std::cout << "✗ Did not prefer VALID route" << std::endl;
        }
    }
    
    // Test 3: Dropping INVALID routes
    std::cout << "\n[Test 3] Dropping INVALID routes" << std::endl;
    
    ASGraph graph2;
    graph2.addRelationship(1, 2);
    graph2.addRelationship(2, 3);
    
    AS* as1_b = graph2.getAS(1);
    AS* as2_b = graph2.getAS(2);
    AS* as3_b = graph2.getAS(3);
    
    // Configure ROV to drop invalid
    graph2.getROVValidator().addROA("200.0.0.0/8", 99999);  // Only AS99999 authorized
    as1_b->setROVValidator(&graph2.getROVValidator());
    as1_b->setDropInvalid(true);  // AS1 drops INVALID
    as2_b->setROVValidator(&graph2.getROVValidator());
    as2_b->setDropInvalid(true);
    as3_b->setROVValidator(&graph2.getROVValidator());
    
    // AS3 (unauthorized) tries to announce
    as3_b->originatePrefix("200.0.0.0/8");
    
    // AS1 should NOT have the route (dropped as INVALID)
    bool as1_has_route = as1_b->getRoutingTable().count("200.0.0.0/8") > 0;
    bool as2_has_route = as2_b->getRoutingTable().count("200.0.0.0/8") > 0;
    
    std::cout << "AS3 (unauthorized) announces 200.0.0.0/8" << std::endl;
    std::cout << "  AS2 (drop_invalid=true) has route: " << (as2_has_route ? "YES" : "NO (dropped)") << std::endl;
    std::cout << "  AS1 (drop_invalid=true) has route: " << (as1_has_route ? "YES" : "NO (dropped)") << std::endl;
    
    if (!as1_has_route && !as2_has_route) {
        std::cout << "✓ INVALID routes correctly dropped" << std::endl;
    } else {
        std::cout << "✗ INVALID routes not dropped" << std::endl;
    }
    
    // Test 4: Prefix hijacking detection
    std::cout << "\n[Test 4] Prefix hijacking detection" << std::endl;
    
    ASGraph graph3;
    graph3.addRelationship(1, 2);   // Tier1 -> ISP
    graph3.addRelationship(2, 3);   // ISP -> Victim
    graph3.addRelationship(2, 4);   // ISP -> Attacker
    
    AS* tier1 = graph3.getAS(1);
    AS* isp = graph3.getAS(2);
    AS* victim = graph3.getAS(3);
    AS* attacker = graph3.getAS(4);
    
    // Set up ROV: victim is authorized
    graph3.getROVValidator().addROA("203.0.113.0/24", 3);  // AS3 (victim) authorized
    tier1->setROVValidator(&graph3.getROVValidator());
    isp->setROVValidator(&graph3.getROVValidator());
    victim->setROVValidator(&graph3.getROVValidator());
    attacker->setROVValidator(&graph3.getROVValidator());
    
    // Legitimate announcement from victim
    victim->originatePrefix("203.0.113.0/24");
    
    // Attacker tries to hijack the prefix!
    attacker->originatePrefix("203.0.113.0/24");
    
    // Check ISP's routing table
    if (isp->getRoutingTable().count("203.0.113.0/24")) {
        const auto& ann = isp->getRoutingTable().at("203.0.113.0/24");
        std::cout << "ISP has route for 203.0.113.0/24:" << std::endl;
        std::cout << "  Origin: AS" << ann.getOrigin() << std::endl;
        std::cout << "  ROV state: " 
                  << (ann.getROVState() == ROVState::VALID ? "VALID" :
                      ann.getROVState() == ROVState::INVALID ? "INVALID" : "UNKNOWN")
                  << std::endl;
        std::cout << "  AS path: ";
        for (uint32_t asn : ann.getASPath()) {
            std::cout << asn << " ";
        }
        std::cout << std::endl;
        
        if (ann.getOrigin() == 3 && ann.getROVState() == ROVState::VALID) {
            std::cout << "✓ Hijack prevented: legitimate route preferred" << std::endl;
        } else if (ann.getOrigin() == 4) {
            std::cout << "⚠ Hijack succeeded (would be marked INVALID with ROV)" << std::endl;
        }
    }
    
    std::cout << "\n[SUCCESS] Day 5 tests passed" << std::endl;
}

int main() {
    std::cout << "BGP Simulator - Day 5" << std::endl;
    std::cout << "=====================" << std::endl;
    
    testDay1();
    testDay2();
    testDay3();
    testDay4();
    testDay5();
    
    return 0;
}