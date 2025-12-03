#include "AS.h"
#include "Announcement.h"
#include "Policy.h"
#include <algorithm>

AS::AS(uint32_t asn) 
    : asn_(asn), propagation_rank_(-1), rov_validator_(nullptr), drop_invalid_(false) {}

void AS::addProvider(AS* provider) {
    if (provider && std::find(providers_.begin(), providers_.end(), provider) == providers_.end()) {
        providers_.push_back(provider);
        // Sort providers by ASN for deterministic processing
        std::sort(providers_.begin(), providers_.end(),
            [](const AS* a, const AS* b) { return a->getASN() < b->getASN(); });
    }
}

void AS::addCustomer(AS* customer) {
    if (customer && std::find(customers_.begin(), customers_.end(), customer) == customers_.end()) {
        customers_.push_back(customer);
        // Sort customers by ASN for deterministic processing
        std::sort(customers_.begin(), customers_.end(),
            [](const AS* a, const AS* b) { return a->getASN() < b->getASN(); });
    }
}

void AS::addPeer(AS* peer) {
    if (peer && std::find(peers_.begin(), peers_.end(), peer) == peers_.end()) {
        peers_.push_back(peer);
        // Sort peers by ASN for deterministic processing
        std::sort(peers_.begin(), peers_.end(),
            [](const AS* a, const AS* b) { return a->getASN() < b->getASN(); });
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
    routes_to_propagate_[prefix] = ann;  // Mark for propagation
}

void AS::receiveAnnouncement(const Announcement& ann, AS* from) {
    // Queue the announcement for processing
    incoming_queue_.push_back({ann, from});
}

bool AS::processIncomingQueue() {
    bool changed = false;

    // Process all queued announcements
    for (const auto& queued : incoming_queue_) {
        const Announcement& ann = queued.ann;
        AS* from = queued.from;

        // Check if we should accept this announcement
        if (!shouldAccept(ann, from)) {
            continue;
        }

        // Loop prevention: reject if our ASN is already in the path
        if (ann.hasASN(asn_)) {
            continue;
        }

        // Create a copy and prepend our ASN
        Announcement new_ann = ann.copy();
        new_ann.prependASPath(asn_);

        // Determine relationship with sender
        Relationship rel = Policy::getRelationship(from, this);
        new_ann.setRelationship(rel);

        // Validate with ROV if available
        if (rov_validator_) {
            ROVState state = rov_validator_->validate(new_ann.getPrefix(), new_ann.getOrigin());
            new_ann.setROVState(state);

            // Drop INVALID routes if configured
            if (drop_invalid_ && state == ROVState::INVALID) {
                continue;  // Reject invalid route
            }
        }

        const std::string& prefix = ann.getPrefix();

        // Check if we have a route for this prefix
        auto it = routing_table_.find(prefix);
        if (it == routing_table_.end()) {
            // No existing route - accept
            routing_table_.insert({prefix, new_ann});
            routes_to_propagate_[prefix] = new_ann;
            changed = true;
        } else {
            // Have existing route - compare with policy-aware decision
            if (isBetterPath(new_ann, it->second)) {
                it->second = new_ann;
                routes_to_propagate_[prefix] = new_ann;
                changed = true;
            }
        }
    }

    // Clear the queue
    incoming_queue_.clear();

    return changed;
}

void AS::propagate() {
    // Propagate all current routes (like BGPy's local_rib)
    for (const auto& [prefix, ann] : routing_table_) {
        propagateToNeighbors(ann);
    }
}

void AS::propagateToProviders() {
    // Only propagate to providers (BGPy Phase 1)
    for (const auto& [prefix, ann] : routing_table_) {
        Relationship learnedFrom = ann.getRelationship();

        // Check communities
        if (ann.getCommunities().hasNoAdvertise()) {
            continue;
        }
        bool no_export = ann.getCommunities().hasNoExport();
        if (no_export) {
            continue;  // NO_EXPORT means don't advertise to providers
        }

        // Export to providers (if policy allows)
        if (Policy::shouldExport(learnedFrom, Relationship::PROVIDER)) {
            for (AS* provider : providers_) {
                provider->receiveAnnouncement(ann, this);
            }
        }
    }
}

void AS::propagateToPeers() {
    // Only propagate to peers (BGPy Phase 2)
    for (const auto& [prefix, ann] : routing_table_) {
        Relationship learnedFrom = ann.getRelationship();

        // Check communities
        if (ann.getCommunities().hasNoAdvertise()) {
            continue;
        }
        bool no_export = ann.getCommunities().hasNoExport();
        if (no_export) {
            continue;  // NO_EXPORT means don't advertise to peers
        }

        // Export to peers (if policy allows)
        if (Policy::shouldExport(learnedFrom, Relationship::PEER)) {
            for (AS* peer : peers_) {
                peer->receiveAnnouncement(ann, this);
            }
        }
    }
}

void AS::propagateToCustomers() {
    // Only propagate to customers (BGPy Phase 3)
    for (const auto& [prefix, ann] : routing_table_) {
        Relationship learnedFrom = ann.getRelationship();

        // Check communities
        if (ann.getCommunities().hasNoAdvertise()) {
            continue;
        }
        // NO_EXPORT still allows advertising to customers

        // Export to customers (if policy allows)
        if (Policy::shouldExport(learnedFrom, Relationship::CUSTOMER)) {
            for (AS* customer : customers_) {
                customer->receiveAnnouncement(ann, this);
            }
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

    // 0. ROV state preference (ONLY for ROV-enabled ASes)
    // Non-ROV ASes don't prefer VALID over INVALID, they just route normally
    if (drop_invalid_ && rov_validator_) {
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

    // 3. Tie-break by neighbor ASN (BGPy algorithm)
    // Use second element in path, or first if path has only one element
    const auto& new_path = new_ann.getASPath();
    const auto& old_path = old_ann.getASPath();

    size_t new_neighbor_idx = std::min(new_path.size(), size_t(1));
    size_t old_neighbor_idx = std::min(old_path.size(), size_t(1));

    uint32_t new_neighbor = new_path[new_neighbor_idx];
    uint32_t old_neighbor = old_path[old_neighbor_idx];

    // Prefer lower neighbor ASN
    if (new_neighbor < old_neighbor) {
        return true;
    }
    if (new_neighbor > old_neighbor) {
        return false;
    }

    // 4. Final tie-break: keep existing path (first-come, first-served)
    // This ensures deterministic behavior
    return false;
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