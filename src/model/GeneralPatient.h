// GeneralPatient.h - a patient with an ordinary (non-urgent) complaint.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include "model/PatientRecord.h"

namespace scapms::model {

class GeneralPatient : public PatientRecord {
public:
    GeneralPatient(std::string id, std::string name, int age, std::string contact,
                   std::string complaint = "");

    const std::string& complaint() const { return complaint_; }

    std::string category() const override;
    int priority() const override;
    std::string consultationPlan() const override;
    std::string details() const override;
    std::vector<std::string> extraValues() const override;
    void setExtraValues(const std::vector<std::string>& values) override;
    std::unique_ptr<PatientRecord> clone() const override;

    /// General patients: standard booking, tagged "Normal".
    void scheduleAppointment(Appointment& appointment) override;

private:
    std::string complaint_;
};

} // namespace scapms::model
