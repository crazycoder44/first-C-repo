// AppointmentPanel.h - "Appointments" tab: schedule, cancel, complete and view appointments.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include "gui/BasePanel.h"

class QComboBox;
class QLineEdit;
class QPushButton;
class QTableWidget;

namespace scapms::gui {

class AppointmentPanel : public BasePanel {
    Q_OBJECT
public:
    explicit AppointmentPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);
    void refresh() override;

private slots:
    void onScheduleClicked();
    void onCancelClicked();
    void onCompleteClicked();
    void refreshTable();
    void onSelectionChanged();

private:
    void buildUi();
    void refreshPatientCombo();
    int selectedAppointmentId() const;

    QComboBox* patientCombo_ = nullptr;
    QLineEdit* dateEdit_ = nullptr;
    QLineEdit* timeEdit_ = nullptr;
    QLineEdit* reasonEdit_ = nullptr;
    QComboBox* filterCombo_ = nullptr;
    QPushButton* scheduleButton_ = nullptr;
    QPushButton* cancelButton_ = nullptr;
    QPushButton* completeButton_ = nullptr;
    QTableWidget* table_ = nullptr;
};

} // namespace scapms::gui
