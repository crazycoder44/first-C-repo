// UndoManager.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "controller/UndoManager.h"

#include "utils/ClinicException.h"

namespace scapms::controller {

// ---------------- DeletePatientCommand ----------------
DeletePatientCommand::DeletePatientCommand(std::size_t index, std::unique_ptr<model::PatientRecord> patient,
                                           std::vector<model::Appointment> appointments)
    : index_(index), patient_(std::move(patient)), appointments_(std::move(appointments)) {}

void DeletePatientCommand::undo(model::ClinicDatabase& database) {
    database.insertPatient(index_, std::move(patient_));
    database.restoreAppointments(std::move(appointments_));
    database.rebuildLinks();
}

std::string DeletePatientCommand::description() const {
    return "Delete patient " + patient_->id() + " (" + patient_->name() + ")";
}

// ---------------- EditPatientCommand ----------------
EditPatientCommand::EditPatientCommand(std::unique_ptr<model::PatientRecord> previous)
    : previous_(std::move(previous)) {}

void EditPatientCommand::undo(model::ClinicDatabase& database) {
    database.replacePatient(std::move(previous_));
}

std::string EditPatientCommand::description() const {
    return "Edit patient " + previous_->id() + " (" + previous_->name() + ")";
}

// ---------------- CancelAppointmentCommand ----------------
CancelAppointmentCommand::CancelAppointmentCommand(int appointmentId) : appointmentId_(appointmentId) {}

void CancelAppointmentCommand::undo(model::ClinicDatabase& database) {
    model::Appointment* appointment = database.appointmentById(appointmentId_);
    if (!appointment) throw utils::NotFoundException("The appointment no longer exists.");
    appointment->setStatus(model::AppointmentStatus::Scheduled);
}

std::string CancelAppointmentCommand::description() const {
    return "Cancel appointment #" + std::to_string(appointmentId_);
}

// ---------------- UndoManager ----------------
void UndoManager::push(std::unique_ptr<UndoCommand> command) {
    history_.push(std::move(command));
}

std::string UndoManager::nextDescription() const {
    return history_.empty() ? std::string() : history_.top()->description();
}

std::string UndoManager::undoLast(model::ClinicDatabase& database) {
    if (history_.empty()) throw utils::ClinicException("There is nothing to undo.");
    const std::string description = history_.top()->description();
    history_.top()->undo(database);  // if this throws the command stays on the stack
    history_.pop();
    return description;
}

void UndoManager::clear() {
    while (!history_.empty()) history_.pop();
}

} // namespace scapms::controller
