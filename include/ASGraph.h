#pragma once

#include "AS.h"
#include "ROV.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

/**
 * ASGraph - Manages the entire AS topology
 */
class ASGraph {
public:
    ASGraph() : rov_enabled_(false) {}
    
    // Graph construction
    AS* getOrCreateAS(uint32_t asn);
    void addRelationship(uint32_t provider_asn, uint32_t customer_asn);
    void addPeeringRelationship(uint32_t asn1, uint32_t asn2);
    
    // Graph access
    AS* getAS(uint32_t asn) const;
    size_t size() const { return ases_.size(); }
    const std::unordered_map<uint32_t, std::unique_ptr<AS>>& getAllASes() const { return ases_; }
    
    // Validation
    bool hasCycle() const;
    std::vector<uint32_t> findCycle() const;
    
    // ROV Support (Day 5)
    ROVValidator& getROVValidator() { return rov_validator_; }
    const ROVValidator& getROVValidator() const { return rov_validator_; }
    void enableROV(bool enable) { rov_enabled_ = enable; }
    bool isROVEnabled() const { return rov_enabled_; }
    
private:
    std::unordered_map<uint32_t, std::unique_ptr<AS>> ases_;
    ROVValidator rov_validator_;
    bool rov_enabled_;
    
    // Cycle detection helper
    bool hasCycleDFS(const AS* node, 
                     std::unordered_map<uint32_t, int>& visited,
                     std::vector<uint32_t>& path) const;
};