// EmergencyPatient.h - a patient who needs urgent attention.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include "model/PatientRecord.h"

namespace scapms::model {

class EmergencyPatient : public PatientRecord {
public:
    /// severity: 1 (least serious) to 5 (life threatening).
    EmergencyPatient(std::string id, std::string name, int age, std::string contact,
                     int severity, std::string triageNote = "");

    int severity() const { return severity_; }
    const std::string& triageNote() const { return triageNote_; }

    std::string category() const override;
    int priority() const override;
    std::string consultationPlan() const override;
    std::string details() const override;
    std::vector<std::string> extraValues() const override;
    void setExtraValues(const std::vector<std::string>& values) override;
    std::unique_ptr<PatientRecord> clone() const override;

    /// Emergency patients: the appointment is tagged "URGENT".
    void scheduleAppointment(Appointment& appointment) override;

    static int parseSeverity(const std::string& text);

private:
    int severity_;
    std::string triageNote_;
};

} // namespace scapms::model
