// FollowUpPatient.h - a returning patient who is being reviewed after an earlier visit.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include "model/PatientRecord.h"

namespace scapms::model {

class FollowUpPatient : public PatientRecord {
public:
    /// previousVisitDate must be a valid "YYYY-MM-DD" date.
    FollowUpPatient(std::string id, std::string name, int age, std::string contact,
                    std::string previousVisitDate, std::string doctor = "");

    const std::string& previousVisitDate() const { return previousVisitDate_; }
    const std::string& doctor() const { return doctor_; }

    std::string category() const override;
    int priority() const override;
    std::string consultationPlan() const override;
    std::string details() const override;
    std::vector<std::string> extraValues() const override;
    void setExtraValues(const std::vector<std::string>& values) override;
    std::unique_ptr<PatientRecord> clone() const override;

    /// Follow-up patients: the appointment cannot be earlier than the previous visit.
    void scheduleAppointment(Appointment& appointment) override;

private:
    static std::string validateVisitDate(const std::string& date);

    std::string previousVisitDate_;
    std::string doctor_;
};

} // namespace scapms::model
