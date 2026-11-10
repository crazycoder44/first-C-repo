// ClinicException.h - custom exception hierarchy for SCAPMS.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// Every error the application can raise on purpose derives from ClinicException,
// so the GUI can catch one type and show a meaningful message instead of crashing.
#pragma once

#include <stdexcept>
#include <string>

namespace scapms::utils {

/// Base class for all application-specific errors.
class ClinicException : public std::runtime_error {
public:
    explicit ClinicException(const std::string& message) : std::runtime_error(message) {}
};

/// Invalid patient information (bad name, contact, extra fields ...).
class InvalidPatientException : public ClinicException {
public:
    explicit InvalidPatientException(const std::string& message) : ClinicException(message) {}
};

/// Age outside the allowed range (0-120) or not a number.
class InvalidAgeException : public InvalidPatientException {
public:
    explicit InvalidAgeException(const std::string& message) : InvalidPatientException(message) {}
};

/// A patient ID (or appointment ID) that already exists.
class DuplicateIdException : public ClinicException {
public:
    explicit DuplicateIdException(const std::string& message) : ClinicException(message) {}
};

/// A requested patient/appointment does not exist.
class NotFoundException : public ClinicException {
public:
    explicit NotFoundException(const std::string& message) : ClinicException(message) {}
};

/// Invalid appointment data (bad date/time, double booking, wrong status ...).
class InvalidAppointmentException : public ClinicException {
public:
    explicit InvalidAppointmentException(const std::string& message) : ClinicException(message) {}
};

/// File could not be opened, written or parsed.
class FileException : public ClinicException {
public:
    explicit FileException(const std::string& message) : ClinicException(message) {}
};

/// Invalid search input or an unsuitable search algorithm for the current data.
class InvalidSearchException : public ClinicException {
public:
    explicit InvalidSearchException(const std::string& message) : ClinicException(message) {}
};

} // namespace scapms::utils
