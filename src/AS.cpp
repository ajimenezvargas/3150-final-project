#include "AS.h"
#include "Announcement.h"
#include <algorithm>

AS::AS(uint32_t asn) : asn_(asn), propagation_rank_(-1) {}

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

// Day 3: Announcement handling

void AS::originatePrefix(const std::string& prefix) {
    Announcement ann(asn_, prefix);
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
    
    const std::string& prefix = ann.getPrefix();
    
    // Check if we have a route for this prefix
    auto it = routing_table_.find(prefix);
    if (it == routing_table_.end()) {
        // No existing route - accept
        routing_table_.insert({prefix, new_ann});
        propagateToNeighbors(new_ann);
    } else {
        // Have existing route - compare
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
    // Simplified BGP decision process:
    // 1. Prefer shorter AS path
    if (new_ann.getPathLength() < old_ann.getPathLength()) {
        return true;
    }
    if (new_ann.getPathLength() > old_ann.getPathLength()) {
        return false;
    }
    
    // 2. Tie-break by origin ASN (lower is better)
    return new_ann.getOrigin() < old_ann.getOrigin();
}

void AS::propagateToNeighbors(const Announcement& ann) {
    // Export to all customers
    for (AS* customer : customers_) {
        customer->receiveAnnouncement(ann, this);
    }
    
    // Export to all peers
    for (AS* peer : peers_) {
        peer->receiveAnnouncement(ann, this);
    }
    
    // Export to all providers
    for (AS* provider : providers_) {
        provider->receiveAnnouncement(ann, this);
    }
}