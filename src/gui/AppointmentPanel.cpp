// AppointmentPanel.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "gui/AppointmentPanel.h"

#include <QBrush>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QVBoxLayout>

#include "gui/GuiHelpers.h"
#include "utils/DateUtils.h"

namespace scapms::gui {

AppointmentPanel::AppointmentPanel(controller::ClinicManager& manager, QWidget* parent) : BasePanel(manager, parent) {
    buildUi();
    refresh();
}

void AppointmentPanel::buildUi() {
    auto* root = new QVBoxLayout(this);

    // LAYOUT: a QGridLayout lines the booking fields up in columns (label above/beside field).
    auto* box = new QGroupBox(tr("Schedule an appointment"));
    auto* grid = new QGridLayout(box);

    patientCombo_ = new QComboBox;
    patientCombo_->setToolTip(tr("The patient the appointment is for."));
    dateEdit_ = new QLineEdit(qs(utils::todayIso()));
    dateEdit_->setPlaceholderText("YYYY-MM-DD");
    dateEdit_->setToolTip(tr("Appointment date, e.g. 2026-11-20."));
    timeEdit_ = new QLineEdit;
    timeEdit_->setPlaceholderText("HH:MM");
    timeEdit_->setToolTip(tr("24-hour time, e.g. 14:30."));
    reasonEdit_ = new QLineEdit;
    reasonEdit_->setPlaceholderText(tr("Reason for the visit"));
    reasonEdit_->setToolTip(tr("Short reason for the appointment (100 characters maximum)."));

    grid->addWidget(new QLabel(tr("Patient:")), 0, 0);
    grid->addWidget(patientCombo_, 0, 1, 1, 3);
    grid->addWidget(new QLabel(tr("Date:")), 1, 0);
    grid->addWidget(dateEdit_, 1, 1);
    grid->addWidget(new QLabel(tr("Time:")), 1, 2);
    grid->addWidget(timeEdit_, 1, 3);
    grid->addWidget(new QLabel(tr("Reason:")), 2, 0);
    grid->addWidget(reasonEdit_, 2, 1, 1, 3);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(3, 1);
    root->addWidget(box);

    auto* buttons = new QHBoxLayout;
    scheduleButton_ = new QPushButton(tr("Schedule Appointment"));
    scheduleButton_->setToolTip(tr("Book the appointment described above."));
    cancelButton_ = new QPushButton(tr("Cancel Selected"));
    cancelButton_->setToolTip(tr("Cancel the selected scheduled appointment. (can be undone)"));
    completeButton_ = new QPushButton(tr("Mark Completed"));
    completeButton_->setToolTip(tr("The patient attended: mark the selected appointment as completed."));
    filterCombo_ = new QComboBox;
    filterCombo_->addItems({tr("All"), tr("Scheduled"), tr("Completed"), tr("Cancelled")});
    filterCombo_->setToolTip(tr("Show only appointments with this status."));
    buttons->addWidget(scheduleButton_);
    buttons->addWidget(cancelButton_);
    buttons->addWidget(completeButton_);
    buttons->addStretch();
    buttons->addWidget(new QLabel(tr("Show:")));
    buttons->addWidget(filterCombo_);
    root->addLayout(buttons);

    table_ = new QTableWidget;
    const QStringList headers = {tr("No."), tr("Patient ID"), tr("Patient"), tr("Date"),
                                 tr("Time"), tr("Reason"),     tr("Type"),    tr("Status")};
    table_->setColumnCount(headers.size());
    table_->setHorizontalHeaderLabels(headers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    root->addWidget(table_, 1);

    connect(scheduleButton_, &QPushButton::clicked, this, &AppointmentPanel::onScheduleClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &AppointmentPanel::onCancelClicked);
    connect(completeButton_, &QPushButton::clicked, this, &AppointmentPanel::onCompleteClicked);
    connect(filterCombo_, &QComboBox::currentIndexChanged, this, &AppointmentPanel::refreshTable);
    connect(table_, &QTableWidget::itemSelectionChanged, this, &AppointmentPanel::onSelectionChanged);
    onSelectionChanged();
}

int AppointmentPanel::selectedAppointmentId() const {
    const auto rows = table_->selectionModel()->selectedRows();
    if (rows.isEmpty()) return -1;
    return table_->item(rows.first().row(), 0)->text().toInt();
}

void AppointmentPanel::onSelectionChanged() {
    const bool selected = selectedAppointmentId() > 0;
    cancelButton_->setEnabled(selected);
    completeButton_->setEnabled(selected);
}

void AppointmentPanel::onScheduleClicked() {
    const bool ok = runGuarded(this, [&] {
        const QString patientId = patientCombo_->currentData().toString();
        const model::Appointment& created = manager_.appointments().schedule(
            ss(patientId), ss(dateEdit_->text()), ss(timeEdit_->text()), ss(reasonEdit_->text()));
        emit statusMessage(tr("Appointment #%1 scheduled for %2 on %3 at %4.")
                               .arg(created.id()).arg(patientId, qs(created.date()), qs(created.time())));
    });
    if (ok) {
        timeEdit_->clear();
        reasonEdit_->clear();
        emit dataChanged();
    }
}

void AppointmentPanel::onCancelClicked() {
    const int id = selectedAppointmentId();
    if (id <= 0) return;
    const bool ok = runGuarded(this, [&] {
        manager_.appointments().cancel(id);
        emit statusMessage(tr("Appointment #%1 cancelled. (Ctrl+Z to undo)").arg(id));
    });
    if (ok) emit dataChanged();
}

void AppointmentPanel::onCompleteClicked() {
    const int id = selectedAppointmentId();
    if (id <= 0) return;
    const bool ok = runGuarded(this, [&] {
        manager_.appointments().complete(id);
        emit statusMessage(tr("Appointment #%1 marked as completed.").arg(id));
    });
    if (ok) emit dataChanged();
}

void AppointmentPanel::refreshPatientCombo() {
    const QString previous = patientCombo_->currentData().toString();
    const QSignalBlocker blocker(patientCombo_);
    patientCombo_->clear();
    for (const auto& patient : manager_.patients()) {
        patientCombo_->addItem(qs(patient->id() + " - " + patient->name()), qs(patient->id()));
    }
    const int index = patientCombo_->findData(previous);
    if (index >= 0) patientCombo_->setCurrentIndex(index);
}

void AppointmentPanel::refreshTable() {
    const int selectedId = selectedAppointmentId();
    const int filter = filterCombo_->currentIndex();  // 0 all, 1 scheduled, 2 completed, 3 cancelled
    const auto& database = manager_.database();

    const QSignalBlocker blocker(table_);
    table_->setRowCount(0);
    for (const model::Appointment& a : database.appointments()) {
        if (filter == 1 && a.status() != model::AppointmentStatus::Scheduled) continue;
        if (filter == 2 && a.status() != model::AppointmentStatus::Completed) continue;
        if (filter == 3 && a.status() != model::AppointmentStatus::Cancelled) continue;

        const model::PatientRecord* patient = database.patientById(a.patientId());
        const QStringList cells = {QString::number(a.id()), qs(a.patientId()),
                                   patient ? qs(patient->name()) : QString("?"),
                                   qs(a.date()), qs(a.time()), qs(a.reason()), qs(a.tag()),
                                   qs(model::toString(a.status()))};
        const int row = table_->rowCount();
        table_->insertRow(row);
        for (int column = 0; column < cells.size(); ++column) {
            auto* item = new QTableWidgetItem(cells[column]);
            item->setBackground(QBrush(statusColor(a.status())));  // colour shows the status
            item->setForeground(QBrush(a.tag() == "URGENT" ? QColor(140, 0, 0) : QColor(20, 20, 20)));
            table_->setItem(row, column, item);
        }
        if (a.id() == selectedId) table_->selectRow(row);
    }
    onSelectionChanged();
}

void AppointmentPanel::refresh() {
    refreshPatientCombo();
    refreshTable();
}

} // namespace scapms::gui
