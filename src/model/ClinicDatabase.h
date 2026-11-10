// ClinicDatabase.h - owns every patient record and every appointment.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// COMPOSITION: the database *owns* its patients (std::vector<std::unique_ptr<PatientRecord>>)
// and its appointments. When the database is destroyed, everything it owns is destroyed too.
// The vector holds base-class pointers, so GeneralPatient, EmergencyPatient and
// FollowUpPatient objects live side by side and are used polymorphically.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "model/Appointment.h"
#include "model/PatientRecord.h"

namespace scapms::model {

class ClinicDatabase {
public:
    using PatientPtr = std::unique_ptr<PatientRecord>;

    ClinicDatabase() = default;
    ClinicDatabase(ClinicDatabase&&) = default;
    ClinicDatabase& operator=(ClinicDatabase&&) = default;
    ClinicDatabase(const ClinicDatabase&) = delete;
    ClinicDatabase& operator=(const ClinicDatabase&) = delete;

    // ---------------- patients ----------------
    const std::vector<PatientPtr>& patients() const { return patients_; }
    /// Direct access to the storage so a sort algorithm can reorder the records in place.
    std::vector<PatientPtr>& mutablePatients() { return patients_; }
    std::size_t patientCount() const { return patients_.size(); }

    bool hasPatient(const std::string& id) const;
    PatientRecord* patientById(const std::string& id);
    const PatientRecord* patientById(const std::string& id) const;

    /// Throws utils::DuplicateIdException if the ID is already used.
    void addPatient(PatientPtr patient);
    /// Insert at a given position (used by Undo to restore the original order).
    void insertPatient(std::size_t index, PatientPtr patient);
    /// Removes and returns the patient. Throws utils::NotFoundException.
    PatientPtr removePatient(const std::string& id, std::size_t* indexOut = nullptr);
    /// Swap in an updated record with the same ID; returns the old one.
    PatientPtr replacePatient(PatientPtr updated);

    // ---------------- appointments ----------------
    const std::vector<Appointment>& appointments() const { return appointments_; }
    Appointment* appointmentById(int id);
    const Appointment* appointmentById(int id) const;
    void addAppointment(Appointment appointment);
    std::vector<const Appointment*> appointmentsOf(const std::string& patientId) const;

    /// Removes every appointment of a patient and returns them (so Undo can restore them).
    std::vector<Appointment> removeAppointmentsOf(const std::string& patientId);
    void restoreAppointments(std::vector<Appointment> appointments);

    // ---------------- whole database ----------------
    void clear();
    /// Re-create the patient -> appointment links from the appointment list (after loading).
    void rebuildLinks();

private:
    std::vector<PatientPtr> patients_;
    std::vector<Appointment> appointments_;
};

} // namespace scapms::model
