// GuiHelpers.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "gui/GuiHelpers.h"

#include <QBrush>
#include <QFont>
#include <QHeaderView>
#include <QTableWidgetItem>

#include "utils/StringUtils.h"

namespace scapms::gui {

QColor categoryColor(const std::string& category) {
    const std::string key = utils::normalizeKey(category);
    if (key == "emergency") return QColor(255, 214, 214);  // High priority -> red tint
    if (key == "followup") return QColor(214, 232, 255);   // Scheduled -> blue tint
    return QColor(222, 244, 222);                          // Normal -> green tint
}

QColor statusColor(model::AppointmentStatus status) {
    switch (status) {
        case model::AppointmentStatus::Scheduled: return QColor(255, 244, 204);  // amber: still to happen
        case model::AppointmentStatus::Completed: return QColor(214, 240, 214);  // green: done
        case model::AppointmentStatus::Cancelled: return QColor(232, 232, 232);  // grey: cancelled
    }
    return QColor();
}

void setupPatientTable(QTableWidget* table) {
    const QStringList headers = {QObject::tr("Patient ID"), QObject::tr("Name"), QObject::tr("Age"),
                                 QObject::tr("Contact"),    QObject::tr("Category"), QObject::tr("Priority"),
                                 QObject::tr("Details"),    QObject::tr("Appointments")};
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
}

void populatePatientTable(QTableWidget* table, const std::vector<const model::PatientRecord*>& patients) {
    table->setRowCount(static_cast<int>(patients.size()));
    int row = 0;
    for (const model::PatientRecord* patient : patients) {
        const QStringList cells = {qs(patient->id()),
                                   qs(patient->name()),
                                   QString::number(patient->age()),
                                   qs(patient->contact()),
                                   qs(patient->category()),
                                   QString::number(patient->priority()),
                                   qs(patient->details()),
                                   QString::number(patient->appointmentIds().size())};
        const bool emergency = utils::normalizeKey(patient->category()) == "emergency";
        for (int column = 0; column < cells.size(); ++column) {
            auto* item = new QTableWidgetItem(cells[column]);
            item->setBackground(QBrush(categoryColor(patient->category())));
            item->setForeground(QBrush(emergency ? QColor(140, 0, 0) : QColor(20, 20, 20)));
            if (emergency) {
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
            }
            table->setItem(row, column, item);
        }
        ++row;
    }
}

} // namespace scapms::gui
