// RecentPatientCache.h - small FIXED-SIZE ARRAY cache of frequently accessed patient records.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// Uses a plain C++ array (not a vector) of 10 slots, as required by the brief:
//     const PatientRecord* slots_[10]
// The cache stores NON-OWNING pointers: the ClinicDatabase still owns every patient, so the
// cache must be told (invalidate/clear) whenever a record is deleted or replaced.
// Slot 0 holds the most recently used record; when the cache is full the least recently used
// record (last slot) is dropped.
#pragma once

#include <cstddef>
#include <string>

#include "model/PatientRecord.h"

namespace scapms::controller {

class RecentPatientCache {
public:
    static constexpr std::size_t kCapacity = 10;

    RecentPatientCache();

    /// Returns the cached record (and moves it to the front) or nullptr. Counts a hit or a miss.
    const model::PatientRecord* find(const std::string& id);

    /// Add (or refresh) a record at the front of the cache.
    void put(const model::PatientRecord* patient);

    /// Forget one patient (call before deleting/replacing the record).
    void invalidate(const std::string& id);
    void clear();

    std::size_t size() const { return count_; }
    const model::PatientRecord* at(std::size_t index) const;
    int usesAt(std::size_t index) const;
    int hits() const { return hits_; }
    int misses() const { return misses_; }

private:
    void moveToFront(std::size_t index);

    const model::PatientRecord* slots_[kCapacity];  // the fixed-size array
    int uses_[kCapacity];                           // how many times each slot was accessed
    std::size_t count_ = 0;
    int hits_ = 0;
    int misses_ = 0;
};

} // namespace scapms::controller
