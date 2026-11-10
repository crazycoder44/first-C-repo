// Appointment.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "model/Appointment.h"

#include <cctype>

#include "utils/ClinicException.h"
#include "utils/StringUtils.h"

namespace scapms::model {

std::string toString(AppointmentStatus status) {
    switch (status) {
        case AppointmentStatus::Scheduled: return "Scheduled";
        case AppointmentStatus::Completed: return "Completed";
        case AppointmentStatus::Cancelled: return "Cancelled";
    }
    return "Scheduled";
}

AppointmentStatus statusFromString(const std::string& text) {
    const std::string key = utils::toLower(utils::trim(text));
    if (key == "scheduled") return AppointmentStatus::Scheduled;
    if (key == "completed") return AppointmentStatus::Completed;
    if (key == "cancelled") return AppointmentStatus::Cancelled;
    throw utils::InvalidAppointmentException("Unknown appointment status: '" + text + "'.");
}

static bool allDigits(const std::string& text, std::size_t from, std::size_t count) {
    if (from + count > text.size()) return false;
    for (std::size_t i = from; i < from + count; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) return false;
    }
    return true;
}

bool Appointment::isValidDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    if (!allDigits(date, 0, 4) || !allDigits(date, 5, 2) || !allDigits(date, 8, 2)) return false;

    const int year = std::stoi(date.substr(0, 4));
    const int month = std::stoi(date.substr(5, 2));
    const int day = std::stoi(date.substr(8, 2));
    if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1) return false;

    static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = daysInMonth[month - 1];
    const bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (month == 2 && leap) maxDay = 29;
    return day <= maxDay;
}

bool Appointment::isValidTime(const std::string& time) {
    if (time.size() != 5 || time[2] != ':') return false;
    if (!allDigits(time, 0, 2) || !allDigits(time, 3, 2)) return false;
    const int hour = std::stoi(time.substr(0, 2));
    const int minute = std::stoi(time.substr(3, 2));
    return hour <= 23 && minute <= 59;
}

Appointment::Appointment(int id, std::string patientId, std::string date, std::string time,
                         std::string reason, AppointmentStatus status, std::string tag)
    : id_(id),
      patientId_(utils::toUpper(utils::trim(patientId))),
      date_(utils::trim(date)),
      time_(utils::trim(time)),
      reason_(utils::trim(reason)),
      status_(status),
      tag_(std::move(tag)) {
    if (id_ <= 0) throw utils::InvalidAppointmentException("Appointment number must be positive.");
    if (patientId_.empty()) throw utils::InvalidAppointmentException("An appointment needs a patient.");
    if (!isValidDate(date_)) {
        throw utils::InvalidAppointmentException("Invalid appointment date '" + date_ +
                                                 "'. Use the format YYYY-MM-DD (e.g. 2026-11-20).");
    }
    if (!isValidTime(time_)) {
        throw utils::InvalidAppointmentException("Invalid appointment time '" + time_ +
                                                 "'. Use the 24-hour format HH:MM (e.g. 14:30).");
    }
    if (reason_.empty()) throw utils::InvalidAppointmentException("Please enter a reason for the appointment.");
    if (reason_.size() > 100) throw utils::InvalidAppointmentException("The reason must be 100 characters or fewer.");
    if (utils::hasControlChars(reason_)) throw utils::InvalidAppointmentException("The reason contains invalid characters.");
}

} // namespace scapms::model
