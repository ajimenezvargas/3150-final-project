#include "Announcement.h"
#include <algorithm>

Announcement::Announcement(uint32_t origin, const std::string& prefix)
    : origin_(origin), prefix_(prefix) {
    as_path_.push_back(origin);
}

void Announcement::prependASPath(uint32_t asn) {
    as_path_.insert(as_path_.begin(), asn);
}

bool Announcement::hasASN(uint32_t asn) const {
    return std::find(as_path_.begin(), as_path_.end(), asn) != as_path_.end();
}

Announcement Announcement::copy() const {
    Announcement ann(origin_, prefix_);
    ann.as_path_ = as_path_;
    return ann;
}