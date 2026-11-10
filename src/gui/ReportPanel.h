// ReportPanel.h - "Reports" tab.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include "gui/BasePanel.h"

class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

namespace scapms::gui {

class ReportPanel : public BasePanel {
    Q_OBJECT
public:
    explicit ReportPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);
    void refresh() override;

private slots:
    void generate();
    void exportReport();
    void onReportChanged();

private:
    void buildUi();

    QComboBox* reportCombo_ = nullptr;
    QLineEdit* dateEdit_ = nullptr;
    QPushButton* generateButton_ = nullptr;
    QPushButton* exportButton_ = nullptr;
    QPlainTextEdit* output_ = nullptr;
};

} // namespace scapms::gui
