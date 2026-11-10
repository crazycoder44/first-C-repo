// DateUtils.h - current date/time helpers (ISO formatted strings).
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include <cstdio>
#include <ctime>
#include <string>

namespace scapms::utils {

/// Today's date as "YYYY-MM-DD".
inline std::string todayIso() {
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", local);
    return buffer;
}

/// Minutes elapsed since midnight (local time).
inline int minutesSinceMidnight() {
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    return local->tm_hour * 60 + local->tm_min;
}

/// Formats a number of seconds as "mm:ss" (or "h:mm:ss" for an hour or more).
inline std::string formatDuration(long seconds) {
    if (seconds < 0) seconds = 0;
    char buffer[32];
    if (seconds >= 3600) {
        std::snprintf(buffer, sizeof(buffer), "%ld:%02ld:%02ld", seconds / 3600, (seconds / 60) % 60, seconds % 60);
    } else {
        std::snprintf(buffer, sizeof(buffer), "%02ld:%02ld", seconds / 60, seconds % 60);
    }
    return buffer;
}

} // namespace scapms::utils
