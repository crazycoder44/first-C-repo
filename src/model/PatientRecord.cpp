// PatientRecord.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "model/PatientRecord.h"

#include <cctype>

#include "utils/ClinicException.h"
#include "utils/StringUtils.h"

namespace scapms::model {

using utils::InvalidAgeException;
using utils::InvalidAppointmentException;
using utils::InvalidPatientException;

PatientRecord::PatientRecord(std::string id, std::string name, int age, std::string contact)
    : id_(validateId(id)),
      name_(validateName(name)),
      age_(validateAge(age)),
      contact_(validateContact(contact)) {}

void PatientRecord::setName(const std::string& name) { name_ = validateName(name); }
void PatientRecord::setAge(int age) { age_ = validateAge(age); }
void PatientRecord::setContact(const std::string& contact) { contact_ = validateContact(contact); }

std::string PatientRecord::describe() const {
    // details() is virtual -> the right subclass version runs (runtime polymorphism).
    return id_ + " | " + name_ + " | age " + std::to_string(age_) + " | " + category() + " | " + details();
}

bool PatientRecord::cancelAppointment(Appointment& appointment) {
    if (appointment.patientId() != id_) {
        throw InvalidAppointmentException("That appointment does not belong to patient " + id_ + ".");
    }
    if (appointment.status() != AppointmentStatus::Scheduled) {
        throw InvalidAppointmentException("Only scheduled appointments can be cancelled (this one is " +
                                          toString(appointment.status()) + ").");
    }
    appointment.setStatus(AppointmentStatus::Cancelled);
    return true;
}

void PatientRecord::linkAppointment(int appointmentId) {
    for (int existing : appointmentIds_) {
        if (existing == appointmentId) return;  // already linked
    }
    appointmentIds_.push_back(appointmentId);
}

std::string PatientRecord::validateId(const std::string& id) {
    const std::string cleaned = utils::toUpper(utils::trim(id));
    if (cleaned.empty()) throw InvalidPatientException("Patient ID is required.");
    if (cleaned.size() > 12) throw InvalidPatientException("Patient ID must be 12 characters or fewer.");
    for (char c : cleaned) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-') {
            throw InvalidPatientException("Patient ID may only contain letters, digits and '-'.");
        }
    }
    return cleaned;
}

std::string PatientRecord::validateName(const std::string& name) {
    const std::string cleaned = utils::trim(name);
    if (cleaned.empty()) throw InvalidPatientException("Patient name is required.");
    if (cleaned.size() > 60) throw InvalidPatientException("Patient name must be 60 characters or fewer.");
    for (char c : cleaned) {
        const unsigned char u = static_cast<unsigned char>(c);
        // Allow letters, spaces and . ' - ; bytes >= 128 are accepted so accented names work.
        if (!std::isalpha(u) && u < 128 && c != ' ' && c != '.' && c != '\'' && c != '-') {
            throw InvalidPatientException("Patient name may only contain letters, spaces and . ' -");
        }
    }
    return cleaned;
}

int PatientRecord::validateAge(int age) {
    if (age < 0 || age > 120) throw InvalidAgeException("Patient age must be between 0 and 120.");
    return age;
}

std::string PatientRecord::validateContact(const std::string& contact) {
    const std::string cleaned = utils::trim(contact);
    const InvalidPatientException error("Contact must be a valid phone number (7-15 digits) or an email address.");
    if (cleaned.empty() || cleaned.size() > 60) throw error;

    // Email: one '@' with text before it and a '.' after it, no spaces.
    const std::size_t at = cleaned.find('@');
    if (at != std::string::npos) {
        const bool noSpaces = cleaned.find(' ') == std::string::npos;
        const bool dotAfterAt = cleaned.find('.', at) != std::string::npos;
        if (at > 0 && noSpaces && dotAfterAt && cleaned.back() != '.' && cleaned.find('@', at + 1) == std::string::npos) {
            return cleaned;
        }
        throw error;
    }

    // Phone: digits plus optional + ( ) - and spaces; 7 to 15 digits in total.
    int digits = 0;
    for (std::size_t i = 0; i < cleaned.size(); ++i) {
        const char c = cleaned[i];
        if (std::isdigit(static_cast<unsigned char>(c))) {
            ++digits;
        } else if (!(c == ' ' || c == '-' || c == '(' || c == ')' || (c == '+' && i == 0))) {
            throw error;
        }
    }
    if (digits < 7 || digits > 15) throw error;
    return cleaned;
}

std::string PatientRecord::validateText(const std::string& label, const std::string& text,
                                        std::size_t maxLength, bool required) {
    const std::string cleaned = utils::trim(text);
    if (required && cleaned.empty()) throw InvalidPatientException(label + " is required.");
    if (cleaned.size() > maxLength) {
        throw InvalidPatientException(label + " must be " + std::to_string(maxLength) + " characters or fewer.");
    }
    if (utils::hasControlChars(cleaned)) throw InvalidPatientException(label + " contains invalid characters.");
    return cleaned;
}

} // namespace scapms::model
