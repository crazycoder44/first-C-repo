// Appointment.h - one scheduled visit for a patient.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// A patient is associated with one or more appointments (association via patientId;
// the ClinicDatabase owns the appointments, each PatientRecord remembers their IDs).
#pragma once

#include <string>
#include <utility>

namespace scapms::model {

enum class AppointmentStatus { Scheduled, Completed, Cancelled };

std::string toString(AppointmentStatus status);
/// Throws InvalidAppointmentException for unknown text.
AppointmentStatus statusFromString(const std::string& text);

class Appointment {
public:
    /// Validates date ("YYYY-MM-DD"), time ("HH:MM") and reason.
    /// Throws utils::InvalidAppointmentException on bad data.
    Appointment(int id, std::string patientId, std::string date, std::string time,
                std::string reason, AppointmentStatus status = AppointmentStatus::Scheduled,
                std::string tag = "Normal");

    int id() const { return id_; }
    const std::string& patientId() const { return patientId_; }
    const std::string& date() const { return date_; }
    const std::string& time() const { return time_; }
    const std::string& reason() const { return reason_; }
    const std::string& tag() const { return tag_; }  // "Normal", "URGENT" or "Follow-up"
    AppointmentStatus status() const { return status_; }

    void setStatus(AppointmentStatus status) { status_ = status; }
    void setTag(std::string tag) { tag_ = std::move(tag); }

    static bool isValidDate(const std::string& date);
    static bool isValidTime(const std::string& time);

private:
    int id_;
    std::string patientId_;
    std::string date_;
    std::string time_;
    std::string reason_;
    AppointmentStatus status_;
    std::string tag_;
};

} // namespace scapms::model
