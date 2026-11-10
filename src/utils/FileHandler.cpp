// FileHandler.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "utils/FileHandler.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "model/PatientFactory.h"
#include "utils/ClinicException.h"
#include "utils/StringUtils.h"

namespace scapms::utils {

namespace {

// Wrap a field in quotes if it contains a comma, a quote or leading/trailing spaces.
std::string csvEscape(const std::string& field) {
    const bool needsQuotes = field.find_first_of(",\"") != std::string::npos ||
                             (!field.empty() && (field.front() == ' ' || field.back() == ' '));
    if (!needsQuotes) return field;
    std::string quoted = "\"";
    for (char c : field) {
        if (c == '"') quoted += "\"\"";
        else quoted += c;
    }
    return quoted + "\"";
}

// Split one CSV line into fields, understanding "quoted, fields" and "" escapes.
std::vector<std::string> parseCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') { current += '"'; ++i; }
                else inQuotes = false;
            } else {
                current += c;
            }
        } else if (c == '"') {
            inQuotes = true;
        } else if (c == ',') {
            fields.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    if (inQuotes) throw FileException("unterminated quoted field");
    fields.push_back(current);
    return fields;
}

std::filesystem::path toPath(const std::string& path) {
    return std::filesystem::u8path(path);  // paths coming from the GUI are UTF-8
}

} // namespace

void FileHandler::save(const model::ClinicDatabase& database, const std::string& path) {
    try {
        const std::filesystem::path target = toPath(path);
        if (target.has_parent_path()) std::filesystem::create_directories(target.parent_path());

        std::ofstream out(target, std::ios::out | std::ios::trunc);
        if (!out) throw FileException("Cannot open '" + path + "' for writing.");

        out << "# SCAPMS data file v1\n";
        for (const auto& patient : database.patients()) {
            std::vector<std::string> extras = patient->extraValues();
            extras.resize(2);  // every PATIENT line has exactly two extra columns
            out << "PATIENT," << csvEscape(patient->category()) << ',' << csvEscape(patient->id()) << ','
                << csvEscape(patient->name()) << ',' << patient->age() << ',' << csvEscape(patient->contact())
                << ',' << csvEscape(extras[0]) << ',' << csvEscape(extras[1]) << '\n';
        }
        for (const auto& a : database.appointments()) {
            out << "APPOINTMENT," << a.id() << ',' << csvEscape(a.patientId()) << ',' << a.date() << ','
                << a.time() << ',' << csvEscape(a.reason()) << ',' << model::toString(a.status()) << ','
                << csvEscape(a.tag()) << '\n';
        }
        out.flush();
        if (!out) throw FileException("Writing to '" + path + "' failed (disk full or no permission?).");
    } catch (const FileException&) {
        throw;
    } catch (const std::exception& e) {
        throw FileException("Could not save '" + path + "': " + e.what());
    }
}

model::ClinicDatabase FileHandler::load(const std::string& path) {
    std::ifstream in(toPath(path));
    if (!in) throw FileException("Cannot open '" + path + "'. Does the file exist?");

    model::ClinicDatabase database;
    std::string line;
    int lineNumber = 0;

    while (std::getline(in, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') line.pop_back();  // Windows line endings
        if (trim(line).empty() || line[0] == '#') continue;

        try {
            const std::vector<std::string> f = parseCsvLine(line);
            if (f[0] == "PATIENT") {
                if (f.size() != 8) throw FileException("a PATIENT line needs 8 fields but has " + std::to_string(f.size()));
                int age = 0;
                try {
                    std::size_t used = 0;
                    age = std::stoi(f[4], &used);
                    if (used != f[4].size()) throw std::invalid_argument("age");
                } catch (const std::exception&) {
                    throw FileException("age '" + f[4] + "' is not a number");
                }
                auto patient = model::PatientFactory::create(f[1], f[2], f[3], age, f[5], {f[6], f[7]});
                database.addPatient(std::move(patient));
            } else if (f[0] == "APPOINTMENT") {
                if (f.size() != 8) throw FileException("an APPOINTMENT line needs 8 fields but has " + std::to_string(f.size()));
                int id = 0;
                try {
                    std::size_t used = 0;
                    id = std::stoi(f[1], &used);
                    if (used != f[1].size()) throw std::invalid_argument("id");
                } catch (const std::exception&) {
                    throw FileException("appointment number '" + f[1] + "' is not a number");
                }
                database.addAppointment(model::Appointment(id, f[2], f[3], f[4], f[5],
                                                           model::statusFromString(f[6]), f[7]));
            } else {
                throw FileException("unknown record type '" + f[0] + "'");
            }
        } catch (const ClinicException& e) {
            throw FileException("Line " + std::to_string(lineNumber) + ": " + e.what());
        }
    }

    // Every appointment must belong to a patient that exists in the file.
    for (const auto& a : database.appointments()) {
        if (!database.hasPatient(a.patientId())) {
            throw FileException("Appointment " + std::to_string(a.id()) + " refers to unknown patient '" +
                                a.patientId() + "'.");
        }
    }
    database.rebuildLinks();
    return database;
}

void FileHandler::writeText(const std::string& path, const std::string& text) {
    std::ofstream out(toPath(path), std::ios::out | std::ios::trunc);
    if (!out) throw FileException("Cannot open '" + path + "' for writing.");
    out << text;
    out.flush();
    if (!out) throw FileException("Writing to '" + path + "' failed.");
}

} // namespace scapms::utils
