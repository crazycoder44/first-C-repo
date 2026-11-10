// ClinicDatabase.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "model/ClinicDatabase.h"

#include "utils/ClinicException.h"

namespace scapms::model {

bool ClinicDatabase::hasPatient(const std::string& id) const {
    return patientById(id) != nullptr;
}

PatientRecord* ClinicDatabase::patientById(const std::string& id) {
    for (auto& patient : patients_) {
        if (patient->id() == id) return patient.get();
    }
    return nullptr;
}

const PatientRecord* ClinicDatabase::patientById(const std::string& id) const {
    for (const auto& patient : patients_) {
        if (patient->id() == id) return patient.get();
    }
    return nullptr;
}

void ClinicDatabase::addPatient(PatientPtr patient) {
    if (!patient) throw utils::InvalidPatientException("Cannot add an empty patient record.");
    if (hasPatient(patient->id())) {
        throw utils::DuplicateIdException("Patient ID '" + patient->id() + "' already exists.");
    }
    patients_.push_back(std::move(patient));
}

void ClinicDatabase::insertPatient(std::size_t index, PatientPtr patient) {
    if (!patient) throw utils::InvalidPatientException("Cannot add an empty patient record.");
    if (hasPatient(patient->id())) {
        throw utils::DuplicateIdException("Patient ID '" + patient->id() + "' already exists.");
    }
    if (index > patients_.size()) index = patients_.size();
    patients_.insert(patients_.begin() + static_cast<std::ptrdiff_t>(index), std::move(patient));
}

ClinicDatabase::PatientPtr ClinicDatabase::removePatient(const std::string& id, std::size_t* indexOut) {
    for (std::size_t i = 0; i < patients_.size(); ++i) {
        if (patients_[i]->id() == id) {
            PatientPtr removed = std::move(patients_[i]);
            patients_.erase(patients_.begin() + static_cast<std::ptrdiff_t>(i));
            if (indexOut) *indexOut = i;
            return removed;
        }
    }
    throw utils::NotFoundException("No patient with ID '" + id + "' was found.");
}

ClinicDatabase::PatientPtr ClinicDatabase::replacePatient(PatientPtr updated) {
    if (!updated) throw utils::InvalidPatientException("Cannot replace with an empty patient record.");
    for (auto& slot : patients_) {
        if (slot->id() == updated->id()) {
            PatientPtr old = std::move(slot);
            slot = std::move(updated);
            return old;
        }
    }
    throw utils::NotFoundException("No patient with ID '" + updated->id() + "' was found.");
}

Appointment* ClinicDatabase::appointmentById(int id) {
    for (auto& appointment : appointments_) {
        if (appointment.id() == id) return &appointment;
    }
    return nullptr;
}

const Appointment* ClinicDatabase::appointmentById(int id) const {
    for (const auto& appointment : appointments_) {
        if (appointment.id() == id) return &appointment;
    }
    return nullptr;
}

void ClinicDatabase::addAppointment(Appointment appointment) {
    if (appointmentById(appointment.id()) != nullptr) {
        throw utils::DuplicateIdException("Appointment number " + std::to_string(appointment.id()) + " already exists.");
    }
    appointments_.push_back(std::move(appointment));
}

std::vector<const Appointment*> ClinicDatabase::appointmentsOf(const std::string& patientId) const {
    std::vector<const Appointment*> result;
    for (const auto& appointment : appointments_) {
        if (appointment.patientId() == patientId) result.push_back(&appointment);
    }
    return result;
}

std::vector<Appointment> ClinicDatabase::removeAppointmentsOf(const std::string& patientId) {
    std::vector<Appointment> removed;
    std::vector<Appointment> kept;
    for (auto& appointment : appointments_) {
        if (appointment.patientId() == patientId) removed.push_back(std::move(appointment));
        else kept.push_back(std::move(appointment));
    }
    appointments_ = std::move(kept);
    return removed;
}

void ClinicDatabase::restoreAppointments(std::vector<Appointment> appointments) {
    for (auto& appointment : appointments) addAppointment(std::move(appointment));
}

void ClinicDatabase::clear() {
    patients_.clear();
    appointments_.clear();
}

void ClinicDatabase::rebuildLinks() {
    for (auto& patient : patients_) patient->clearAppointmentLinks();
    for (const auto& appointment : appointments_) {
        if (PatientRecord* owner = patientById(appointment.patientId())) {
            owner->linkAppointment(appointment.id());
        }
    }
}

} // namespace scapms::model
