// QueuePanel.h - "Waiting Queue" tab: add patients to the queue and call the next one.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include "gui/BasePanel.h"

class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace scapms::gui {

class QueuePanel : public BasePanel {
    Q_OBJECT
public:
    explicit QueuePanel(controller::ClinicManager& manager, QWidget* parent = nullptr);
    void refresh() override;

    /// Redraw only the queue table (cheap; called by the one-second timer to update waiting times).
    void refreshTable();

private slots:
    void onAddClicked();
    void onCallNextClicked();

private:
    void buildUi();
    void refreshPatientCombo();

    QComboBox* patientCombo_ = nullptr;
    QPushButton* addButton_ = nullptr;
    QPushButton* callNextButton_ = nullptr;
    QSpinBox* alertMinutes_ = nullptr;
    QLabel* nowServing_ = nullptr;
    QLabel* alertLabel_ = nullptr;
    QTableWidget* table_ = nullptr;
};

} // namespace scapms::gui
