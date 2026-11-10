// SearchEngine.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "controller/SearchEngine.h"

#include "utils/ClinicException.h"
#include "utils/StringUtils.h"

namespace scapms::controller {

std::string toString(SearchField field) {
    switch (field) {
        case SearchField::Id: return "Patient ID";
        case SearchField::Name: return "Patient Name";
        case SearchField::Category: return "Category";
    }
    return "Patient ID";
}

std::string toString(SearchAlgorithm algorithm) {
    switch (algorithm) {
        case SearchAlgorithm::Auto: return "Auto (recommended)";
        case SearchAlgorithm::Linear: return "Linear Search";
        case SearchAlgorithm::Binary: return "Binary Search";
        case SearchAlgorithm::Recursive: return "Recursive Search";
    }
    return "Linear Search";
}

std::string SearchEngine::normalizeKeyword(SearchField field, const std::string& keyword) {
    const std::string cleaned = utils::trim(keyword);
    switch (field) {
        case SearchField::Id: return utils::toUpper(cleaned);
        case SearchField::Name: return utils::toLower(cleaned);
        case SearchField::Category: return utils::normalizeKey(cleaned);
    }
    return cleaned;
}

bool SearchEngine::matches(const model::PatientRecord& patient, SearchField field, const std::string& keyword) {
    switch (field) {
        case SearchField::Id: return patient.id() == keyword;
        case SearchField::Name: return utils::startsWith(utils::toLower(patient.name()), keyword);
        case SearchField::Category: return utils::normalizeKey(patient.category()) == keyword;
    }
    return false;
}

// ---------------------------------------------------------------- LINEAR SEARCH
// Look at every record from the first to the last until the ID is found. O(n).
int SearchEngine::linearSearchById(const std::vector<PatientPtr>& patients, const std::string& id,
                                   std::size_t& comparisons) {
    for (std::size_t i = 0; i < patients.size(); ++i) {
        ++comparisons;
        if (patients[i]->id() == id) return static_cast<int>(i);
    }
    return -1;
}

std::vector<std::size_t> SearchEngine::linearSearchByName(const std::vector<PatientPtr>& patients,
                                                          const std::string& prefixLower,
                                                          std::size_t& comparisons) {
    std::vector<std::size_t> found;
    for (std::size_t i = 0; i < patients.size(); ++i) {
        ++comparisons;
        if (utils::startsWith(utils::toLower(patients[i]->name()), prefixLower)) found.push_back(i);
    }
    return found;
}

// ---------------------------------------------------------------- BINARY SEARCH
// Repeatedly compare with the MIDDLE record and discard the half that cannot contain the ID.
// Only valid when the vector is sorted by ID. O(log n).
int SearchEngine::binarySearchById(const std::vector<PatientPtr>& patients, const std::string& id,
                                   std::size_t& comparisons) {
    int low = 0;
    int high = static_cast<int>(patients.size()) - 1;
    while (low <= high) {
        const int mid = low + (high - low) / 2;
        ++comparisons;
        const std::string& midId = patients[static_cast<std::size_t>(mid)]->id();
        if (midId == id) return mid;
        if (midId < id) low = mid + 1;   // target is in the right half
        else high = mid - 1;             // target is in the left half
    }
    return -1;
}

// Names starting with a prefix form ONE contiguous block in a name-sorted list.
// Step 1: binary search for the FIRST name that is >= prefix. Step 2: walk forward while it still matches.
std::vector<std::size_t> SearchEngine::binarySearchByName(const std::vector<PatientPtr>& patients,
                                                          const std::string& prefixLower,
                                                          std::size_t& comparisons) {
    std::size_t low = 0;
    std::size_t high = patients.size();  // search in [low, high)
    while (low < high) {
        const std::size_t mid = low + (high - low) / 2;
        ++comparisons;
        if (utils::toLower(patients[mid]->name()) < prefixLower) low = mid + 1;
        else high = mid;
    }
    std::vector<std::size_t> found;
    for (std::size_t i = low; i < patients.size(); ++i) {
        ++comparisons;
        if (!utils::startsWith(utils::toLower(patients[i]->name()), prefixLower)) break;
        found.push_back(i);
    }
    return found;
}

// ---------------------------------------------------------------- RECURSIVE SEARCH
// Base case: we ran past the last record. Recursive case: test this record, then search the rest.
void SearchEngine::recursiveSearch(const std::vector<PatientPtr>& patients, SearchField field,
                                   const std::string& keyword, std::size_t index,
                                   std::vector<std::size_t>& found, std::size_t& comparisons) {
    if (index >= patients.size()) return;  // base case
    ++comparisons;
    if (matches(*patients[index], field, keyword)) found.push_back(index);
    recursiveSearch(patients, field, keyword, index + 1, found, comparisons);  // recursive call
}

// ---------------------------------------------------------------- DISPATCHER
SearchResult SearchEngine::search(const std::vector<PatientPtr>& patients, SearchField field,
                                  const std::string& keyword, SearchAlgorithm algorithm,
                                  std::optional<SortField> sortedBy) {
    const std::string key = normalizeKeyword(field, keyword);
    if (key.empty()) throw utils::InvalidSearchException("Please enter something to search for.");
    if (key.size() > 60) throw utils::InvalidSearchException("The search text is too long (60 characters maximum).");

    const bool sortedForField =
        (field == SearchField::Id && sortedBy && *sortedBy == SortField::Id) ||
        (field == SearchField::Name && sortedBy && *sortedBy == SortField::Name);

    SearchResult result;
    if (algorithm == SearchAlgorithm::Auto) {
        // Choose by the state of the data: sorted -> binary search, otherwise a simple scan.
        algorithm = sortedForField ? SearchAlgorithm::Binary : SearchAlgorithm::Linear;
        result.explanation = sortedForField
            ? "Auto chose Binary Search because the records are sorted by " + toString(field) +
              " (O(log n) comparisons)."
            : "Auto chose Linear Search because the records are not sorted by " + toString(field) +
              " (the only safe choice for unsorted data).";
    }
    result.used = algorithm;

    switch (algorithm) {
        case SearchAlgorithm::Linear:
            if (field == SearchField::Id) {
                const int index = linearSearchById(patients, key, result.comparisons);
                if (index >= 0) result.indices.push_back(static_cast<std::size_t>(index));
            } else if (field == SearchField::Name) {
                result.indices = linearSearchByName(patients, key, result.comparisons);
            } else {
                for (std::size_t i = 0; i < patients.size(); ++i) {
                    ++result.comparisons;
                    if (matches(*patients[i], field, key)) result.indices.push_back(i);
                }
            }
            break;

        case SearchAlgorithm::Binary:
            if (field == SearchField::Category) {
                throw utils::InvalidSearchException(
                    "Binary search cannot be used for Category (many patients share a category). "
                    "Use Linear or Recursive search.");
            }
            if (!sortedForField) {
                throw utils::InvalidSearchException(
                    "Binary search needs the records to be sorted by " + toString(field) +
                    ". Sort the records by " + toString(field) + " first, or choose Linear/Auto.");
            }
            if (field == SearchField::Id) {
                const int index = binarySearchById(patients, key, result.comparisons);
                if (index >= 0) result.indices.push_back(static_cast<std::size_t>(index));
            } else {
                result.indices = binarySearchByName(patients, key, result.comparisons);
            }
            break;

        case SearchAlgorithm::Recursive:
            recursiveSearch(patients, field, key, 0, result.indices, result.comparisons);
            break;

        case SearchAlgorithm::Auto:
            break;  // already resolved above
    }

    if (result.explanation.empty()) {
        result.explanation = toString(algorithm) + " was selected manually.";
    }
    return result;
}

} // namespace scapms::controller
