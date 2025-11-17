#pragma once

#include "Announcement.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

/**
 * Autonomous System (AS) class
 * Represents a node in the internet graph
 */
class AS {
public:
    // Constructor
    explicit AS(uint32_t asn);
    
    // Getters
    uint32_t getASN() const { return asn_; }
    const std::vector<AS*>& getProviders() const { return providers_; }
    const std::vector<AS*>& getCustomers() const { return customers_; }
    const std::vector<AS*>& getPeers() const { return peers_; }
    int getPropagationRank() const { return propagation_rank_; }
    
    // Setters
    void setPropagationRank(int rank) { propagation_rank_ = rank; }
    
    // Relationship management
    void addProvider(AS* provider);
    void addCustomer(AS* customer);
    void addPeer(AS* peer);
    
    // Helper methods
    bool hasCustomers() const { return !customers_.empty(); }
    bool hasProviders() const { return !providers_.empty(); }
    
    // Announcement/Routing (Day 3-5)
    void receiveAnnouncement(const Announcement& ann, AS* from);
    void originatePrefix(const std::string& prefix);
    const std::unordered_map<std::string, Announcement>& getRoutingTable() const { 
        return routing_table_; 
    }
    
    // ROV Support (Day 5)
    void setROVValidator(const ROVValidator* validator) { rov_validator_ = validator; }
    void setDropInvalid(bool drop) { drop_invalid_ = drop; }
    bool getDropInvalid() const { return drop_invalid_; }
    
private:
    uint32_t asn_;                    // Autonomous System Number (unique ID)
    std::vector<AS*> providers_;      // ASes that provide service to this AS
    std::vector<AS*> customers_;      // ASes that this AS provides service to
    std::vector<AS*> peers_;          // ASes that peer with this AS
    int propagation_rank_;            // Rank for propagation (will be set later)
    
    // Routing table: prefix -> best announcement
    std::unordered_map<std::string, Announcement> routing_table_;
    
    // ROV (Day 5)
    const ROVValidator* rov_validator_;  // Pointer to graph's validator
    bool drop_invalid_;                   // Drop INVALID routes?
    
    // BGP decision process
    bool shouldAccept(const Announcement& ann, AS* from) const;
    bool isBetterPath(const Announcement& new_ann, const Announcement& old_ann) const;
    void propagateToNeighbors(const Announcement& ann);
};