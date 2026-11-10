// SortEngine.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "controller/SortEngine.h"

#include <chrono>
#include <functional>

#include "utils/Algorithms.h"
#include "utils/StringUtils.h"

namespace scapms::controller {

namespace {

constexpr std::size_t kSmallDataLimit = 20;  // up to this many records insertion sort is preferred

// Builds the "a goes before b" rule for the chosen field.
std::function<bool(const std::unique_ptr<model::PatientRecord>&, const std::unique_ptr<model::PatientRecord>&)>
makeLess(SortField field, const std::unordered_map<std::string, std::string>& earliest) {
    using Ptr = std::unique_ptr<model::PatientRecord>;
    switch (field) {
        case SortField::Id:
            return [](const Ptr& a, const Ptr& b) { return a->id() < b->id(); };
        case SortField::Age:
            return [](const Ptr& a, const Ptr& b) { return a->age() < b->age(); };
        case SortField::AppointmentDate:
            return [&earliest](const Ptr& a, const Ptr& b) {
                const auto ia = earliest.find(a->id());
                const auto ib = earliest.find(b->id());
                const std::string da = ia == earliest.end() ? "9999-99-99" : ia->second;  // none -> last
                const std::string db = ib == earliest.end() ? "9999-99-99" : ib->second;
                return da < db;
            };
        case SortField::Name:
        default:
            // Case-insensitive, so "adaeze" and "Adaeze" are treated alike. The binary search on
            // names relies on exactly this ordering.
            return [](const Ptr& a, const Ptr& b) { return utils::toLower(a->name()) < utils::toLower(b->name()); };
    }
}

} // namespace

std::string toString(SortField field) {
    switch (field) {
        case SortField::Name: return "Patient Name";
        case SortField::Id: return "Patient ID";
        case SortField::Age: return "Age";
        case SortField::AppointmentDate: return "Appointment Date";
    }
    return "Patient Name";
}

std::string toString(SortAlgorithm algorithm) {
    switch (algorithm) {
        case SortAlgorithm::Auto: return "Auto (recommended)";
        case SortAlgorithm::Insertion: return "Insertion Sort";
        case SortAlgorithm::Merge: return "Merge Sort";
    }
    return "Insertion Sort";
}

SortAlgorithm SortEngine::recommend(std::size_t recordCount, std::size_t outOfOrderPairs) {
    // Insertion sort is best for small lists, or lists where fewer than ~10% of neighbours are out of order.
    if (recordCount <= kSmallDataLimit || outOfOrderPairs * 10 <= recordCount) return SortAlgorithm::Insertion;
    return SortAlgorithm::Merge;
}

SortReport SortEngine::sort(std::vector<PatientPtr>& patients, SortField field, SortAlgorithm algorithm,
                            const std::unordered_map<std::string, std::string>& earliestAppointment) {
    SortReport report;
    report.recordCount = patients.size();

    const auto baseLess = makeLess(field, earliestAppointment);

    // Count how many neighbouring pairs are out of order (tells us how "nearly sorted" the data is).
    std::size_t outOfOrder = 0;
    for (std::size_t i = 1; i < patients.size(); ++i) {
        if (baseLess(patients[i], patients[i - 1])) ++outOfOrder;
    }

    if (algorithm == SortAlgorithm::Auto) algorithm = recommend(patients.size(), outOfOrder);
    report.used = algorithm;

    // Wrap the comparison so we can count how many comparisons the algorithm needed.
    std::size_t comparisons = 0;
    auto countingLess = [&](const PatientPtr& a, const PatientPtr& b) {
        ++comparisons;
        return baseLess(a, b);
    };

    const auto start = std::chrono::steady_clock::now();
    if (algorithm == SortAlgorithm::Insertion) utils::insertionSort(patients, countingLess);
    else utils::mergeSort(patients, countingLess);
    const auto stop = std::chrono::steady_clock::now();

    report.comparisons = comparisons;
    report.milliseconds = std::chrono::duration<double, std::milli>(stop - start).count();

    const std::string n = std::to_string(patients.size());
    if (algorithm == SortAlgorithm::Insertion) {
        report.reason = "Insertion Sort suits this situation: with " + n +
                        " record(s) the list is small" +
                        (outOfOrder * 10 <= patients.size() && patients.size() > kSmallDataLimit
                             ? " and already nearly sorted"
                             : "") +
                        ", and insertion sort has almost no overhead, needs no extra memory, is stable "
                        "and runs close to O(n) on ordered data.";
        if (patients.size() > kSmallDataLimit && outOfOrder * 10 > patients.size()) {
            report.reason += " Note: with this many unordered records Merge Sort (O(n log n)) would be faster.";
        }
    } else {
        report.reason = "Merge Sort suits this situation: with " + n +
                        " record(s) the list is larger and not nearly sorted, so the O(n log n) divide-and-conquer "
                        "approach beats insertion sort's O(n^2). It is also stable, so patients with equal keys "
                        "keep their previous order.";
        if (patients.size() <= kSmallDataLimit) {
            report.reason += " Note: for such a small list Insertion Sort would have had less overhead.";
        }
    }
    return report;
}

} // namespace scapms::controller
