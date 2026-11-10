// ReportGenerator.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "controller/ReportGenerator.h"

#include <iomanip>
#include <sstream>

#include "utils/Algorithms.h"
#include "utils/ClinicException.h"
#include "utils/StringUtils.h"

namespace scapms::controller {

using model::AppointmentStatus;

DashboardStats ReportGenerator::dashboardStats(const std::string& todayIso) const {
    DashboardStats stats;
    for (const auto& patient : database_.patients()) {
        ++stats.totalPatients;
        const std::string key = utils::normalizeKey(patient->category());
        if (key == "general") ++stats.generalPatients;
        else if (key == "emergency") ++stats.emergencyPatients;
        else ++stats.followUpPatients;
    }
    for (const auto& a : database_.appointments()) {
        switch (a.status()) {
            case AppointmentStatus::Scheduled:
                ++stats.scheduledAppointments;
                if (a.date() == todayIso) ++stats.scheduledToday;
                break;
            case AppointmentStatus::Completed: ++stats.completedAppointments; break;
            case AppointmentStatus::Cancelled: ++stats.cancelledAppointments; break;
        }
    }
    return stats;
}

namespace {

struct Row {
    std::string id;
    std::string name;
    std::string category;
    int count;
};

std::string line(char c, int n) { return std::string(static_cast<std::size_t>(n), c); }

} // namespace

std::string ReportGenerator::mostFrequentlyAttended() const {
    std::vector<Row> rows;
    for (const auto& patient : database_.patients()) {
        int completed = 0;
        for (const auto* a : database_.appointmentsOf(patient->id())) {
            if (a->status() == AppointmentStatus::Completed) ++completed;
        }
        if (completed > 0) rows.push_back({patient->id(), patient->name(), patient->category(), completed});
    }
    // Rank with our own merge sort: most completed appointments first.
    utils::mergeSort(rows, [](const Row& a, const Row& b) { return a.count > b.count; });

    std::ostringstream out;
    out << "MOST FREQUENTLY ATTENDED PATIENTS\n" << line('=', 62) << '\n';
    out << std::left << std::setw(8) << "Rank" << std::setw(10) << "ID" << std::setw(24) << "Name"
        << std::setw(12) << "Category" << "Completed\n" << line('-', 62) << '\n';
    if (rows.empty()) out << "No completed appointments yet.\n";
    int rank = 0;
    for (const auto& r : rows) {
        out << std::left << std::setw(8) << ++rank << std::setw(10) << r.id << std::setw(24) << r.name.substr(0, 22)
            << std::setw(12) << r.category << r.count << '\n';
    }
    return out.str();
}

std::string ReportGenerator::pendingAppointments() const {
    std::ostringstream out;
    out << "PATIENTS WITH PENDING APPOINTMENTS\n" << line('=', 62) << '\n';
    int patientsListed = 0;
    int totalPending = 0;
    for (const auto& patient : database_.patients()) {
        std::vector<const model::Appointment*> pending;
        for (const auto* a : database_.appointmentsOf(patient->id())) {
            if (a->status() == AppointmentStatus::Scheduled) pending.push_back(a);
        }
        if (pending.empty()) continue;
        ++patientsListed;
        out << patient->id() << "  " << patient->name() << "  [" << patient->category() << "]\n";
        for (const auto* a : pending) {
            out << "    #" << a->id() << "  " << a->date() << " " << a->time() << "  " << a->reason()
                << "  (" << a->tag() << ")\n";
            ++totalPending;
        }
    }
    if (patientsListed == 0) out << "No pending appointments.\n";
    out << line('-', 62) << '\n' << patientsListed << " patient(s), " << totalPending << " pending appointment(s).\n";
    return out.str();
}

std::string ReportGenerator::categoryDistribution() const {
    const DashboardStats stats = dashboardStats("");
    std::ostringstream out;
    out << "PATIENT CATEGORY DISTRIBUTION\n" << line('=', 40) << '\n';
    out << std::left << std::setw(24) << "General Patients" << std::right << std::setw(6) << stats.generalPatients << '\n';
    out << std::left << std::setw(24) << "Emergency Patients" << std::right << std::setw(6) << stats.emergencyPatients << '\n';
    out << std::left << std::setw(24) << "Follow-Up Patients" << std::right << std::setw(6) << stats.followUpPatients << '\n';
    out << line('-', 40) << '\n';
    out << std::left << std::setw(24) << "Total" << std::right << std::setw(6) << stats.totalPatients << '\n';
    return out.str();
}

std::string ReportGenerator::dailySummary(const std::string& dateText) const {
    const std::string date = utils::trim(dateText);
    if (!date.empty() && !model::Appointment::isValidDate(date)) {
        throw utils::InvalidAppointmentException("Invalid date '" + date + "'. Use YYYY-MM-DD or leave it empty for all dates.");
    }
    int total = 0, completed = 0, pending = 0, cancelled = 0;
    for (const auto& a : database_.appointments()) {
        if (!date.empty() && a.date() != date) continue;
        ++total;
        switch (a.status()) {
            case AppointmentStatus::Completed: ++completed; break;
            case AppointmentStatus::Scheduled: ++pending; break;
            case AppointmentStatus::Cancelled: ++cancelled; break;
        }
    }
    std::ostringstream out;
    out << "DAILY APPOINTMENT SUMMARY (" << (date.empty() ? std::string("all dates") : date) << ")\n"
        << line('=', 40) << '\n';
    out << "Total Appointments: " << total << '\n';
    out << "Completed:          " << completed << '\n';
    out << "Pending:            " << pending << '\n';
    out << "Cancelled:          " << cancelled << '\n';
    return out.str();
}

} // namespace scapms::controller
