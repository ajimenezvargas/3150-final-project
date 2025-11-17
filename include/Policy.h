#pragma once

#include <cstdint>

/**
 * BGP Relationship types for policy decisions
 */
enum class Relationship {
    CUSTOMER,   // Routes learned from customers
    PROVIDER,   // Routes learned from providers
    PEER,       // Routes learned from peers
    ORIGIN      // Routes originated by this AS
};

/**
 * Policy helper functions
 */
class Policy {
public:
    /**
     * Should this AS export a route to a neighbor?
     * Implements valley-free routing:
     * - Export everything to customers
     * - Export customer routes to peers and providers
     * - Don't export peer/provider routes to peers/providers
     */
    static bool shouldExport(Relationship learnedFrom, Relationship exportTo);
    
    /**
     * Get local preference for a relationship
     * Higher values are preferred
     * CUSTOMER > PEER > PROVIDER
     */
    static int getLocalPreference(Relationship rel);
    
    /**
     * Determine relationship type between two ASes
     */
    static Relationship getRelationship(const class AS* from, const class AS* to);
};
