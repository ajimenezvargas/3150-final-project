#include "AS.h"
#include "Announcement.h"
#include "Policy.h"
#include <algorithm>

AS::AS(uint32_t asn) 
    : asn_(asn), propagation_rank_(-1), rov_validator_(nullptr), drop_invalid_(false) {}

void AS::addProvider(AS* provider) {
    if (provider && std::find(providers_.begin(), providers_.end(), provider) == providers_.end()) {
        providers_.push_back(provider);
    }
}

void AS::addCustomer(AS* customer) {
    if (customer && std::find(customers_.begin(), customers_.end(), customer) == customers_.end()) {
        customers_.push_back(customer);
    }
}

void AS::addPeer(AS* peer) {
    if (peer && std::find(peers_.begin(), peers_.end(), peer) == peers_.end()) {
        peers_.push_back(peer);
    }
}

// Day 3-5: Announcement handling with policies and ROV

void AS::originatePrefix(const std::string& prefix) {
    Announcement ann(asn_, prefix);
    ann.setRelationship(Relationship::ORIGIN);
    
    // Validate with ROV if available
    if (rov_validator_) {
        ROVState state = rov_validator_->validate(prefix, asn_);
        ann.setROVState(state);
    }
    
    routing_table_.insert({prefix, ann});
    propagateToNeighbors(ann);
}

void AS::receiveAnnouncement(const Announcement& ann, AS* from) {
    // Check if we should accept this announcement
    if (!shouldAccept(ann, from)) {
        return;
    }
    
    // Loop prevention: reject if our ASN is already in the path
    if (ann.hasASN(asn_)) {
        return;
    }
    
    // Create a copy and prepend our ASN
    Announcement new_ann = ann.copy();
    new_ann.prependASPath(asn_);
    
    // Determine relationship with sender
    Relationship rel = Policy::getRelationship(from, this);
    new_ann.setRelationship(rel);
    
    // Validate with ROV if available (Day 5)
    if (rov_validator_) {
        ROVState state = rov_validator_->validate(new_ann.getPrefix(), new_ann.getOrigin());
        new_ann.setROVState(state);
        
        // Drop INVALID routes if configured
        if (drop_invalid_ && state == ROVState::INVALID) {
            return;  // Reject invalid route
        }
    }
    
    const std::string& prefix = ann.getPrefix();
    
    // Check if we have a route for this prefix
    auto it = routing_table_.find(prefix);
    if (it == routing_table_.end()) {
        // No existing route - accept
        routing_table_.insert({prefix, new_ann});
        propagateToNeighbors(new_ann);
    } else {
        // Have existing route - compare with policy-aware decision
        if (isBetterPath(new_ann, it->second)) {
            it->second = new_ann;
            propagateToNeighbors(new_ann);
        }
    }
}

bool AS::shouldAccept(const Announcement&, AS* from) const {
    // Check if 'from' is a valid neighbor
    bool is_customer = std::find(customers_.begin(), customers_.end(), from) != customers_.end();
    bool is_provider = std::find(providers_.begin(), providers_.end(), from) != providers_.end();
    bool is_peer = std::find(peers_.begin(), peers_.end(), from) != peers_.end();
    
    return is_customer || is_provider || is_peer;
}

bool AS::isBetterPath(const Announcement& new_ann, const Announcement& old_ann) const {
    // BGP decision process with policies and ROV:
    
    // 0. ROV state (if enabled) - VALID > UNKNOWN > INVALID
    if (rov_validator_) {
        ROVState new_state = new_ann.getROVState();
        ROVState old_state = old_ann.getROVState();
        
        // Prefer VALID over UNKNOWN
        if (new_state == ROVState::VALID && old_state != ROVState::VALID) {
            return true;
        }
        if (old_state == ROVState::VALID && new_state != ROVState::VALID) {
            return false;
        }
        
        // Prefer UNKNOWN over INVALID
        if (new_state == ROVState::UNKNOWN && old_state == ROVState::INVALID) {
            return true;
        }
        if (old_state == ROVState::UNKNOWN && new_state == ROVState::INVALID) {
            return false;
        }
    }
    
    // 1. Prefer higher local preference (based on relationship)
    if (new_ann.getLocalPref() > old_ann.getLocalPref()) {
        return true;
    }
    if (new_ann.getLocalPref() < old_ann.getLocalPref()) {
        return false;
    }
    
    // 2. Prefer shorter AS path
    if (new_ann.getPathLength() < old_ann.getPathLength()) {
        return true;
    }
    if (new_ann.getPathLength() > old_ann.getPathLength()) {
        return false;
    }
    
    // 3. Tie-break by origin ASN (lower is better for stability)
    return new_ann.getOrigin() < old_ann.getOrigin();
}

void AS::propagateToNeighbors(const Announcement& ann) {
    Relationship learnedFrom = ann.getRelationship();
    
    // Check for NO_ADVERTISE community - don't propagate at all
    if (ann.getCommunities().hasNoAdvertise()) {
        return;  // Don't advertise to anyone
    }
    
    // Check for NO_EXPORT community - only advertise to customers
    bool no_export = ann.getCommunities().hasNoExport();
    
    // Export to customers (if policy allows)
    for (AS* customer : customers_) {
        if (Policy::shouldExport(learnedFrom, Relationship::CUSTOMER)) {
            customer->receiveAnnouncement(ann, this);
        }
    }
    
    // Don't export to peers/providers if NO_EXPORT is set
    if (no_export) {
        return;
    }
    
    // Export to peers (if policy allows)
    for (AS* peer : peers_) {
        if (Policy::shouldExport(learnedFrom, Relationship::PEER)) {
            peer->receiveAnnouncement(ann, this);
        }
    }
    
    // Export to providers (if policy allows)
    for (AS* provider : providers_) {
        if (Policy::shouldExport(learnedFrom, Relationship::PROVIDER)) {
            provider->receiveAnnouncement(ann, this);
        }
    }
}