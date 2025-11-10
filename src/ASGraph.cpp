#include "ASGraph.h"
#include <iostream>

AS* ASGraph::getOrCreateAS(uint32_t asn) {
    auto it = ases_.find(asn);
    if (it != ases_.end()) {
        return it->second.get();
    }
    
    // Create new AS
    auto as = std::make_unique<AS>(asn);
    AS* ptr = as.get();
    ases_[asn] = std::move(as);
    return ptr;
}

void ASGraph::addRelationship(uint32_t provider_asn, uint32_t customer_asn) {
    AS* provider = getOrCreateAS(provider_asn);
    AS* customer = getOrCreateAS(customer_asn);
    
    provider->addCustomer(customer);
    customer->addProvider(provider);
}

void ASGraph::addPeeringRelationship(uint32_t asn1, uint32_t asn2) {
    AS* as1 = getOrCreateAS(asn1);
    AS* as2 = getOrCreateAS(asn2);
    
    as1->addPeer(as2);
    as2->addPeer(as1);
}

AS* ASGraph::getAS(uint32_t asn) const {
    auto it = ases_.find(asn);
    return (it != ases_.end()) ? it->second.get() : nullptr;
}

bool ASGraph::hasCycleDFS(const AS* node,
                          std::unordered_map<uint32_t, int>& visited,
                          std::vector<uint32_t>& path) const {
    uint32_t asn = node->getASN();
    
    // 0 = unvisited, 1 = in current path, 2 = fully explored
    if (visited[asn] == 1) {
        return true; // Cycle found
    }
    if (visited[asn] == 2) {
        return false; // Already checked
    }
    
    visited[asn] = 1;
    path.push_back(asn);
    
    // Check all providers (following customer-to-provider edges)
    for (const AS* provider : node->getProviders()) {
        if (hasCycleDFS(provider, visited, path)) {
            return true;
        }
    }
    
    visited[asn] = 2;
    path.pop_back();
    return false;
}

bool ASGraph::hasCycle() const {
    std::unordered_map<uint32_t, int> visited;
    std::vector<uint32_t> path;
    
    for (const auto& [asn, as_ptr] : ases_) {
        if (visited[asn] == 0) {
            if (hasCycleDFS(as_ptr.get(), visited, path)) {
                return true;
            }
        }
    }
    
    return false;
}

std::vector<uint32_t> ASGraph::findCycle() const {
    std::unordered_map<uint32_t, int> visited;
    std::vector<uint32_t> path;
    
    for (const auto& [asn, as_ptr] : ases_) {
        if (visited[asn] == 0) {
            if (hasCycleDFS(as_ptr.get(), visited, path)) {
                return path;
            }
        }
    }
    
    return {};
}