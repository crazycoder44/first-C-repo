// ReportGenerator.h - builds the plain-text reports and the dashboard numbers.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include <string>

#include "model/ClinicDatabase.h"

namespace scapms::controller {

/// Numbers shown on the dashboard.
struct DashboardStats {
    int totalPatients = 0;
    int generalPatients = 0;
    int emergencyPatients = 0;
    int followUpPatients = 0;
    int scheduledAppointments = 0;
    int completedAppointments = 0;
    int cancelledAppointments = 0;
    int scheduledToday = 0;
};

class ReportGenerator {
public:
    explicit ReportGenerator(const model::ClinicDatabase& database) : database_(database) {}

    DashboardStats dashboardStats(const std::string& todayIso) const;

    /// Report 1: patients ranked by number of completed appointments (highest first).
    std::string mostFrequentlyAttended() const;
    /// Report 2: patients who still have scheduled (not yet completed) appointments.
    std::string pendingAppointments() const;
    /// Report 3: how many patients are General / Emergency / Follow-Up.
    std::string categoryDistribution() const;
    /// Report 4: totals for one date ("YYYY-MM-DD"), or for all dates when `date` is empty.
    /// Throws utils::InvalidAppointmentException for a badly formatted date.
    std::string dailySummary(const std::string& date) const;

private:
    const model::ClinicDatabase& database_;
};

} // namespace scapms::controller
