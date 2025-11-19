#include "Statistics.h"
#include "ROV.h"
#include <sstream>
#include <iomanip>

void BGPStats::reset() {
    routes_received = 0;
    routes_accepted = 0;
    routes_rejected = 0;
    routes_withdrawn = 0;
    valley_free_violations = 0;
    loop_preventions = 0;
    rov_valid = 0;
    rov_invalid = 0;
    rov_unknown = 0;
    no_export_filtered = 0;
    no_advertise_filtered = 0;
    custom_communities_used = 0;
    path_changes = 0;
    prepending_used = 0;
    max_path_length = 0;
    total_path_length = 0;
}

void BGPStats::recordROVState(ROVState state) {
    switch (state) {
        case ROVState::VALID:
            rov_valid++;
            break;
        case ROVState::INVALID:
            rov_invalid++;
            break;
        case ROVState::UNKNOWN:
            rov_unknown++;
            break;
    }
}

void BGPStats::recordPathLength(uint32_t length) {
    if (length > max_path_length) {
        max_path_length = length;
    }
    total_path_length += length;
}

double BGPStats::getAcceptanceRate() const {
    if (routes_received == 0) return 0.0;
    return (double)routes_accepted / routes_received * 100.0;
}

double BGPStats::getAveragePathLength() const {
    if (routes_accepted == 0) return 0.0;
    return (double)total_path_length / routes_accepted;
}

std::string BGPStats::getSummary() const {
    std::ostringstream oss;
    oss << "Route Statistics:\n";
    oss << "  Received: " << routes_received << "\n";
    oss << "  Accepted: " << routes_accepted << "\n";
    oss << "  Rejected: " << routes_rejected << "\n";
    oss << "  Acceptance Rate: " << std::fixed << std::setprecision(1) 
        << getAcceptanceRate() << "%\n";
    
    if (loop_preventions > 0) {
        oss << "\nLoop Prevention:\n";
        oss << "  Loops prevented: " << loop_preventions << "\n";
    }
    
    if (rov_valid + rov_invalid + rov_unknown > 0) {
        oss << "\nROV Statistics:\n";
        oss << "  VALID: " << rov_valid << "\n";
        oss << "  INVALID: " << rov_invalid << "\n";
        oss << "  UNKNOWN: " << rov_unknown << "\n";
    }
    
    if (no_export_filtered + no_advertise_filtered > 0) {
        oss << "\nCommunity Filtering:\n";
        oss << "  NO_EXPORT filtered: " << no_export_filtered << "\n";
        oss << "  NO_ADVERTISE filtered: " << no_advertise_filtered << "\n";
    }
    
    if (routes_accepted > 0) {
        oss << "\nPath Metrics:\n";
        oss << "  Average path length: " << std::fixed << std::setprecision(2) 
            << getAveragePathLength() << "\n";
        oss << "  Max path length: " << max_path_length << "\n";
        if (prepending_used > 0) {
            oss << "  Prepending used: " << prepending_used << " times\n";
        }
    }
    
    return oss.str();
}

void BGPStats::merge(const BGPStats& other) {
    routes_received += other.routes_received;
    routes_accepted += other.routes_accepted;
    routes_rejected += other.routes_rejected;
    routes_withdrawn += other.routes_withdrawn;
    valley_free_violations += other.valley_free_violations;
    loop_preventions += other.loop_preventions;
    rov_valid += other.rov_valid;
    rov_invalid += other.rov_invalid;
    rov_unknown += other.rov_unknown;
    no_export_filtered += other.no_export_filtered;
    no_advertise_filtered += other.no_advertise_filtered;
    custom_communities_used += other.custom_communities_used;
    path_changes += other.path_changes;
    prepending_used += other.prepending_used;
    if (other.max_path_length > max_path_length) {
        max_path_length = other.max_path_length;
    }
    total_path_length += other.total_path_length;
}

// GlobalStats implementation

void GlobalStats::aggregate() {
    global.reset();
    for (const auto& [asn, stats] : per_as_stats) {
        global.merge(stats);
    }
}

void GlobalStats::reset() {
    per_as_stats.clear();
    global.reset();
}

std::string GlobalStats::generateReport() const {
    std::ostringstream oss;
    oss << "=== GLOBAL BGP STATISTICS ===\n\n";
    oss << global.getSummary();
    oss << "\nTotal ASes tracked: " << per_as_stats.size() << "\n";
    return oss.str();
}

std::string GlobalStats::generatePerASReport() const {
    std::ostringstream oss;
    oss << "=== PER-AS STATISTICS ===\n\n";
    
    for (const auto& [asn, stats] : per_as_stats) {
        if (stats.routes_received > 0 || stats.routes_accepted > 0) {
            oss << "AS" << asn << ":\n";
            oss << "  Received: " << stats.routes_received;
            oss << ", Accepted: " << stats.routes_accepted;
            oss << ", Rejected: " << stats.routes_rejected;
            oss << " (" << std::fixed << std::setprecision(1) 
                << stats.getAcceptanceRate() << "% acceptance)\n";
        }
    }
    
    return oss.str();
}