// AppointmentController.h - business rules for scheduling, cancelling and completing appointments.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include <string>

#include "controller/UndoManager.h"
#include "model/ClinicDatabase.h"
#include "utils/IDGenerator.h"

namespace scapms::controller {

class AppointmentController {
public:
    AppointmentController(model::ClinicDatabase& database, UndoManager& undo, utils::IDGenerator& ids)
        : database_(database), undo_(undo), ids_(ids) {}

    /// Creates an appointment. The patient's own booking rules (Appointable) are applied.
    /// Throws InvalidAppointmentException / NotFoundException.
    const model::Appointment& schedule(const std::string& patientId, const std::string& date,
                                       const std::string& time, const std::string& reason);

    /// Cancels a scheduled appointment; the operation can be undone.
    void cancel(int appointmentId);

    /// Marks a scheduled appointment as completed (the patient attended).
    void complete(int appointmentId);

private:
    model::ClinicDatabase& database_;
    UndoManager& undo_;
    utils::IDGenerator& ids_;
};

} // namespace scapms::controller
