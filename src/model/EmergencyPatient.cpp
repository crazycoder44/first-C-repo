// EmergencyPatient.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "model/EmergencyPatient.h"

#include <stdexcept>

#include "utils/ClinicException.h"
#include "utils/StringUtils.h"

namespace scapms::model {

int EmergencyPatient::parseSeverity(const std::string& text) {
    const std::string cleaned = utils::trim(text);
    int value = 0;
    try {
        std::size_t used = 0;
        value = std::stoi(cleaned, &used);
        if (used != cleaned.size()) throw std::invalid_argument("trailing characters");
    } catch (const std::exception&) {
        throw utils::InvalidPatientException("Severity must be a whole number from 1 to 5.");
    }
    if (value < 1 || value > 5) throw utils::InvalidPatientException("Severity must be a whole number from 1 to 5.");
    return value;
}

EmergencyPatient::EmergencyPatient(std::string id, std::string name, int age, std::string contact,
                                   int severity, std::string triageNote)
    : PatientRecord(std::move(id), std::move(name), age, std::move(contact)),
      severity_(parseSeverity(std::to_string(severity))),
      triageNote_(validateText("Triage note", triageNote, 100, false)) {}

std::string EmergencyPatient::category() const { return category::Emergency; }

// Emergency patients always outrank other types; a higher severity ranks higher still.
int EmergencyPatient::priority() const { return 10 + severity_; }

std::string EmergencyPatient::consultationPlan() const {
    return "IMMEDIATE triage (severity " + std::to_string(severity_) +
           "/5). Skips the normal queue and is seen first.";
}

std::string EmergencyPatient::details() const {
    return "Severity " + std::to_string(severity_) + "/5" +
           (triageNote_.empty() ? std::string() : ", note: " + triageNote_);
}

std::vector<std::string> EmergencyPatient::extraValues() const {
    return {std::to_string(severity_), triageNote_};
}

void EmergencyPatient::setExtraValues(const std::vector<std::string>& values) {
    if (values.size() != 2) throw utils::InvalidPatientException("Emergency patients need exactly 2 extra fields.");
    const int severity = parseSeverity(values[0]);                       // validate first ...
    const std::string note = validateText("Triage note", values[1], 100, false);
    severity_ = severity;                                                // ... then change state
    triageNote_ = note;
}

std::unique_ptr<PatientRecord> EmergencyPatient::clone() const {
    return std::make_unique<EmergencyPatient>(*this);
}

void EmergencyPatient::scheduleAppointment(Appointment& appointment) {
    appointment.setTag("URGENT");
    linkAppointment(appointment.id());
}

} // namespace scapms::model
