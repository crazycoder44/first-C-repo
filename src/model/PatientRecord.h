// PatientRecord.h - ABSTRACT base class for every kind of patient.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// Common data (ID, name, age, contact) lives here. Behaviour that differs between patient
// types is declared as PURE VIRTUAL functions, so PatientRecord itself can never be
// instantiated and every concrete subclass must provide its own version.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "model/Appointable.h"

namespace scapms::model {

/// Category names used throughout the program (GUI, files, reports).
namespace category {
inline constexpr const char* General = "General";
inline constexpr const char* Emergency = "Emergency";
inline constexpr const char* FollowUp = "Follow-Up";
} // namespace category

class PatientRecord : public Appointable {
public:
    /// Validates every field; throws utils::InvalidPatientException / InvalidAgeException.
    PatientRecord(std::string id, std::string name, int age, std::string contact);
    ~PatientRecord() override = default;

    PatientRecord& operator=(const PatientRecord&) = delete;

    // ---- Encapsulated common data (getters + validating setters) ----
    const std::string& id() const { return id_; }
    const std::string& name() const { return name_; }
    int age() const { return age_; }
    const std::string& contact() const { return contact_; }

    void setName(const std::string& name);
    void setAge(int age);
    void setContact(const std::string& contact);

    // ---- PURE VIRTUAL: every patient type must define these ----
    /// "General", "Emergency" or "Follow-Up".
    virtual std::string category() const = 0;
    /// Higher number = more urgent.
    virtual int priority() const = 0;
    /// What the clinic should do when this patient is called in (runtime polymorphism demo).
    virtual std::string consultationPlan() const = 0;
    /// Short description of the type-specific fields.
    virtual std::string details() const = 0;
    /// Type-specific fields as text (used by the GUI, the file format and the factory).
    virtual std::vector<std::string> extraValues() const = 0;
    virtual void setExtraValues(const std::vector<std::string>& values) = 0;
    /// "Virtual copy constructor": copy a patient through a base-class pointer.
    virtual std::unique_ptr<PatientRecord> clone() const = 0;

    /// One-line summary built from the common data plus details() (which is virtual).
    std::string describe() const;

    // ---- Appointable (partly implemented here; scheduleAppointment is per type) ----
    bool cancelAppointment(Appointment& appointment) override;
    const std::vector<int>& appointmentIds() const override { return appointmentIds_; }

    /// Bookkeeping used by the database (e.g. when rebuilding links after loading a file).
    void linkAppointment(int appointmentId);
    void clearAppointmentLinks() { appointmentIds_.clear(); }

    // ---- Validation helpers (each returns the cleaned value or throws) ----
    static std::string validateId(const std::string& id);
    static std::string validateName(const std::string& name);
    static int validateAge(int age);
    static std::string validateContact(const std::string& contact);
    /// Free text field: trims, rejects control characters and enforces a maximum length.
    static std::string validateText(const std::string& label, const std::string& text,
                                    std::size_t maxLength, bool required);

protected:
    PatientRecord(const PatientRecord&) = default;

private:
    std::string id_;
    std::string name_;
    int age_;
    std::string contact_;
    std::vector<int> appointmentIds_;
};

} // namespace scapms::model
