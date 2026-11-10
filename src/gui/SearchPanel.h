// SearchPanel.h - "Search & Sort" tab.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include <string>
#include <vector>

#include "gui/BasePanel.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedLayout;
class QTableWidget;

namespace scapms::gui {

class SearchPanel : public BasePanel {
    Q_OBJECT
public:
    explicit SearchPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);
    void refresh() override;

    /// Moves the keyboard focus to the search box (Ctrl+F).
    void focusSearch();

    /// Runs a sample sort + search (used only when generating documentation screenshots).
    void runDemo();

private slots:
    void onSearchFieldChanged();
    void onSearchClicked();
    void onSortClicked();
    void onShowAllClicked();

private:
    void buildUi();
    void showResults(const std::vector<const model::PatientRecord*>& rows);
    void showAll();
    void updateSortedLabel();

    QComboBox* searchFieldCombo_ = nullptr;
    QStackedLayout* keywordStack_ = nullptr;   // page 0: free text, page 1: category chooser
    QLineEdit* keywordEdit_ = nullptr;
    QComboBox* categoryCombo_ = nullptr;
    QComboBox* searchAlgorithmCombo_ = nullptr;
    QPushButton* searchButton_ = nullptr;
    QLabel* searchInfo_ = nullptr;

    QComboBox* sortFieldCombo_ = nullptr;
    QComboBox* sortAlgorithmCombo_ = nullptr;
    QPushButton* sortButton_ = nullptr;
    QLabel* sortedLabel_ = nullptr;
    QLabel* sortInfo_ = nullptr;

    QPushButton* showAllButton_ = nullptr;
    QLabel* resultsLabel_ = nullptr;
    QTableWidget* table_ = nullptr;

    bool showingAll_ = true;
    std::vector<std::string> shownIds_;  // IDs currently shown when showingAll_ is false
};

} // namespace scapms::gui
