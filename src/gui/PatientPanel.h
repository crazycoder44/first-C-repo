// PatientPanel.h - "Patient Records" tab: view, add, edit and delete patients.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include <QList>

#include "gui/BasePanel.h"

class QComboBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

namespace scapms::gui {

class PatientPanel : public BasePanel {
    Q_OBJECT
public:
    explicit PatientPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);
    void refresh() override;

    /// Fills the form with sample text (used only when generating documentation screenshots).
    void showDemoInput();

private slots:
    void onCategoryChanged();            // combo box event -> rebuild the type-specific fields
    void onIdTextChanged(const QString& text);  // text field event -> live duplicate-ID warning
    void onAddClicked();
    void onUpdateClicked();
    void onDeleteClicked();
    void onClearClicked();
    void onSelectionChanged();

private:
    void buildUi();
    void rebuildExtraFields(const QString& category);
    controller::PatientInput readForm() const;
    void loadIntoForm(const model::PatientRecord& patient);
    void setEditMode(bool editing);
    QString selectedPatientId() const;
    void clearForm();

    // form widgets
    QComboBox* categoryCombo_ = nullptr;
    QLineEdit* idEdit_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QLineEdit* ageEdit_ = nullptr;
    QLineEdit* contactEdit_ = nullptr;
    QFormLayout* formLayout_ = nullptr;
    QList<QLineEdit*> extraEdits_;       // created dynamically for the selected category
    QLabel* idHint_ = nullptr;
    QLabel* detailLabel_ = nullptr;

    QPushButton* addButton_ = nullptr;
    QPushButton* updateButton_ = nullptr;
    QPushButton* deleteButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;

    QTableWidget* table_ = nullptr;
    int fixedRowCount_ = 0;              // form rows that are always present (before the dynamic ones)
    bool editing_ = false;
};

} // namespace scapms::gui
