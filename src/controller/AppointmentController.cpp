// AppointmentController.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "controller/AppointmentController.h"

#include "utils/ClinicException.h"
#include "utils/StringUtils.h"

namespace scapms::controller {

using model::Appointment;
using model::AppointmentStatus;

const Appointment& AppointmentController::schedule(const std::string& patientIdText, const std::string& date,
                                                   const std::string& time, const std::string& reason) {
    const std::string patientId = utils::toUpper(utils::trim(patientIdText));
    model::PatientRecord* patient = database_.patientById(patientId);
    if (!patient) throw utils::NotFoundException("Please choose an existing patient to book.");

    // Preview the next free number; it is only committed once the appointment is stored.
    int newId = ids_.previewAppointmentId();
    for (int step = 2; database_.appointmentById(newId) != nullptr; ++step) newId = ids_.previewAppointmentId(step);

    // The Appointment constructor validates the date, time and reason.
    Appointment appointment(newId, patientId, date, time, reason);

    // One doctor, one slot: refuse to double-book the same date and time.
    for (const auto& existing : database_.appointments()) {
        if (existing.status() == AppointmentStatus::Scheduled && existing.date() == appointment.date() &&
            existing.time() == appointment.time()) {
            throw utils::InvalidAppointmentException("The slot " + appointment.date() + " " + appointment.time() +
                                                     " is already booked. Please choose another time.");
        }
    }

    // Polymorphic call: General / Emergency / FollowUp patients each apply their own rules here.
    patient->scheduleAppointment(appointment);
    database_.addAppointment(appointment);
    ids_.observeAppointmentId(appointment.id());  // commit the number
    return *database_.appointmentById(appointment.id());
}

void AppointmentController::cancel(int appointmentId) {
    Appointment* appointment = database_.appointmentById(appointmentId);
    if (!appointment) throw utils::NotFoundException("Appointment #" + std::to_string(appointmentId) + " was not found.");
    model::PatientRecord* patient = database_.patientById(appointment->patientId());
    if (!patient) throw utils::NotFoundException("The patient of this appointment no longer exists.");

    patient->cancelAppointment(*appointment);  // throws if the appointment is not 'Scheduled'
    undo_.push(std::make_unique<CancelAppointmentCommand>(appointmentId));
}

void AppointmentController::complete(int appointmentId) {
    Appointment* appointment = database_.appointmentById(appointmentId);
    if (!appointment) throw utils::NotFoundException("Appointment #" + std::to_string(appointmentId) + " was not found.");
    if (appointment->status() != AppointmentStatus::Scheduled) {
        throw utils::InvalidAppointmentException("Only scheduled appointments can be marked as completed.");
    }
    appointment->setStatus(AppointmentStatus::Completed);
}

} // namespace scapms::controller
