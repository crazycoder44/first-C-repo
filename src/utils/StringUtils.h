// StringUtils.h - small string helpers used across the project.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include <cctype>
#include <string>

namespace scapms::utils {

inline std::string trim(const std::string& text) {
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(start, end - start);
}

inline std::string toLower(std::string text) {
    for (char& c : text) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return text;
}

inline std::string toUpper(std::string text) {
    for (char& c : text) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return text;
}

/// True if `text` starts with `prefix` (both are compared as-is, case sensitive).
inline bool startsWith(const std::string& text, const std::string& prefix) {
    return text.size() >= prefix.size() && text.compare(0, prefix.size(), prefix) == 0;
}

/// Lower-case and keep only letters/digits: "Follow-Up" and "follow up" both become "followup".
inline std::string normalizeKey(const std::string& text) {
    std::string result;
    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
    }
    return result;
}

/// True if the text contains a control character such as a newline (not allowed in stored fields).
inline bool hasControlChars(const std::string& text) {
    for (char c : text) {
        if (std::iscntrl(static_cast<unsigned char>(c))) return true;
    }
    return false;
}

} // namespace scapms::utils
