// IDGenerator.h - produces unique patient IDs ("P0001") and appointment numbers.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// IDs are only "used up" once a record has really been stored: callers PREVIEW the next ID,
// try to create the record, and then call observe...() on success. A failed (invalid)
// registration therefore never leaves a gap in the numbering.
#pragma once

#include <string>

namespace scapms::utils {

class IDGenerator {
public:
    IDGenerator() = default;

    /// The patient ID `step` places after the highest one in use: step 1 -> "P0013" if P0012 is the highest.
    std::string previewPatientId(int step = 1) const;

    /// The appointment number `step` places after the highest one in use.
    int previewAppointmentId(int step = 1) const;

    /// Tell the generator an ID is in use (after storing a record or loading a file),
    /// so newly generated IDs never collide with it.
    void observePatientId(const std::string& id);
    void observeAppointmentId(int id);

    void reset();

private:
    int patientCounter_ = 0;      // highest P-number seen so far
    int appointmentCounter_ = 0;  // highest appointment number seen so far
};

} // namespace scapms::utils
