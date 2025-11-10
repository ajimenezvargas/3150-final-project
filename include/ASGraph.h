#pragma once

#include "AS.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

/**
 * ASGraph - Manages the entire AS topology
 */
class ASGraph {
public:
    ASGraph() = default;
    
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
    
private:
    std::unordered_map<uint32_t, std::unique_ptr<AS>> ases_;
    
    // Cycle detection helper
    bool hasCycleDFS(const AS* node, 
                     std::unordered_map<uint32_t, int>& visited,
                     std::vector<uint32_t>& path) const;
};