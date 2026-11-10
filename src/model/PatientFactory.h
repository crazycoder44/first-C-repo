// PatientFactory.h - creates the right PatientRecord subclass from a category name.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// The GUI and the file loader only talk to the abstract PatientRecord type; this factory is the
// one place that knows which concrete class belongs to which category.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "model/PatientRecord.h"

namespace scapms::model {

/// Describes one type-specific input field (used to build the form dynamically).
struct ExtraField {
    std::string label;
    std::string placeholder;
};

class PatientFactory {
public:
    /// {"General", "Emergency", "Follow-Up"}
    static std::vector<std::string> categories();

    /// The extra fields required by a category (empty if the category is unknown).
    static std::vector<ExtraField> extraFields(const std::string& category);

    /// Builds a patient. `extras` must match extraFields(category).size().
    /// Throws utils::InvalidPatientException for unknown categories or invalid data.
    static std::unique_ptr<PatientRecord> create(const std::string& category, const std::string& id,
                                                 const std::string& name, int age,
                                                 const std::string& contact,
                                                 const std::vector<std::string>& extras);
};

} // namespace scapms::model
