#pragma once

#include "Policy.h"
#include "ROV.h"
#include <cstdint>
#include <vector>
#include <string>

/**
 * BGP Announcement - represents a route announcement
 */
class Announcement {
public:
    Announcement() : origin_(0), prefix_(""), relationship_(Relationship::PROVIDER), 
                     local_pref_(100), rov_state_(ROVState::UNKNOWN) {}
    Announcement(uint32_t origin, const std::string& prefix);
    
    // Copy constructor and assignment
    Announcement(const Announcement& other) = default;
    Announcement& operator=(const Announcement& other) = default;
    
    // Getters
    uint32_t getOrigin() const { return origin_; }
    const std::string& getPrefix() const { return prefix_; }
    const std::vector<uint32_t>& getASPath() const { return as_path_; }
    Relationship getRelationship() const { return relationship_; }
    int getLocalPref() const { return local_pref_; }
    ROVState getROVState() const { return rov_state_; }
    
    // Setters
    void setRelationship(Relationship rel);
    void setLocalPref(int pref) { local_pref_ = pref; }
    void setROVState(ROVState state) { rov_state_ = state; }
    
    // Path manipulation
    void prependASPath(uint32_t asn);
    bool hasASN(uint32_t asn) const;
    int getPathLength() const { return as_path_.size(); }
    
    // Copy announcement (for propagation)
    Announcement copy() const;
    
private:
    uint32_t origin_;                // Originating AS
    std::string prefix_;             // IP prefix (e.g., "1.0.0.0/24")
    std::vector<uint32_t> as_path_;  // AS path
    Relationship relationship_;      // How this route was learned
    int local_pref_;                 // Local preference value
    ROVState rov_state_;             // ROV validation state
};