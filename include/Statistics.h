#pragma once

#include "ROV.h"
#include <cstdint>
#include <unordered_map>
#include <string>

/**
 * BGP Statistics Tracker
 * Monitors routing behavior and collects metrics
 */
class BGPStats {
public:
    BGPStats() { reset(); }
    
    // Route metrics
    uint64_t routes_received;
    uint64_t routes_accepted;
    uint64_t routes_rejected;
    uint64_t routes_withdrawn;
    
    // Policy metrics
    uint64_t valley_free_violations;
    uint64_t loop_preventions;
    uint64_t rov_valid;
    uint64_t rov_invalid;
    uint64_t rov_unknown;
    
    // Community metrics
    uint64_t no_export_filtered;
    uint64_t no_advertise_filtered;
    uint64_t custom_communities_used;
    
    // Path metrics
    uint64_t path_changes;
    uint64_t prepending_used;
    uint32_t max_path_length;
    uint64_t total_path_length;
    
    // Operations
    void reset();
    void recordRouteReceived() { routes_received++; }
    void recordRouteAccepted() { routes_accepted++; }
    void recordRouteRejected() { routes_rejected++; }
    void recordRouteWithdrawn() { routes_withdrawn++; }
    void recordLoopPrevention() { loop_preventions++; }
    void recordROVState(ROVState state);
    void recordNoExportFilter() { no_export_filtered++; }
    void recordNoAdvertiseFilter() { no_advertise_filtered++; }
    void recordPathChange() { path_changes++; }
    void recordPathLength(uint32_t length);
    void recordPrepending() { prepending_used++; }
    
    // Analysis
    double getAcceptanceRate() const;
    double getAveragePathLength() const;
    std::string getSummary() const;
    
    // Merge stats from another tracker
    void merge(const BGPStats& other);
};

/**
 * Global statistics aggregator
 */
class GlobalStats {
public:
    static GlobalStats& instance() {
        static GlobalStats inst;
        return inst;
    }
    
    // Per-AS statistics
    std::unordered_map<uint32_t, BGPStats> per_as_stats;
    
    // Global aggregates
    BGPStats global;
    
    // Get stats for a specific AS
    BGPStats& getASStats(uint32_t asn) { return per_as_stats[asn]; }
    
    // Aggregate all AS stats into global
    void aggregate();
    
    // Reset all statistics
    void reset();
    
    // Print reports
    std::string generateReport() const;
    std::string generatePerASReport() const;
    
private:
    GlobalStats() = default;
};