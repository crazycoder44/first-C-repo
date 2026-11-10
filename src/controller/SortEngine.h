// SortEngine.h - sorts the patient list with a hand-written algorithm chosen by the user.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "model/PatientRecord.h"

namespace scapms::controller {

enum class SortField { Name, Id, Age, AppointmentDate };
enum class SortAlgorithm { Auto, Insertion, Merge };

std::string toString(SortField field);
std::string toString(SortAlgorithm algorithm);

/// What happened during a sort - shown in the GUI to justify the algorithm choice.
struct SortReport {
    SortAlgorithm used = SortAlgorithm::Insertion;
    std::size_t recordCount = 0;
    std::size_t comparisons = 0;
    double milliseconds = 0.0;
    std::string reason;  // why this algorithm suits the situation
};

class SortEngine {
public:
    using PatientPtr = std::unique_ptr<model::PatientRecord>;

    /// Sort `patients` in place. `earliestAppointment` maps patientId -> earliest scheduled
    /// date and is only used for SortField::AppointmentDate (patients without one go last).
    static SortReport sort(std::vector<PatientPtr>& patients, SortField field, SortAlgorithm algorithm,
                           const std::unordered_map<std::string, std::string>& earliestAppointment = {});

    /// Auto mode: chooses Insertion for small or nearly sorted data, otherwise Merge.
    static SortAlgorithm recommend(std::size_t recordCount, std::size_t outOfOrderPairs);
};

} // namespace scapms::controller
