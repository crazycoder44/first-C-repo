// RecentPatientCache.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "controller/RecentPatientCache.h"

namespace scapms::controller {

RecentPatientCache::RecentPatientCache() {
    clear();
}

void RecentPatientCache::clear() {
    for (std::size_t i = 0; i < kCapacity; ++i) {
        slots_[i] = nullptr;
        uses_[i] = 0;
    }
    count_ = 0;
}

void RecentPatientCache::moveToFront(std::size_t index) {
    const model::PatientRecord* patient = slots_[index];
    const int uses = uses_[index];
    for (std::size_t i = index; i > 0; --i) {  // shift the entries in front of it one step back
        slots_[i] = slots_[i - 1];
        uses_[i] = uses_[i - 1];
    }
    slots_[0] = patient;
    uses_[0] = uses;
}

const model::PatientRecord* RecentPatientCache::find(const std::string& id) {
    for (std::size_t i = 0; i < count_; ++i) {
        if (slots_[i]->id() == id) {
            ++uses_[i];
            ++hits_;
            moveToFront(i);
            return slots_[0];
        }
    }
    ++misses_;
    return nullptr;
}

void RecentPatientCache::put(const model::PatientRecord* patient) {
    if (!patient) return;
    for (std::size_t i = 0; i < count_; ++i) {  // already cached -> just refresh
        if (slots_[i] == patient) {
            ++uses_[i];
            moveToFront(i);
            return;
        }
    }
    if (count_ < kCapacity) ++count_;           // otherwise the last slot is overwritten (evicted)
    for (std::size_t i = count_ - 1; i > 0; --i) {
        slots_[i] = slots_[i - 1];
        uses_[i] = uses_[i - 1];
    }
    slots_[0] = patient;
    uses_[0] = 1;
}

void RecentPatientCache::invalidate(const std::string& id) {
    for (std::size_t i = 0; i < count_; ++i) {
        if (slots_[i]->id() == id) {
            for (std::size_t j = i; j + 1 < count_; ++j) {
                slots_[j] = slots_[j + 1];
                uses_[j] = uses_[j + 1];
            }
            --count_;
            slots_[count_] = nullptr;
            uses_[count_] = 0;
            return;
        }
    }
}

const model::PatientRecord* RecentPatientCache::at(std::size_t index) const {
    return index < count_ ? slots_[index] : nullptr;
}

int RecentPatientCache::usesAt(std::size_t index) const {
    return index < count_ ? uses_[index] : 0;
}

} // namespace scapms::controller
