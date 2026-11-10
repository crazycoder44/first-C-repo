// ReportPanel.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "gui/ReportPanel.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "gui/GuiHelpers.h"
#include "utils/DateUtils.h"
#include "utils/FileHandler.h"

namespace scapms::gui {

ReportPanel::ReportPanel(controller::ClinicManager& manager, QWidget* parent) : BasePanel(manager, parent) {
    buildUi();
    generate();
}

void ReportPanel::buildUi() {
    auto* root = new QVBoxLayout(this);

    auto* bar = new QHBoxLayout;
    reportCombo_ = new QComboBox;
    reportCombo_->addItems({tr("Most Frequently Attended Patients"), tr("Patients with Pending Appointments"),
                            tr("Patient Category Distribution"), tr("Daily Appointment Summary")});
    reportCombo_->setToolTip(tr("Choose which report to show."));
    dateEdit_ = new QLineEdit(qs(utils::todayIso()));
    dateEdit_->setPlaceholderText("YYYY-MM-DD (empty = all dates)");
    dateEdit_->setToolTip(tr("Date for the Daily Appointment Summary. Leave empty for all dates."));
    generateButton_ = new QPushButton(tr("Generate"));
    generateButton_->setToolTip(tr("Create the selected report from the current data."));
    exportButton_ = new QPushButton(tr("Export..."));
    exportButton_->setToolTip(tr("Save the report as a text file."));
    bar->addWidget(new QLabel(tr("Report:")));
    bar->addWidget(reportCombo_, 1);
    bar->addWidget(new QLabel(tr("Date:")));
    bar->addWidget(dateEdit_);
    bar->addWidget(generateButton_);
    bar->addWidget(exportButton_);
    root->addLayout(bar);

    output_ = new QPlainTextEdit;
    output_->setReadOnly(true);
    output_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));  // columns line up in a fixed-width font
    root->addWidget(output_, 1);

    connect(generateButton_, &QPushButton::clicked, this, &ReportPanel::generate);
    connect(exportButton_, &QPushButton::clicked, this, &ReportPanel::exportReport);
    connect(reportCombo_, &QComboBox::currentIndexChanged, this, &ReportPanel::onReportChanged);
    connect(dateEdit_, &QLineEdit::returnPressed, this, &ReportPanel::generate);
    onReportChanged();
}

void ReportPanel::onReportChanged() {
    dateEdit_->setEnabled(reportCombo_->currentIndex() == 3);
    generate();
}

void ReportPanel::generate() {
    runGuarded(this, [&] {
        const controller::ReportGenerator reports = manager_.reports();
        switch (reportCombo_->currentIndex()) {
            case 0: output_->setPlainText(qs(reports.mostFrequentlyAttended())); break;
            case 1: output_->setPlainText(qs(reports.pendingAppointments())); break;
            case 2: output_->setPlainText(qs(reports.categoryDistribution())); break;
            default: output_->setPlainText(qs(reports.dailySummary(ss(dateEdit_->text())))); break;
        }
    });
}

void ReportPanel::exportReport() {
    const QString path = QFileDialog::getSaveFileName(this, tr("Export report"), "report.txt",
                                                      tr("Text files (*.txt);;All files (*)"));
    if (path.isEmpty()) return;
    const bool ok = runGuarded(this, [&] {
        utils::FileHandler::writeText(ss(path), ss(output_->toPlainText()));
        emit statusMessage(tr("Report exported to %1.").arg(path));
    });
    Q_UNUSED(ok);
}

void ReportPanel::refresh() {
    // Regenerate quietly: a half-typed date should not pop up an error dialog on every refresh.
    try {
        const controller::ReportGenerator reports = manager_.reports();
        switch (reportCombo_->currentIndex()) {
            case 0: output_->setPlainText(qs(reports.mostFrequentlyAttended())); break;
            case 1: output_->setPlainText(qs(reports.pendingAppointments())); break;
            case 2: output_->setPlainText(qs(reports.categoryDistribution())); break;
            default: output_->setPlainText(qs(reports.dailySummary(ss(dateEdit_->text())))); break;
        }
    } catch (const utils::ClinicException& error) {
        output_->setPlainText(qs(std::string("Error: ") + error.what()));
    }
}

} // namespace scapms::gui
