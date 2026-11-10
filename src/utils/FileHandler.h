// FileHandler.h - saves and loads the clinic database as a CSV text file.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// File format (one record per line, fields separated by commas, text in "quotes" when needed):
//   # SCAPMS data file v1
//   PATIENT,<category>,<id>,<name>,<age>,<contact>,<extra1>,<extra2>
//   APPOINTMENT,<number>,<patientId>,<date>,<time>,<reason>,<status>,<tag>
#pragma once

#include <string>

#include "model/ClinicDatabase.h"

namespace scapms::utils {

class FileHandler {
public:
    /// Writes the whole database. Throws utils::FileException on failure.
    static void save(const model::ClinicDatabase& database, const std::string& path);

    /// Reads a database. The result is built separately, so a bad file never damages the data
    /// currently in memory. Throws utils::FileException (with the line number) on failure.
    static model::ClinicDatabase load(const std::string& path);

    /// Writes plain text (used to export reports). Throws utils::FileException on failure.
    static void writeText(const std::string& path, const std::string& text);
};

} // namespace scapms::utils
