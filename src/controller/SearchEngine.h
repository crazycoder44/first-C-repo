// SearchEngine.h - hand-written searching algorithms (no std::find / std::binary_search).
// Author: Linda Obodo (COS202 Lab Assessment)
//
//   * Linear search     - works on ANY data, checks records one by one.
//   * Binary search     - needs data SORTED by the searched field; halves the range each step.
//   * Recursive search  - the same scan as linear search, written as a function that calls itself.
//
// Matching rules (the same for all algorithms so their results agree):
//   ID       : exact match, not case sensitive
//   Name     : the patient's name STARTS WITH the keyword, not case sensitive
//   Category : exact category, ignoring case, spaces and hyphens ("follow up" == "Follow-Up")
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "controller/SortEngine.h"
#include "model/PatientRecord.h"

namespace scapms::controller {

enum class SearchField { Id, Name, Category };
enum class SearchAlgorithm { Auto, Linear, Binary, Recursive };

std::string toString(SearchField field);
std::string toString(SearchAlgorithm algorithm);

struct SearchResult {
    std::vector<std::size_t> indices;  // positions in the patient vector (ascending order)
    SearchAlgorithm used = SearchAlgorithm::Linear;
    std::size_t comparisons = 0;
    std::string explanation;           // why this algorithm was used / what it did
};

class SearchEngine {
public:
    using PatientPtr = std::unique_ptr<model::PatientRecord>;

    /// Main entry point used by the GUI.
    /// `sortedBy` says how the vector is currently ordered (empty = unsorted); Auto mode and
    /// Binary search rely on it. Throws utils::InvalidSearchException for an empty keyword or an
    /// unsuitable algorithm (e.g. Binary search on unsorted data).
    static SearchResult search(const std::vector<PatientPtr>& patients, SearchField field,
                               const std::string& keyword, SearchAlgorithm algorithm,
                               std::optional<SortField> sortedBy);

    // ---- The individual algorithms (public so they can be demonstrated/tested directly) ----

    /// Linear search by ID. Returns the index or -1.
    static int linearSearchById(const std::vector<PatientPtr>& patients, const std::string& id,
                                std::size_t& comparisons);

    /// Binary search by ID. REQUIRES `patients` sorted by ID. Returns the index or -1.
    static int binarySearchById(const std::vector<PatientPtr>& patients, const std::string& id,
                                std::size_t& comparisons);

    /// Linear search for names starting with `prefixLower` (already lower case).
    static std::vector<std::size_t> linearSearchByName(const std::vector<PatientPtr>& patients,
                                                       const std::string& prefixLower,
                                                       std::size_t& comparisons);

    /// Binary search for names starting with `prefixLower`. REQUIRES patients sorted by name.
    static std::vector<std::size_t> binarySearchByName(const std::vector<PatientPtr>& patients,
                                                       const std::string& prefixLower,
                                                       std::size_t& comparisons);

    /// RECURSIVE search: checks record `index`, then calls itself for `index + 1`.
    /// Collects every record whose `field` matches `keyword` (already normalised).
    static void recursiveSearch(const std::vector<PatientPtr>& patients, SearchField field,
                                const std::string& keyword, std::size_t index,
                                std::vector<std::size_t>& found, std::size_t& comparisons);

    /// Does one record match? (shared by linear and recursive search)
    static bool matches(const model::PatientRecord& patient, SearchField field, const std::string& keyword);

    /// Normalises a user keyword for the given field (trim + case folding).
    static std::string normalizeKeyword(SearchField field, const std::string& keyword);
};

} // namespace scapms::controller
