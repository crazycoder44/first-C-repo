// PatientPanel.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "gui/PatientPanel.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QVBoxLayout>

#include "gui/GuiHelpers.h"
#include "model/PatientFactory.h"
#include "utils/StringUtils.h"

namespace scapms::gui {

PatientPanel::PatientPanel(controller::ClinicManager& manager, QWidget* parent) : BasePanel(manager, parent) {
    buildUi();
    refresh();
}

void PatientPanel::buildUi() {
    // LAYOUTS: the panel is a QVBoxLayout (form on top, table below). The form itself is a
    // QFormLayout (label + field pairs are exactly what a registration form needs) and the
    // buttons sit side by side in a QHBoxLayout.
    auto* root = new QVBoxLayout(this);

    auto* formBox = new QGroupBox(tr("Patient details"));
    formLayout_ = new QFormLayout(formBox);

    categoryCombo_ = new QComboBox;
    for (const std::string& name : model::PatientFactory::categories()) categoryCombo_->addItem(qs(name));
    categoryCombo_->setToolTip(tr("The patient type decides which extra fields are shown below."));
    formLayout_->addRow(tr("Category:"), categoryCombo_);

    idEdit_ = new QLineEdit;
    idEdit_->setPlaceholderText(tr("Leave empty to generate automatically (e.g. P0013)"));
    idEdit_->setToolTip(tr("Unique patient ID. Letters, digits and '-' only."));
    idHint_ = new QLabel;
    idHint_->setStyleSheet("color: #b00020;");
    idHint_->hide();  // only shown while a duplicate ID is typed
    auto* idBox = new QVBoxLayout;
    idBox->setContentsMargins(0, 0, 0, 0);
    idBox->addWidget(idEdit_);
    idBox->addWidget(idHint_);
    formLayout_->addRow(tr("Patient ID:"), idBox);

    nameEdit_ = new QLineEdit;
    nameEdit_->setPlaceholderText(tr("Full name"));
    nameEdit_->setToolTip(tr("Letters, spaces and . ' - only."));
    formLayout_->addRow(tr("Name:"), nameEdit_);

    ageEdit_ = new QLineEdit;
    ageEdit_->setPlaceholderText(tr("0 - 120"));
    ageEdit_->setToolTip(tr("Age in whole years (0 to 120)."));
    formLayout_->addRow(tr("Age:"), ageEdit_);

    contactEdit_ = new QLineEdit;
    contactEdit_->setPlaceholderText(tr("Phone number or email"));
    contactEdit_->setToolTip(tr("A phone number (7-15 digits) or an email address."));
    formLayout_->addRow(tr("Contact:"), contactEdit_);

    fixedRowCount_ = formLayout_->rowCount();  // everything after this row is created dynamically
    root->addWidget(formBox);

    auto* buttons = new QHBoxLayout;
    addButton_ = new QPushButton(tr("Add Patient"));
    addButton_->setToolTip(tr("Register a new patient with the details above."));
    updateButton_ = new QPushButton(tr("Update Selected"));
    updateButton_->setToolTip(tr("Save the changes made to the selected patient. (can be undone)"));
    deleteButton_ = new QPushButton(tr("Delete Selected"));
    deleteButton_->setToolTip(tr("Delete the selected patient and their appointments. (can be undone)"));
    clearButton_ = new QPushButton(tr("Clear Form"));
    clearButton_->setToolTip(tr("Empty the form and get ready to register a new patient."));
    buttons->addWidget(addButton_);
    buttons->addWidget(updateButton_);
    buttons->addWidget(deleteButton_);
    buttons->addWidget(clearButton_);
    buttons->addStretch();
    root->addLayout(buttons);

    detailLabel_ = new QLabel(tr("Select a patient to see the details."));
    detailLabel_->setWordWrap(true);
    detailLabel_->setStyleSheet("background:#f4f4f4; border:1px solid #ccc; padding:6px;");
    root->addWidget(detailLabel_);

    table_ = new QTableWidget;
    setupPatientTable(table_);
    root->addWidget(table_, 1);

    // EVENTS: connect widget signals to our slots.
    connect(categoryCombo_, &QComboBox::currentTextChanged, this, &PatientPanel::onCategoryChanged);
    connect(idEdit_, &QLineEdit::textChanged, this, &PatientPanel::onIdTextChanged);
    connect(addButton_, &QPushButton::clicked, this, &PatientPanel::onAddClicked);
    connect(updateButton_, &QPushButton::clicked, this, &PatientPanel::onUpdateClicked);
    connect(deleteButton_, &QPushButton::clicked, this, &PatientPanel::onDeleteClicked);
    connect(clearButton_, &QPushButton::clicked, this, &PatientPanel::onClearClicked);
    connect(table_, &QTableWidget::itemSelectionChanged, this, &PatientPanel::onSelectionChanged);

    rebuildExtraFields(categoryCombo_->currentText());
    setEditMode(false);
}

// DYNAMIC COMPONENTS: remove the old type-specific rows and create the rows the category needs.
void PatientPanel::rebuildExtraFields(const QString& category) {
    while (formLayout_->rowCount() > fixedRowCount_) formLayout_->removeRow(formLayout_->rowCount() - 1);
    extraEdits_.clear();
    for (const model::ExtraField& field : model::PatientFactory::extraFields(ss(category))) {
        auto* edit = new QLineEdit;
        edit->setPlaceholderText(qs(field.placeholder));
        formLayout_->addRow(qs(field.label) + ":", edit);
        extraEdits_.append(edit);
    }
}

void PatientPanel::onCategoryChanged() {
    rebuildExtraFields(categoryCombo_->currentText());
}

void PatientPanel::onIdTextChanged(const QString& text) {
    // Live feedback while typing: warn if the ID is already taken.
    if (editing_ || text.trimmed().isEmpty()) {
        idHint_->clear();
        idHint_->hide();
        return;
    }
    const std::string id = utils::toUpper(utils::trim(ss(text)));
    const bool taken = manager_.database().hasPatient(id);
    idHint_->setText(taken ? tr("Patient ID already exists.") : QString());
    idHint_->setVisible(taken);
}

controller::PatientInput PatientPanel::readForm() const {
    controller::PatientInput input;
    input.category = ss(categoryCombo_->currentText());
    input.id = ss(idEdit_->text());
    input.name = ss(nameEdit_->text());
    input.ageText = ss(ageEdit_->text());
    input.contact = ss(contactEdit_->text());
    for (const QLineEdit* edit : extraEdits_) input.extras.push_back(ss(edit->text()));
    return input;
}

void PatientPanel::onAddClicked() {
    // try/catch lives in runGuarded: any error becomes a dialog, the program keeps running.
    const bool ok = runGuarded(this, [&] {
        const std::string id = manager_.registerPatient(readForm());
        emit statusMessage(tr("Registered patient %1.").arg(qs(id)));
    });
    if (ok) {
        clearForm();
        emit dataChanged();
    }
}

void PatientPanel::onUpdateClicked() {
    const QString id = selectedPatientId();
    if (id.isEmpty()) return;
    const bool ok = runGuarded(this, [&] {
        manager_.editPatient(ss(id), readForm());
        emit statusMessage(tr("Updated patient %1. (Ctrl+Z to undo)").arg(id));
    });
    if (ok) emit dataChanged();
}

void PatientPanel::onDeleteClicked() {
    const QString id = selectedPatientId();
    if (id.isEmpty()) return;
    const auto answer = QMessageBox::question(
        this, tr("Delete patient"),
        tr("Delete patient %1 and all of their appointments?\nYou can undo this with Ctrl+Z.").arg(id));
    if (answer != QMessageBox::Yes) return;

    const bool ok = runGuarded(this, [&] {
        manager_.deletePatient(ss(id));
        emit statusMessage(tr("Deleted patient %1. (Ctrl+Z to undo)").arg(id));
    });
    if (ok) {
        clearForm();
        emit dataChanged();
    }
}

void PatientPanel::onClearClicked() {
    table_->clearSelection();
    clearForm();
}

QString PatientPanel::selectedPatientId() const {
    const auto rows = table_->selectionModel()->selectedRows();
    if (rows.isEmpty()) return QString();
    return table_->item(rows.first().row(), 0)->text();
}

void PatientPanel::onSelectionChanged() {
    const QString id = selectedPatientId();
    if (id.isEmpty()) {
        setEditMode(false);
        return;
    }
    // findPatient() uses the 10-slot array cache before searching the vector.
    if (const model::PatientRecord* patient = manager_.findPatient(ss(id))) {
        setEditMode(true);  // first, so filling the ID box does not trigger the duplicate-ID warning
        loadIntoForm(*patient);
        // POLYMORPHISM: processPatient() works with the base class; the right subclass code runs.
        detailLabel_->setText(qs(manager_.processPatient(*patient)).toHtmlEscaped().replace("\n", "<br>"));
        emit viewChanged();  // the cache changed -> let the dashboard show it (no data was modified)
    }
}

void PatientPanel::loadIntoForm(const model::PatientRecord& patient) {
    {
        const QSignalBlocker blocker(categoryCombo_);
        categoryCombo_->setCurrentText(qs(patient.category()));
    }
    rebuildExtraFields(categoryCombo_->currentText());
    idEdit_->setText(qs(patient.id()));
    nameEdit_->setText(qs(patient.name()));
    ageEdit_->setText(QString::number(patient.age()));
    contactEdit_->setText(qs(patient.contact()));
    const std::vector<std::string> extras = patient.extraValues();
    for (int i = 0; i < extraEdits_.size() && i < static_cast<int>(extras.size()); ++i) {
        extraEdits_[i]->setText(qs(extras[static_cast<std::size_t>(i)]));
    }
}

void PatientPanel::setEditMode(bool editing) {
    editing_ = editing;
    categoryCombo_->setEnabled(!editing);  // the category and ID of an existing patient are fixed
    idEdit_->setEnabled(!editing);
    addButton_->setEnabled(!editing);
    updateButton_->setEnabled(editing);
    deleteButton_->setEnabled(editing);
    if (editing) idHint_->clear();
}

void PatientPanel::clearForm() {
    setEditMode(false);
    idEdit_->clear();
    nameEdit_->clear();
    ageEdit_->clear();
    contactEdit_->clear();
    for (QLineEdit* edit : extraEdits_) edit->clear();
    idHint_->clear();
    detailLabel_->setText(tr("Select a patient to see the details."));
}

void PatientPanel::showDemoInput() {
    table_->clearSelection();
    clearForm();
    categoryCombo_->setCurrentText(qs(model::category::Emergency));
    nameEdit_->setText("Ifeoma Nnamdi");
    ageEdit_->setText("27");
    contactEdit_->setText("08012349876");
    if (extraEdits_.size() >= 2) {
        extraEdits_[0]->setText("4");
        extraEdits_[1]->setText("Severe asthma attack");
    }
}

void PatientPanel::refresh() {
    const QString selected = selectedPatientId();
    std::vector<const model::PatientRecord*> rows;
    for (const auto& patient : manager_.patients()) rows.push_back(patient.get());

    {
        const QSignalBlocker blocker(table_);  // do not treat a redraw as a user selection
        populatePatientTable(table_, rows);
        if (!selected.isEmpty()) {
            for (int row = 0; row < table_->rowCount(); ++row) {
                if (table_->item(row, 0)->text() == selected) {
                    table_->selectRow(row);
                    break;
                }
            }
        }
    }
    if (!selected.isEmpty() && selectedPatientId().isEmpty()) {
        // The selected patient disappeared (deleted / undone) -> leave edit mode.
        clearForm();
    }
}

} // namespace scapms::gui
