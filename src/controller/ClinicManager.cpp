// ClinicManager.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "controller/ClinicManager.h"

#include <stdexcept>

#include "model/PatientFactory.h"
#include "utils/ClinicException.h"
#include "utils/FileHandler.h"
#include "utils/StringUtils.h"

namespace scapms::controller {

using utils::ClinicException;
using utils::NotFoundException;

ClinicManager::ClinicManager() : appointmentController_(database_, undo_, ids_) {}

int ClinicManager::parseAge(const std::string& text) {
    const std::string cleaned = utils::trim(text);
    if (cleaned.empty()) throw utils::InvalidAgeException("Patient age is required.");
    try {
        std::size_t used = 0;
        const int age = std::stoi(cleaned, &used);
        if (used != cleaned.size()) throw std::invalid_argument("not a whole number");
        return age;
    } catch (const std::out_of_range&) {
        throw utils::InvalidAgeException("Patient age must be between 0 and 120.");
    } catch (const std::invalid_argument&) {
        throw utils::InvalidAgeException("Patient age must be a whole number between 0 and 120.");
    }
}

// ---------------------------------------------------------------- patients
std::string ClinicManager::registerPatient(const PatientInput& input) {
    const int age = parseAge(input.ageText);

    std::string id = utils::trim(input.id);
    if (id.empty()) {
        // Preview (do not consume) the next free ID; it is only committed once the patient is stored.
        for (int step = 1; database_.hasPatient(id = ids_.previewPatientId(step)); ++step) {
        }
    }

    // The factory builds the right subclass; the constructors validate every field.
    auto patient = model::PatientFactory::create(input.category, id, input.name, age, input.contact, input.extras);
    const std::string finalId = patient->id();
    database_.addPatient(std::move(patient));  // throws DuplicateIdException for an existing ID
    ids_.observePatientId(finalId);
    sortedBy_.reset();                         // a new record may break the sorted order
    return finalId;
}

void ClinicManager::editPatient(const std::string& id, const PatientInput& input) {
    model::PatientRecord* current = database_.patientById(id);
    if (!current) throw NotFoundException("No patient with ID '" + id + "' was found.");

    // Work on a copy: if any setter throws, the original record is untouched.
    std::unique_ptr<model::PatientRecord> updated = current->clone();
    updated->setName(input.name);
    updated->setAge(parseAge(input.ageText));
    updated->setContact(input.contact);
    updated->setExtraValues(input.extras);

    cache_.invalidate(id);  // the old object is about to be replaced
    std::unique_ptr<model::PatientRecord> previous = database_.replacePatient(std::move(updated));
    undo_.push(std::make_unique<EditPatientCommand>(std::move(previous)));
    sortedBy_.reset();
}

void ClinicManager::deletePatient(const std::string& id) {
    std::size_t index = 0;
    cache_.invalidate(id);  // never leave a dangling pointer in the cache
    std::unique_ptr<model::PatientRecord> removed = database_.removePatient(id, &index);
    std::vector<model::Appointment> removedAppointments = database_.removeAppointmentsOf(id);
    removeFromQueues(id);
    undo_.push(std::make_unique<DeletePatientCommand>(index, std::move(removed), std::move(removedAppointments)));
}

const model::PatientRecord* ClinicManager::findPatient(const std::string& idText) {
    const std::string id = utils::toUpper(utils::trim(idText));
    if (const model::PatientRecord* cached = cache_.find(id)) return cached;  // fast path: the array cache

    std::size_t comparisons = 0;
    const int index = SearchEngine::linearSearchById(database_.patients(), id, comparisons);
    if (index < 0) return nullptr;
    const model::PatientRecord* patient = database_.patients()[static_cast<std::size_t>(index)].get();
    cache_.put(patient);
    return patient;
}

std::string ClinicManager::processPatient(const model::PatientRecord& patient) const {
    // Every call below is a virtual call resolved at run time from the real object type.
    return patient.describe() + "\nPriority: " + std::to_string(patient.priority()) +
           "\nPlan: " + patient.consultationPlan();
}

// ---------------------------------------------------------------- search & sort
SearchResult ClinicManager::search(SearchField field, const std::string& keyword, SearchAlgorithm algorithm) const {
    return SearchEngine::search(database_.patients(), field, keyword, algorithm, sortedBy_);
}

std::unordered_map<std::string, std::string> ClinicManager::earliestAppointments() const {
    std::unordered_map<std::string, std::string> earliest;
    for (const auto& a : database_.appointments()) {
        if (a.status() != model::AppointmentStatus::Scheduled) continue;
        auto it = earliest.find(a.patientId());
        if (it == earliest.end() || a.date() < it->second) earliest[a.patientId()] = a.date();
    }
    return earliest;
}

SortReport ClinicManager::sortPatients(SortField field, SortAlgorithm algorithm) {
    SortReport report = SortEngine::sort(database_.mutablePatients(), field, algorithm, earliestAppointments());
    sortedBy_ = field;
    return report;
}

// ---------------------------------------------------------------- waiting queue
void ClinicManager::addToQueue(const std::string& patientIdText) {
    const std::string id = utils::toUpper(utils::trim(patientIdText));
    const model::PatientRecord* patient = findPatient(id);
    if (!patient) throw NotFoundException("Please choose an existing patient to add to the queue.");

    for (const QueueEntry& entry : queueSnapshot()) {
        if (entry.patientId == id) throw ClinicException(patient->name() + " is already in the waiting queue.");
    }

    QueueEntry entry;
    entry.patientId = id;
    entry.joinedAt = std::time(nullptr);
    if (patient->priority() >= 10) urgentQueue_.push(entry);  // emergencies
    else normalQueue_.push(entry);
}

QueueEntry ClinicManager::callNext() {
    std::queue<QueueEntry>& source = urgentQueue_.empty() ? normalQueue_ : urgentQueue_;
    if (source.empty()) throw ClinicException("The waiting queue is empty.");
    QueueEntry next = source.front();  // FIFO: the one who joined first ...
    source.pop();                      // ... is served first
    return next;
}

std::vector<QueueEntry> ClinicManager::queueSnapshot() const {
    std::vector<QueueEntry> entries;
    std::queue<QueueEntry> urgentCopy = urgentQueue_;  // copy, then pop the copy
    while (!urgentCopy.empty()) { entries.push_back(urgentCopy.front()); urgentCopy.pop(); }
    std::queue<QueueEntry> normalCopy = normalQueue_;
    while (!normalCopy.empty()) { entries.push_back(normalCopy.front()); normalCopy.pop(); }
    return entries;
}

void ClinicManager::removeFromQueues(const std::string& patientId) {
    for (std::queue<QueueEntry>* q : {&urgentQueue_, &normalQueue_}) {
        std::queue<QueueEntry> kept;
        while (!q->empty()) {
            if (q->front().patientId != patientId) kept.push(q->front());
            q->pop();
        }
        *q = std::move(kept);
    }
}

// ---------------------------------------------------------------- undo
std::string ClinicManager::undoLast() {
    const std::string description = undo_.undoLast(database_);
    cache_.clear();       // records may have been replaced or re-inserted
    sortedBy_.reset();
    return description;
}

// ---------------------------------------------------------------- persistence
void ClinicManager::save(const std::string& path) const {
    utils::FileHandler::save(database_, path);
}

void ClinicManager::load(const std::string& path) {
    model::ClinicDatabase loaded = utils::FileHandler::load(path);  // throws before we touch current data
    resetRuntimeState();
    database_ = std::move(loaded);
    for (const auto& patient : database_.patients()) ids_.observePatientId(patient->id());
    for (const auto& appointment : database_.appointments()) ids_.observeAppointmentId(appointment.id());
}

void ClinicManager::clearAll() {
    resetRuntimeState();
    database_.clear();
}

void ClinicManager::resetRuntimeState() {
    cache_.clear();
    undo_.clear();
    urgentQueue_ = std::queue<QueueEntry>();
    normalQueue_ = std::queue<QueueEntry>();
    ids_.reset();
    sortedBy_.reset();
}

} // namespace scapms::controller
