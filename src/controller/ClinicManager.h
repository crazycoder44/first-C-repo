// ClinicManager.h - the application's "brain": all clinic operations go through here.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// The GUI never touches the database directly for changes; it calls ClinicManager, which
//   * validates input and throws meaningful exceptions,
//   * records Undo information on a std::stack,
//   * keeps the waiting lists in std::queue objects,
//   * keeps the fixed-size recent-patient cache consistent,
//   * saves/loads the data files.
#pragma once

#include <ctime>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <vector>

#include "controller/AppointmentController.h"
#include "controller/RecentPatientCache.h"
#include "controller/ReportGenerator.h"
#include "controller/SearchEngine.h"
#include "controller/SortEngine.h"
#include "controller/UndoManager.h"
#include "model/ClinicDatabase.h"
#include "utils/IDGenerator.h"

namespace scapms::controller {

/// Everything the registration form can supply, still as text (validated by the manager).
struct PatientInput {
    std::string category;
    std::string id;       // empty -> generate automatically
    std::string name;
    std::string ageText;
    std::string contact;
    std::vector<std::string> extras;
};

/// One patient in the waiting queue.
struct QueueEntry {
    std::string patientId;
    std::time_t joinedAt = 0;
};

class ClinicManager {
public:
    ClinicManager();

    // ---------------- patients ----------------
    /// Registers a patient and returns the (possibly generated) ID.
    std::string registerPatient(const PatientInput& input);
    /// Edits name/age/contact/extras (ID and category stay fixed). Can be undone.
    void editPatient(const std::string& id, const PatientInput& input);
    /// Deletes a patient and their appointments. Can be undone.
    void deletePatient(const std::string& id);

    /// Looks a patient up by ID, using the cache first and a linear search second.
    const model::PatientRecord* findPatient(const std::string& id);

    const std::vector<std::unique_ptr<model::PatientRecord>>& patients() const { return database_.patients(); }
    const model::ClinicDatabase& database() const { return database_; }

    /// Runtime polymorphism demo: works on ANY PatientRecord through a base-class reference.
    std::string processPatient(const model::PatientRecord& patient) const;

    // ---------------- appointments ----------------
    AppointmentController& appointments() { return appointmentController_; }

    // ---------------- search & sort ----------------
    SearchResult search(SearchField field, const std::string& keyword, SearchAlgorithm algorithm) const;
    SortReport sortPatients(SortField field, SortAlgorithm algorithm);
    std::optional<SortField> sortedBy() const { return sortedBy_; }

    // ---------------- waiting queue ----------------
    void addToQueue(const std::string& patientId);
    /// Removes and returns the next patient to be served (Emergency patients first, then first-come-first-served).
    QueueEntry callNext();
    /// The queue in serving order (copy; std::queue cannot be iterated).
    std::vector<QueueEntry> queueSnapshot() const;
    std::size_t queueSize() const { return urgentQueue_.size() + normalQueue_.size(); }

    // ---------------- undo ----------------
    bool canUndo() const { return undo_.canUndo(); }
    std::size_t undoDepth() const { return undo_.size(); }
    std::string nextUndoDescription() const { return undo_.nextDescription(); }
    /// Reverses the last administrative operation; returns what was undone.
    std::string undoLast();

    // ---------------- cache ----------------
    const RecentPatientCache& cache() const { return cache_; }

    // ---------------- reports ----------------
    ReportGenerator reports() const { return ReportGenerator(database_); }

    // ---------------- persistence ----------------
    void save(const std::string& path) const;
    void load(const std::string& path);
    /// Empties everything (database, queues, undo history, cache).
    void clearAll();

private:
    void resetRuntimeState();
    void removeFromQueues(const std::string& patientId);
    static int parseAge(const std::string& text);
    std::unordered_map<std::string, std::string> earliestAppointments() const;

    model::ClinicDatabase database_;
    utils::IDGenerator ids_;
    UndoManager undo_;
    RecentPatientCache cache_;
    AppointmentController appointmentController_;
    std::optional<SortField> sortedBy_;

    // Two FIFO queues: emergencies are always called before normal patients,
    // and inside each queue the first patient to join is the first to be served.
    std::queue<QueueEntry> urgentQueue_;
    std::queue<QueueEntry> normalQueue_;
};

} // namespace scapms::controller
