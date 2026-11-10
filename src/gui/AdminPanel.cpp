// AdminPanel.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "gui/AdminPanel.h"

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "gui/GuiHelpers.h"
#include "utils/DateUtils.h"

namespace scapms::gui {

AdminPanel::AdminPanel(controller::ClinicManager& manager, QWidget* parent) : BasePanel(manager, parent) {
    buildUi();
    refresh();
}

void AdminPanel::buildUi() {
    auto* root = new QVBoxLayout(this);

    clockLabel_ = new QLabel;
    clockLabel_->setStyleSheet("font-size:18px; font-weight:bold;");
    root->addWidget(clockLabel_);

    // LAYOUT: a QGridLayout arranges the four dashboard panels in a 2 x 2 grid that resizes with the window.
    auto* grid = new QGridLayout;

    auto* overviewBox = new QGroupBox(tr("Clinic overview"));
    auto* overviewLayout = new QVBoxLayout(overviewBox);
    totalLabel_ = new QLabel;
    categoryLabel_ = new QLabel;
    appointmentLabel_ = new QLabel;
    queueLabel_ = new QLabel;
    for (QLabel* label : {totalLabel_, categoryLabel_, appointmentLabel_, queueLabel_}) {
        label->setWordWrap(true);
        overviewLayout->addWidget(label);
    }
    overviewLayout->addStretch();
    grid->addWidget(overviewBox, 0, 0);

    auto* adminBox = new QGroupBox(tr("Administration"));
    auto* adminLayout = new QVBoxLayout(adminBox);
    undoLabel_ = new QLabel;
    undoLabel_->setWordWrap(true);
    undoButton_ = new QPushButton(tr("Undo Last Administrative Operation"));
    undoButton_->setToolTip(tr("Reverse the most recent delete, edit or appointment cancellation (Ctrl+Z)."));
    adminLayout->addWidget(undoLabel_);
    adminLayout->addWidget(undoButton_);
    adminLayout->addStretch();
    grid->addWidget(adminBox, 0, 1);

    auto* cacheBox = new QGroupBox(tr("Frequently accessed patient records (cache of 10)"));
    auto* cacheLayout = new QVBoxLayout(cacheBox);
    cacheLabel_ = new QLabel;
    cacheList_ = new QListWidget;
    cacheLayout->addWidget(cacheLabel_);
    cacheLayout->addWidget(cacheList_);
    grid->addWidget(cacheBox, 1, 0);

    auto* reminderBox = new QGroupBox(tr("Appointment reminders (next 30 minutes)"));
    auto* reminderLayout = new QVBoxLayout(reminderBox);
    reminderList_ = new QListWidget;
    reminderLayout->addWidget(reminderList_);
    grid->addWidget(reminderBox, 1, 1);

    root->addLayout(grid, 1);

    connect(undoButton_, &QPushButton::clicked, this, &AdminPanel::undoRequested);
}

void AdminPanel::setReminders(const QStringList& reminders) {
    reminderList_->clear();
    if (reminders.isEmpty()) reminderList_->addItem(tr("No appointments in the next 30 minutes."));
    else reminderList_->addItems(reminders);
}

void AdminPanel::refresh() {
    clockLabel_->setText(tr("Smart Clinic Dashboard  -  %1").arg(QDateTime::currentDateTime().toString("dddd d MMMM yyyy, hh:mm:ss")));

    const controller::DashboardStats stats = manager_.reports().dashboardStats(utils::todayIso());
    totalLabel_->setText(tr("<b>Registered patients:</b> %1").arg(stats.totalPatients));
    categoryLabel_->setText(tr("General: %1   |   Emergency: %2   |   Follow-Up: %3")
                                .arg(stats.generalPatients).arg(stats.emergencyPatients).arg(stats.followUpPatients));
    appointmentLabel_->setText(tr("<b>Appointments:</b> %1 scheduled (%2 today), %3 completed, %4 cancelled")
                                   .arg(stats.scheduledAppointments).arg(stats.scheduledToday)
                                   .arg(stats.completedAppointments).arg(stats.cancelledAppointments));
    queueLabel_->setText(tr("<b>Patients waiting:</b> %1").arg(manager_.queueSize()));

    if (manager_.canUndo()) {
        undoLabel_->setText(tr("Next undo: <b>%1</b><br>(%2 operation(s) can be undone)")
                                .arg(qs(manager_.nextUndoDescription()).toHtmlEscaped())
                                .arg(manager_.undoDepth()));
    } else {
        undoLabel_->setText(tr("Nothing to undo."));
    }
    undoButton_->setEnabled(manager_.canUndo());

    const controller::RecentPatientCache& cache = manager_.cache();
    cacheLabel_->setText(tr("%1 of %2 slots used  |  hits: %3, misses: %4")
                             .arg(cache.size()).arg(controller::RecentPatientCache::kCapacity)
                             .arg(cache.hits()).arg(cache.misses()));
    cacheList_->clear();
    for (std::size_t i = 0; i < cache.size(); ++i) {
        const model::PatientRecord* patient = cache.at(i);
        cacheList_->addItem(tr("%1  %2 [%3]  - accessed %4x")
                                .arg(qs(patient->id()), qs(patient->name()), qs(patient->category()))
                                .arg(cache.usesAt(i)));
    }
    if (cache.size() == 0) cacheList_->addItem(tr("Select patients in the Patient Records tab to fill the cache."));
}

} // namespace scapms::gui
