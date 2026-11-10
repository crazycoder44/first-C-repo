// SearchPanel.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "gui/SearchPanel.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedLayout>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "gui/GuiHelpers.h"
#include "model/PatientFactory.h"

namespace scapms::gui {

using controller::SearchAlgorithm;
using controller::SearchField;
using controller::SortAlgorithm;
using controller::SortField;

SearchPanel::SearchPanel(controller::ClinicManager& manager, QWidget* parent) : BasePanel(manager, parent) {
    buildUi();
    refresh();
}

void SearchPanel::buildUi() {
    auto* root = new QVBoxLayout(this);

    // ------------------------------------------------------------ search group
    auto* searchBox = new QGroupBox(tr("Search patients"));
    auto* searchForm = new QFormLayout(searchBox);  // QFormLayout: label/field pairs

    searchFieldCombo_ = new QComboBox;
    searchFieldCombo_->addItems({tr("Patient ID"), tr("Patient Name"), tr("Category")});
    searchFieldCombo_->setToolTip(tr("Which field to search in. Name search matches the START of the name."));

    // QStackedLayout: shows EITHER a text box OR a category chooser, depending on the field.
    auto* keywordHolder = new QWidget;
    keywordStack_ = new QStackedLayout(keywordHolder);
    keywordStack_->setContentsMargins(0, 0, 0, 0);
    keywordEdit_ = new QLineEdit;
    keywordEdit_->setPlaceholderText(tr("Type an ID or the start of a name, then press Enter"));
    keywordEdit_->setClearButtonEnabled(true);
    keywordEdit_->setToolTip(tr("Search keyword (Ctrl+F jumps here)."));
    categoryCombo_ = new QComboBox;
    for (const std::string& name : model::PatientFactory::categories()) categoryCombo_->addItem(qs(name));
    keywordStack_->addWidget(keywordEdit_);
    keywordStack_->addWidget(categoryCombo_);

    searchAlgorithmCombo_ = new QComboBox;
    searchAlgorithmCombo_->addItems({tr("Auto (recommended)"), tr("Linear Search"), tr("Binary Search"),
                                     tr("Recursive Search")});
    searchAlgorithmCombo_->setToolTip(
        tr("Auto picks Binary Search when the records are sorted by the searched field, otherwise Linear Search."));

    searchButton_ = new QPushButton(tr("Search"));
    searchButton_->setToolTip(tr("Search with the chosen algorithm."));
    searchInfo_ = new QLabel;
    searchInfo_->setWordWrap(true);

    searchForm->addRow(tr("Search by:"), searchFieldCombo_);
    searchForm->addRow(tr("Keyword:"), keywordHolder);
    searchForm->addRow(tr("Algorithm:"), searchAlgorithmCombo_);
    searchForm->addRow(searchButton_);
    searchForm->addRow(searchInfo_);

    // ------------------------------------------------------------ sort group
    auto* sortBox = new QGroupBox(tr("Sort patients"));
    auto* sortForm = new QFormLayout(sortBox);
    sortAlgorithmCombo_ = new QComboBox;
    sortAlgorithmCombo_->addItems({tr("Auto (recommended)"), tr("Insertion Sort"), tr("Merge Sort")});
    sortAlgorithmCombo_->setToolTip(
        tr("Insertion Sort: small or nearly sorted lists. Merge Sort: large lists. Auto chooses for you."));
    sortFieldCombo_ = new QComboBox;
    sortFieldCombo_->addItems({tr("Patient Name"), tr("Patient ID"), tr("Age"), tr("Appointment Date")});
    sortFieldCombo_->setToolTip(tr("The field the records are ordered by."));
    sortButton_ = new QPushButton(tr("SORT"));
    sortButton_->setToolTip(tr("Reorder the patient list. Binary search becomes available for the sorted field."));
    sortedLabel_ = new QLabel;
    sortInfo_ = new QLabel;
    sortInfo_->setWordWrap(true);
    sortForm->addRow(tr("Sort Algorithm:"), sortAlgorithmCombo_);
    sortForm->addRow(tr("Sort By:"), sortFieldCombo_);
    sortForm->addRow(sortButton_);
    sortForm->addRow(sortedLabel_);
    sortForm->addRow(sortInfo_);

    auto* top = new QHBoxLayout;  // search and sort side by side
    top->addWidget(searchBox, 1);
    top->addWidget(sortBox, 1);
    root->addLayout(top);

    auto* resultsBar = new QHBoxLayout;
    resultsLabel_ = new QLabel;
    showAllButton_ = new QPushButton(tr("Show All Patients"));
    showAllButton_->setToolTip(tr("Clear the search result and list every patient in the current order."));
    resultsBar->addWidget(resultsLabel_, 1);
    resultsBar->addWidget(showAllButton_);
    root->addLayout(resultsBar);

    table_ = new QTableWidget;
    setupPatientTable(table_);
    root->addWidget(table_, 1);

    connect(searchFieldCombo_, &QComboBox::currentIndexChanged, this, &SearchPanel::onSearchFieldChanged);
    connect(searchButton_, &QPushButton::clicked, this, &SearchPanel::onSearchClicked);
    connect(keywordEdit_, &QLineEdit::returnPressed, this, &SearchPanel::onSearchClicked);  // text field event
    connect(sortButton_, &QPushButton::clicked, this, &SearchPanel::onSortClicked);
    connect(showAllButton_, &QPushButton::clicked, this, &SearchPanel::onShowAllClicked);
}

void SearchPanel::focusSearch() {
    keywordEdit_->setFocus();
    keywordEdit_->selectAll();
}

void SearchPanel::onSearchFieldChanged() {
    const bool category = searchFieldCombo_->currentIndex() == 2;
    keywordStack_->setCurrentIndex(category ? 1 : 0);
}

void SearchPanel::onSearchClicked() {
    runGuarded(this, [&] {
        const auto field = static_cast<SearchField>(searchFieldCombo_->currentIndex());
        const auto algorithm = static_cast<SearchAlgorithm>(searchAlgorithmCombo_->currentIndex());
        const QString keyword = field == SearchField::Category ? categoryCombo_->currentText() : keywordEdit_->text();

        const controller::SearchResult result = manager_.search(field, ss(keyword), algorithm);

        std::vector<const model::PatientRecord*> rows;
        shownIds_.clear();
        for (std::size_t index : result.indices) {
            const model::PatientRecord* patient = manager_.patients()[index].get();
            rows.push_back(patient);
            shownIds_.push_back(patient->id());
        }
        showingAll_ = false;
        showResults(rows);

        searchInfo_->setText(tr("<b>%1</b> used, %2 comparison(s), %3 match(es).<br>%4")
                                 .arg(qs(controller::toString(result.used)))
                                 .arg(result.comparisons)
                                 .arg(result.indices.size())
                                 .arg(qs(result.explanation).toHtmlEscaped()));
        resultsLabel_->setText(tr("Search results: %1 patient(s) found.").arg(result.indices.size()));
        emit statusMessage(tr("Search finished: %1 match(es).").arg(result.indices.size()));
    });
}

void SearchPanel::onSortClicked() {
    const bool ok = runGuarded(this, [&] {
        const auto field = static_cast<SortField>(sortFieldCombo_->currentIndex());
        const auto algorithm = static_cast<SortAlgorithm>(sortAlgorithmCombo_->currentIndex());
        const controller::SortReport report = manager_.sortPatients(field, algorithm);

        sortInfo_->setText(tr("<b>%1</b> sorted %2 record(s) by %3 using %4 comparison(s) in %5 ms.<br>%6")
                               .arg(qs(controller::toString(report.used)))
                               .arg(report.recordCount)
                               .arg(qs(controller::toString(field)))
                               .arg(report.comparisons)
                               .arg(report.milliseconds, 0, 'f', 3)
                               .arg(qs(report.reason).toHtmlEscaped()));
        emit statusMessage(tr("Patients sorted by %1 with %2.")
                               .arg(qs(controller::toString(field)), qs(controller::toString(report.used))));
    });
    if (ok) {
        showingAll_ = true;
        emit dataChanged();
    }
}

void SearchPanel::onShowAllClicked() {
    showingAll_ = true;
    searchInfo_->clear();
    showAll();
}

void SearchPanel::showResults(const std::vector<const model::PatientRecord*>& rows) {
    populatePatientTable(table_, rows);
}

void SearchPanel::showAll() {
    std::vector<const model::PatientRecord*> rows;
    for (const auto& patient : manager_.patients()) rows.push_back(patient.get());
    showResults(rows);
    resultsLabel_->setText(tr("Showing all %1 patient(s) in the current order.").arg(rows.size()));
}

void SearchPanel::updateSortedLabel() {
    const auto sorted = manager_.sortedBy();
    sortedLabel_->setText(sorted ? tr("Records are currently sorted by: <b>%1</b>").arg(qs(controller::toString(*sorted)))
                                 : tr("Records are currently <b>unsorted</b> (Linear Search will be used)."));
}

void SearchPanel::refresh() {
    updateSortedLabel();
    if (showingAll_) {
        showAll();
        return;
    }
    // Keep the search result, but drop patients that no longer exist.
    std::vector<const model::PatientRecord*> rows;
    std::vector<std::string> stillThere;
    for (const std::string& id : shownIds_) {
        if (const model::PatientRecord* patient = manager_.database().patientById(id)) {
            rows.push_back(patient);
            stillThere.push_back(id);
        }
    }
    shownIds_ = stillThere;
    showResults(rows);
}

void SearchPanel::runDemo() {
    sortFieldCombo_->setCurrentIndex(0);
    sortAlgorithmCombo_->setCurrentIndex(0);
    onSortClicked();
    searchFieldCombo_->setCurrentIndex(1);
    keywordEdit_->setText("E");
    searchAlgorithmCombo_->setCurrentIndex(0);
    onSearchClicked();
    updateSortedLabel();
}

} // namespace scapms::gui
