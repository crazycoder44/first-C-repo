// GuiHelpers.h - small helpers shared by all GUI panels.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include <exception>
#include <string>
#include <vector>

#include <QColor>
#include <QMessageBox>
#include <QString>
#include <QTableWidget>

#include "model/Appointment.h"
#include "model/PatientRecord.h"
#include "utils/ClinicException.h"

namespace scapms::gui {

inline QString qs(const std::string& text) { return QString::fromStdString(text); }
inline std::string ss(const QString& text) { return text.toStdString(); }

/// Row colour used to distinguish patient categories (custom table presentation).
QColor categoryColor(const std::string& category);
/// Row colour used to distinguish appointment statuses.
QColor statusColor(model::AppointmentStatus status);

/// Gives a table the standard "patient list" columns and look.
void setupPatientTable(QTableWidget* table);
/// Fills a patient table; Emergency rows are red + bold, General green, Follow-Up blue.
void populatePatientTable(QTableWidget* table, const std::vector<const model::PatientRecord*>& patients);

/// Runs `action`, turning any exception into a friendly dialog instead of letting the program crash.
/// Returns true when the action completed without an error.
template <typename Action>
bool runGuarded(QWidget* parent, Action&& action) {
    try {
        action();
        return true;
    } catch (const utils::ClinicException& error) {
        // Errors we raised on purpose: show the message as written for the user.
        QMessageBox::warning(parent, QObject::tr("Cannot complete the operation"),
                             QObject::tr("Error: %1").arg(QString::fromUtf8(error.what())));
    } catch (const std::exception& error) {
        QMessageBox::critical(parent, QObject::tr("Unexpected error"),
                              QObject::tr("An unexpected error occurred: %1").arg(QString::fromUtf8(error.what())));
    } catch (...) {
        QMessageBox::critical(parent, QObject::tr("Unexpected error"),
                              QObject::tr("An unknown error occurred. The operation was cancelled."));
    }
    return false;
}

} // namespace scapms::gui
