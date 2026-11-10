// GeneralPatient.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "model/GeneralPatient.h"

#include "utils/ClinicException.h"

namespace scapms::model {

GeneralPatient::GeneralPatient(std::string id, std::string name, int age, std::string contact,
                               std::string complaint)
    : PatientRecord(std::move(id), std::move(name), age, std::move(contact)),
      complaint_(validateText("Complaint", complaint, 100, false)) {}

std::string GeneralPatient::category() const { return category::General; }

int GeneralPatient::priority() const { return 1; }

std::string GeneralPatient::consultationPlan() const {
    return "Standard consultation (about 15 minutes). Served in order of arrival.";
}

std::string GeneralPatient::details() const {
    return "Complaint: " + (complaint_.empty() ? std::string("not stated") : complaint_);
}

std::vector<std::string> GeneralPatient::extraValues() const { return {complaint_}; }

void GeneralPatient::setExtraValues(const std::vector<std::string>& values) {
    if (values.size() != 1) throw utils::InvalidPatientException("General patients need exactly 1 extra field.");
    complaint_ = validateText("Complaint", values[0], 100, false);
}

std::unique_ptr<PatientRecord> GeneralPatient::clone() const {
    return std::make_unique<GeneralPatient>(*this);
}

void GeneralPatient::scheduleAppointment(Appointment& appointment) {
    appointment.setTag("Normal");
    linkAppointment(appointment.id());
}

} // namespace scapms::model
