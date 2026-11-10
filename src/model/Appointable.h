// Appointable.h - interface for anything that can be booked for appointments.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// This is an "interface-like" abstract class: it has ONLY pure virtual functions and a
// virtual destructor. Every patient type implements it in its own way (see GeneralPatient,
// EmergencyPatient and FollowUpPatient).
#pragma once

#include <vector>

#include "model/Appointment.h"

namespace scapms::model {

class Appointable {
public:
    virtual ~Appointable() = default;

    /// Apply the patient type's booking rules to `appointment` and link it to the patient.
    /// Throws utils::InvalidAppointmentException when the rules are not satisfied.
    virtual void scheduleAppointment(Appointment& appointment) = 0;

    /// Cancel a scheduled appointment (sets its status to Cancelled).
    virtual bool cancelAppointment(Appointment& appointment) = 0;

    /// IDs of every appointment ever linked to this patient.
    virtual const std::vector<int>& appointmentIds() const = 0;
};

} // namespace scapms::model
