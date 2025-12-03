#include "ASGraph.h"
#include <algorithm>
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
                          std::vector<uint32_t>& path,
                          std::vector<uint32_t>* cycle) const {
    uint32_t asn = node->getASN();

    if (visited[asn] == 1) {
        if (cycle) {
            auto it = std::find(path.begin(), path.end(), asn);
            if (it != path.end()) {
                cycle->assign(it, path.end());
                cycle->push_back(asn);
            }
        }
        return true;
    }
    if (visited[asn] == 2) {
        return false;
    }

    visited[asn] = 1;
    path.push_back(asn);
    
    for (const AS* provider : node->getProviders()) {
        uint32_t provider_asn = provider->getASN();

        if (visited[provider_asn] == 0) {
            if (hasCycleDFS(provider, visited, path, cycle)) {
                return true;
            }
        } else if (visited[provider_asn] == 1) {
            if (cycle) {
                auto it = std::find(path.begin(), path.end(), provider_asn);
                if (it != path.end()) {
                    cycle->assign(it, path.end());
                    cycle->push_back(provider_asn);
                }
            }
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
            if (hasCycleDFS(as_ptr.get(), visited, path, nullptr)) {
                return true;
            }
        }
    }
    
    return false;
}

std::vector<uint32_t> ASGraph::findCycle() const {
    std::unordered_map<uint32_t, int> visited;
    std::vector<uint32_t> path;
    std::vector<uint32_t> cycle;

    for (const auto& [asn, as_ptr] : ases_) {
        if (visited[asn] == 0) {
            if (hasCycleDFS(as_ptr.get(), visited, path, &cycle)) {
                if (!cycle.empty()) {
                    return cycle;
                }
                return path;
            }
        }
    }

    return {};
}

void ASGraph::computePropagationRanks() {
    // Reset all ranks
    for (const auto& [asn, as_ptr] : ases_) {
        as_ptr->setPropagationRank(-1);
    }

    // Assign ranks starting from all ASes
    for (const auto& [asn, as_ptr] : ases_) {
        assignRanksHelper(as_ptr.get(), 0);
    }

    // Find max rank
    int max_rank = -1;
    for (const auto& [asn, as_ptr] : ases_) {
        max_rank = std::max(max_rank, as_ptr->getPropagationRank());
    }

    // Group ASes by rank
    propagation_ranks_.clear();
    propagation_ranks_.resize(max_rank + 1);

    for (const auto& [asn, as_ptr] : ases_) {
        int rank = as_ptr->getPropagationRank();
        propagation_ranks_[rank].push_back(as_ptr.get());
    }

    // Sort ASes within each rank by ASN for determinism
    for (auto& rank : propagation_ranks_) {
        std::sort(rank.begin(), rank.end(),
            [](const AS* a, const AS* b) { return a->getASN() < b->getASN(); });
    }
}

void ASGraph::assignRanksHelper(AS* as_obj, int rank) {
    // Only update if this rank is higher than current
    if (as_obj->getPropagationRank() < rank) {
        as_obj->setPropagationRank(rank);

        // Recursively assign ranks to providers (they get rank + 1)
        for (AS* provider : as_obj->getProviders()) {
            assignRanksHelper(provider, rank + 1);
        }
    }
}
