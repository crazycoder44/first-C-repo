// QueuePanel.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "gui/QueuePanel.h"

#include <ctime>

#include <QBrush>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

#include "gui/GuiHelpers.h"
#include "utils/DateUtils.h"

namespace scapms::gui {

QueuePanel::QueuePanel(controller::ClinicManager& manager, QWidget* parent) : BasePanel(manager, parent) {
    buildUi();
    refresh();
}

void QueuePanel::buildUi() {
    auto* root = new QVBoxLayout(this);

    auto* addBox = new QGroupBox(tr("Waiting queue"));
    auto* addRow = new QHBoxLayout(addBox);
    patientCombo_ = new QComboBox;
    patientCombo_->setMinimumWidth(260);
    patientCombo_->setToolTip(tr("Choose the patient who has arrived."));
    addButton_ = new QPushButton(tr("Add to Queue"));
    addButton_->setToolTip(tr("Put the patient at the back of the waiting queue (emergencies go to the front group)."));
    callNextButton_ = new QPushButton(tr("Call Next Patient"));
    callNextButton_->setToolTip(tr("Call the patient who has been waiting longest (emergencies first)."));
    alertMinutes_ = new QSpinBox;
    alertMinutes_->setRange(1, 240);
    alertMinutes_->setValue(15);
    alertMinutes_->setSuffix(tr(" min"));
    alertMinutes_->setToolTip(tr("Warn staff when a patient has waited longer than this."));
    addRow->addWidget(new QLabel(tr("Patient:")));
    addRow->addWidget(patientCombo_, 1);
    addRow->addWidget(addButton_);
    addRow->addWidget(callNextButton_);
    addRow->addWidget(new QLabel(tr("Alert after:")));
    addRow->addWidget(alertMinutes_);
    root->addWidget(addBox);

    nowServing_ = new QLabel(tr("Nobody has been called yet."));
    nowServing_->setWordWrap(true);
    nowServing_->setStyleSheet("background:#eef6ff; border:1px solid #9bbfe8; padding:8px; font-size:13px;");
    root->addWidget(nowServing_);

    alertLabel_ = new QLabel;
    alertLabel_->setStyleSheet("color:#b35900; font-weight:bold;");
    root->addWidget(alertLabel_);

    table_ = new QTableWidget;
    const QStringList headers = {tr("Position"), tr("Patient ID"), tr("Name"), tr("Category"), tr("Waiting")};
    table_->setColumnCount(headers.size());
    table_->setHorizontalHeaderLabels(headers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    root->addWidget(table_, 1);

    connect(addButton_, &QPushButton::clicked, this, &QueuePanel::onAddClicked);
    connect(callNextButton_, &QPushButton::clicked, this, &QueuePanel::onCallNextClicked);
    connect(alertMinutes_, &QSpinBox::valueChanged, this, &QueuePanel::refreshTable);
}

void QueuePanel::onAddClicked() {
    const bool ok = runGuarded(this, [&] {
        const QString id = patientCombo_->currentData().toString();
        manager_.addToQueue(ss(id));
        emit statusMessage(tr("Patient %1 added to the waiting queue.").arg(id));
    });
    if (ok) emit dataChanged();
}

void QueuePanel::onCallNextClicked() {
    const bool ok = runGuarded(this, [&] {
        const controller::QueueEntry entry = manager_.callNext();
        const model::PatientRecord* patient = manager_.findPatient(entry.patientId);
        if (patient) {
            // POLYMORPHISM: one base-class reference, three possible behaviours.
            nowServing_->setText(tr("<b>Now serving</b><br>%1")
                                     .arg(qs(manager_.processPatient(*patient)).toHtmlEscaped().replace("\n", "<br>")));
            emit statusMessage(tr("Calling %1 (%2).").arg(qs(patient->name()), qs(patient->category())));
        }
    });
    if (ok) emit dataChanged();
}

void QueuePanel::refreshPatientCombo() {
    const QString previous = patientCombo_->currentData().toString();
    const QSignalBlocker blocker(patientCombo_);
    patientCombo_->clear();
    for (const auto& patient : manager_.patients()) {
        patientCombo_->addItem(qs(patient->id() + " - " + patient->name() + " [" + patient->category() + "]"),
                               qs(patient->id()));
    }
    const int index = patientCombo_->findData(previous);
    if (index >= 0) patientCombo_->setCurrentIndex(index);
}

void QueuePanel::refreshTable() {
    const std::vector<controller::QueueEntry> entries = manager_.queueSnapshot();
    const std::time_t now = std::time(nullptr);
    const long limitSeconds = static_cast<long>(alertMinutes_->value()) * 60;

    table_->setRowCount(static_cast<int>(entries.size()));
    int overdue = 0;
    int row = 0;
    for (const controller::QueueEntry& entry : entries) {
        const model::PatientRecord* patient = manager_.database().patientById(entry.patientId);
        const long waited = static_cast<long>(now - entry.joinedAt);
        const bool late = waited > limitSeconds;
        if (late) ++overdue;

        const QStringList cells = {QString::number(row + 1), qs(entry.patientId),
                                   patient ? qs(patient->name()) : QString("?"),
                                   patient ? qs(patient->category()) : QString("?"),
                                   qs(utils::formatDuration(waited))};
        for (int column = 0; column < cells.size(); ++column) {
            auto* item = new QTableWidgetItem(cells[column]);
            QColor background = patient ? categoryColor(patient->category()) : QColor();
            if (late) background = QColor(255, 220, 160);  // orange: waiting too long
            item->setBackground(QBrush(background));
            item->setForeground(QBrush(QColor(20, 20, 20)));
            table_->setItem(row, column, item);
        }
        ++row;
    }

    alertLabel_->setText(overdue > 0 ? tr("ALERT: %1 patient(s) have waited longer than %2 minute(s).")
                                           .arg(overdue).arg(alertMinutes_->value())
                                     : QString());
    callNextButton_->setEnabled(!entries.empty());
}

void QueuePanel::refresh() {
    refreshPatientCombo();
    refreshTable();
}

} // namespace scapms::gui
