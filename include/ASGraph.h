#pragma once

#include "AS.h"
#include "ROV.h"
#include <map>
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
    const std::map<uint32_t, std::unique_ptr<AS>>& getAllASes() const { return ases_; }
    
    // Validation
    bool hasCycle() const;
    std::vector<uint32_t> findCycle() const;
    
    // ROV Support (Day 5)
    ROVValidator& getROVValidator() { return rov_validator_; }
    const ROVValidator& getROVValidator() const { return rov_validator_; }
    void enableROV(bool enable) { rov_enabled_ = enable; }
    bool isROVEnabled() const { return rov_enabled_; }

    // Propagation ranks (BGPy-style hierarchical propagation)
    void computePropagationRanks();
    const std::vector<std::vector<AS*>>& getPropagationRanks() const { return propagation_ranks_; }
    
private:
    std::map<uint32_t, std::unique_ptr<AS>> ases_;
    ROVValidator rov_validator_;
    bool rov_enabled_;

    // Propagation ranks: ASes grouped by hierarchy level
    std::vector<std::vector<AS*>> propagation_ranks_;

    // Cycle detection helper
    bool hasCycleDFS(const AS* node,
                     std::unordered_map<uint32_t, int>& visited,
                     std::vector<uint32_t>& path) const;

    // Propagation rank helpers
    void assignRanksHelper(AS* as_obj, int rank);
};