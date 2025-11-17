#include "Policy.h"
#include "AS.h"
#include <algorithm>

bool Policy::shouldExport(Relationship learnedFrom, Relationship exportTo) {
    // Always export originated routes
    if (learnedFrom == Relationship::ORIGIN) {
        return true;
    }
    
    // Always export to customers (they pay us)
    if (exportTo == Relationship::CUSTOMER) {
        return true;
    }
    
    // Export customer routes to peers and providers
    if (learnedFrom == Relationship::CUSTOMER) {
        return true;
    }
    
    // Don't export peer routes to other peers or providers (valley-free)
    if (learnedFrom == Relationship::PEER && 
        (exportTo == Relationship::PEER || exportTo == Relationship::PROVIDER)) {
        return false;
    }
    
    // Don't export provider routes to peers or other providers (valley-free)
    if (learnedFrom == Relationship::PROVIDER &&
        (exportTo == Relationship::PEER || exportTo == Relationship::PROVIDER)) {
        return false;
    }
    
    return true;
}

int Policy::getLocalPreference(Relationship rel) {
    switch (rel) {
        case Relationship::ORIGIN:
            return 400;  // Highest - our own routes
        case Relationship::CUSTOMER:
            return 300;  // High - we make money
        case Relationship::PEER:
            return 200;  // Medium - free transit
        case Relationship::PROVIDER:
            return 100;  // Low - we pay for this
        default:
            return 0;
    }
}

Relationship Policy::getRelationship(const AS* from, const AS* to) {
    // Check if 'from' is a customer of 'to'
    const auto& customers = to->getCustomers();
    if (std::find(customers.begin(), customers.end(), from) != customers.end()) {
        return Relationship::CUSTOMER;
    }
    
    // Check if 'from' is a provider of 'to'
    const auto& providers = to->getProviders();
    if (std::find(providers.begin(), providers.end(), from) != providers.end()) {
        return Relationship::PROVIDER;
    }
    
    // Check if 'from' is a peer of 'to'
    const auto& peers = to->getPeers();
    if (std::find(peers.begin(), peers.end(), from) != peers.end()) {
        return Relationship::PEER;
    }
    
    // Should not happen if called correctly
    return Relationship::PROVIDER;
}