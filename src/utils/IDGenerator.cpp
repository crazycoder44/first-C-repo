// IDGenerator.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "utils/IDGenerator.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>

namespace scapms::utils {

std::string IDGenerator::previewPatientId(int step) const {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "P%04d", patientCounter_ + step);
    return buffer;
}

int IDGenerator::previewAppointmentId(int step) const {
    return appointmentCounter_ + step;
}

void IDGenerator::observePatientId(const std::string& id) {
    // Only IDs of the form P<digits> influence the counter; custom IDs are ignored.
    if (id.size() < 2 || (id[0] != 'P' && id[0] != 'p')) return;
    for (std::size_t i = 1; i < id.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(id[i]))) return;
    }
    const int number = std::atoi(id.c_str() + 1);
    if (number > patientCounter_) patientCounter_ = number;
}

void IDGenerator::observeAppointmentId(int id) {
    if (id > appointmentCounter_) appointmentCounter_ = id;
}

void IDGenerator::reset() {
    patientCounter_ = 0;
    appointmentCounter_ = 0;
}

} // namespace scapms::utils
