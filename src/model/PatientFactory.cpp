// PatientFactory.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "model/PatientFactory.h"

#include "model/EmergencyPatient.h"
#include "model/FollowUpPatient.h"
#include "model/GeneralPatient.h"
#include "utils/ClinicException.h"
#include "utils/StringUtils.h"

namespace scapms::model {

std::vector<std::string> PatientFactory::categories() {
    return {category::General, category::Emergency, category::FollowUp};
}

std::vector<ExtraField> PatientFactory::extraFields(const std::string& categoryName) {
    const std::string key = utils::normalizeKey(categoryName);
    if (key == "general") return {{"Complaint", "e.g. Headache and fever"}};
    if (key == "emergency") return {{"Severity (1-5)", "1 = mild ... 5 = critical"}, {"Triage note", "e.g. Chest pain"}};
    if (key == "followup") return {{"Previous visit", "YYYY-MM-DD"}, {"Doctor", "e.g. Adeyemi"}};
    return {};
}

std::unique_ptr<PatientRecord> PatientFactory::create(const std::string& categoryName, const std::string& id,
                                                      const std::string& name, int age,
                                                      const std::string& contact,
                                                      const std::vector<std::string>& extras) {
    const std::string key = utils::normalizeKey(categoryName);
    const std::size_t expected = extraFields(categoryName).size();
    if (expected == 0) throw utils::InvalidPatientException("Unknown patient category '" + categoryName + "'.");
    if (extras.size() < expected) {
        throw utils::InvalidPatientException("Missing type-specific information for a " + categoryName + " patient.");
    }

    if (key == "general") {
        return std::make_unique<GeneralPatient>(id, name, age, contact, extras[0]);
    }
    if (key == "emergency") {
        return std::make_unique<EmergencyPatient>(id, name, age, contact,
                                                  EmergencyPatient::parseSeverity(extras[0]), extras[1]);
    }
    return std::make_unique<FollowUpPatient>(id, name, age, contact, extras[0], extras[1]);
}

} // namespace scapms::model
