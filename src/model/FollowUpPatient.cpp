// FollowUpPatient.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "model/FollowUpPatient.h"

#include "utils/ClinicException.h"
#include "utils/StringUtils.h"

namespace scapms::model {

std::string FollowUpPatient::validateVisitDate(const std::string& date) {
    const std::string cleaned = utils::trim(date);
    if (!Appointment::isValidDate(cleaned)) {
        throw utils::InvalidPatientException("Previous visit date must be a valid date in the form YYYY-MM-DD.");
    }
    return cleaned;
}

FollowUpPatient::FollowUpPatient(std::string id, std::string name, int age, std::string contact,
                                 std::string previousVisitDate, std::string doctor)
    : PatientRecord(std::move(id), std::move(name), age, std::move(contact)),
      previousVisitDate_(validateVisitDate(previousVisitDate)),
      doctor_(validateText("Doctor", doctor, 60, false)) {}

std::string FollowUpPatient::category() const { return category::FollowUp; }

int FollowUpPatient::priority() const { return 5; }

std::string FollowUpPatient::consultationPlan() const {
    return "Follow-up review. Retrieve notes from the visit on " + previousVisitDate_ +
           (doctor_.empty() ? std::string(".") : " (Dr. " + doctor_ + ").");
}

std::string FollowUpPatient::details() const {
    return "Last visit " + previousVisitDate_ + (doctor_.empty() ? std::string() : ", Dr. " + doctor_);
}

std::vector<std::string> FollowUpPatient::extraValues() const { return {previousVisitDate_, doctor_}; }

void FollowUpPatient::setExtraValues(const std::vector<std::string>& values) {
    if (values.size() != 2) throw utils::InvalidPatientException("Follow-up patients need exactly 2 extra fields.");
    const std::string date = validateVisitDate(values[0]);
    const std::string doctor = validateText("Doctor", values[1], 60, false);
    previousVisitDate_ = date;
    doctor_ = doctor;
}

std::unique_ptr<PatientRecord> FollowUpPatient::clone() const {
    return std::make_unique<FollowUpPatient>(*this);
}

void FollowUpPatient::scheduleAppointment(Appointment& appointment) {
    // ISO dates ("YYYY-MM-DD") compare correctly as plain strings.
    if (appointment.date() < previousVisitDate_) {
        throw utils::InvalidAppointmentException("A follow-up appointment cannot be before the previous visit (" +
                                                 previousVisitDate_ + ").");
    }
    appointment.setTag("Follow-up");
    linkAppointment(appointment.id());
}

} // namespace scapms::model
