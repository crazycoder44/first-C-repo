// MainWindow.cpp
// Author: Linda Obodo (COS202 Lab Assessment)
#include "gui/MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>

#include "gui/AdminPanel.h"
#include "gui/AppointmentPanel.h"
#include "gui/GuiHelpers.h"
#include "gui/PatientPanel.h"
#include "gui/QueuePanel.h"
#include "gui/ReportPanel.h"
#include "gui/SearchPanel.h"
#include "utils/DateUtils.h"

namespace scapms::gui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(tr("Smart Clinic Appointment & Patient Management System"));
    resize(1180, 760);

    buildTabs();
    buildMenus();

    statusSummary_ = new QLabel;
    statusBar()->addPermanentWidget(statusSummary_);

    // Load the default data file that sits next to the program (if there is one).
    currentPath_ = QDir(QApplication::applicationDirPath()).filePath("data/clinic_data.csv");
    if (QFileInfo::exists(currentPath_)) {
        // If the file is damaged we show the reason and start with an empty clinic instead of crashing.
        runGuarded(this, [&] { manager_.load(ss(currentPath_)); });
    }

    // TIMER EVENT: fires every second. It updates waiting times, refreshes the dashboard and
    // checks appointment reminders at slower intervals.
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::onTick);
    timer_->start(1000);

    refreshAll();
    checkReminders();
    statusBar()->showMessage(tr("Ready."), 5000);
}

void MainWindow::connectPanel(BasePanel* panel) {
    connect(panel, &BasePanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(panel, &BasePanel::viewChanged, this, &MainWindow::onViewChanged);
    connect(panel, &BasePanel::statusMessage, this, [this](const QString& message) {
        statusBar()->showMessage(message, 6000);
    });
}

void MainWindow::buildTabs() {
    tabs_ = new QTabWidget;
    adminPanel_ = new AdminPanel(manager_);
    patientPanel_ = new PatientPanel(manager_);
    appointmentPanel_ = new AppointmentPanel(manager_);
    queuePanel_ = new QueuePanel(manager_);
    searchPanel_ = new SearchPanel(manager_);
    reportPanel_ = new ReportPanel(manager_);

    tabs_->addTab(adminPanel_, tr("Dashboard"));
    tabs_->addTab(patientPanel_, tr("Patient Records"));
    tabs_->addTab(appointmentPanel_, tr("Appointments"));
    tabs_->addTab(queuePanel_, tr("Waiting Queue"));
    tabs_->addTab(searchPanel_, tr("Search && Sort"));  // && shows a single &
    tabs_->addTab(reportPanel_, tr("Reports"));
    setCentralWidget(tabs_);

    for (BasePanel* panel : std::initializer_list<BasePanel*>{adminPanel_, patientPanel_, appointmentPanel_,
                                                              queuePanel_, searchPanel_, reportPanel_}) {
        connectPanel(panel);
    }
    connect(adminPanel_, &AdminPanel::undoRequested, this, &MainWindow::onUndo);
}

void MainWindow::buildMenus() {
    // MENU EVENTS: every action emits triggered(), which calls one of our slots.
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QAction* newAction = fileMenu->addAction(tr("&New"), this, &MainWindow::onNew);
    newAction->setShortcut(QKeySequence::New);
    QAction* openAction = fileMenu->addAction(tr("&Load / Import..."), this, &MainWindow::onOpen);
    openAction->setShortcut(QKeySequence::Open);
    QAction* saveAction = fileMenu->addAction(tr("&Save"), this, &MainWindow::onSave);
    saveAction->setShortcut(QKeySequence::Save);  // Ctrl+S
    fileMenu->addAction(tr("Save &As / Export..."), this, &MainWindow::onSaveAs);
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));  // Ctrl+Q

    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    undoAction_ = editMenu->addAction(tr("&Undo"), this, &MainWindow::onUndo);
    undoAction_->setShortcut(QKeySequence::Undo);  // Ctrl+Z
    QAction* findAction = editMenu->addAction(tr("&Find Patient"), this, &MainWindow::onFind);
    findAction->setShortcut(QKeySequence::Find);   // Ctrl+F

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, &MainWindow::onAbout);
}

// ------------------------------------------------------------------ menu slots
bool MainWindow::askToSaveChanges() {
    if (!dirty_) return true;
    const auto answer = QMessageBox::question(this, tr("Unsaved changes"),
                                              tr("There are unsaved changes. Save them first?"),
                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (answer == QMessageBox::Cancel) return false;
    if (answer == QMessageBox::Save) {
        onSave();
        return !dirty_;  // still dirty -> saving failed or was cancelled
    }
    return true;
}

void MainWindow::onNew() {
    if (!askToSaveChanges()) return;
    manager_.clearAll();
    remindedAppointments_.clear();
    dirty_ = false;
    refreshAll();
    statusBar()->showMessage(tr("Started a new, empty clinic."), 5000);
}

void MainWindow::onOpen() {
    if (!askToSaveChanges()) return;
    const QString path = QFileDialog::getOpenFileName(this, tr("Load / import patient data"), currentPath_,
                                                      tr("SCAPMS data (*.csv *.txt);;All files (*)"));
    if (path.isEmpty()) return;
    const bool ok = runGuarded(this, [&] { manager_.load(ss(path)); });
    if (ok) {
        currentPath_ = path;
        remindedAppointments_.clear();
        dirty_ = false;
        refreshAll();
        checkReminders();
        statusBar()->showMessage(tr("Loaded %1.").arg(path), 5000);
    }
}

bool MainWindow::saveTo(const QString& path) {
    const bool ok = runGuarded(this, [&] { manager_.save(ss(path)); });
    if (ok) {
        currentPath_ = path;
        dirty_ = false;
        updateTitleAndStatus();
        statusBar()->showMessage(tr("Saved to %1.").arg(path), 5000);
    }
    return ok;
}

void MainWindow::onSave() {
    if (currentPath_.isEmpty()) onSaveAs();
    else saveTo(currentPath_);
}

void MainWindow::onSaveAs() {
    const QString path = QFileDialog::getSaveFileName(this, tr("Save / export patient data"), currentPath_,
                                                      tr("SCAPMS data (*.csv);;Text files (*.txt)"));
    if (!path.isEmpty()) saveTo(path);
}

void MainWindow::onUndo() {
    if (!manager_.canUndo()) {
        statusBar()->showMessage(tr("Nothing to undo."), 4000);
        return;
    }
    std::string what;
    const bool ok = runGuarded(this, [&] { what = manager_.undoLast(); });
    if (ok) {
        dirty_ = true;
        refreshAll();
        statusBar()->showMessage(tr("Undone: %1").arg(qs(what)), 6000);
    }
}

void MainWindow::onFind() {
    tabs_->setCurrentWidget(searchPanel_);
    searchPanel_->focusSearch();
}

void MainWindow::onAbout() {
    QMessageBox::about(this, tr("About SCAPMS"),
                       tr("<b>Smart Clinic Appointment & Patient Management System</b><br>"
                          "COS202 Computer Programming II - Lab Assessment<br>"
                          "Author: Linda Obodo<br><br>"
                          "Shortcuts: Ctrl+S save, Ctrl+F search, Ctrl+Z undo, Ctrl+Q exit."));
}

// ------------------------------------------------------------------ panel events
void MainWindow::onDataChanged() {
    dirty_ = true;
    refreshAll();
}

void MainWindow::onViewChanged() {
    refreshAll();
}

void MainWindow::refreshAll() {
    adminPanel_->refresh();
    patientPanel_->refresh();
    appointmentPanel_->refresh();
    queuePanel_->refresh();
    searchPanel_->refresh();
    reportPanel_->refresh();
    updateTitleAndStatus();
}

void MainWindow::updateTitleAndStatus() {
    const QString file = currentPath_.isEmpty() ? tr("untitled") : QFileInfo(currentPath_).fileName();
    setWindowTitle(tr("SCAPMS - %1%2").arg(file, QString(dirty_ ? "*" : "")));
    undoAction_->setEnabled(manager_.canUndo());
    undoAction_->setText(manager_.canUndo() ? tr("&Undo: %1").arg(qs(manager_.nextUndoDescription())) : tr("&Undo"));
    statusSummary_->setText(tr("Patients: %1  |  Waiting: %2  |  Undo steps: %3")
                                .arg(manager_.patients().size())
                                .arg(manager_.queueSize())
                                .arg(manager_.undoDepth()));
}

// ------------------------------------------------------------------ timer events
void MainWindow::onTick() {
    ++tickCount_;
    queuePanel_->refreshTable();               // every second: waiting times
    if (tickCount_ % 5 == 0) adminPanel_->refresh();  // every 5 s: dashboard clock and statistics
    if (tickCount_ % 30 == 0) checkReminders();       // every 30 s: appointment reminders
}

void MainWindow::checkReminders() {
    const std::string today = utils::todayIso();
    const int now = utils::minutesSinceMidnight();
    QStringList upcoming;
    QStringList fresh;

    for (const model::Appointment& a : manager_.database().appointments()) {
        if (a.status() != model::AppointmentStatus::Scheduled || a.date() != today) continue;
        const int minutes = std::stoi(a.time().substr(0, 2)) * 60 + std::stoi(a.time().substr(3, 2));
        const int until = minutes - now;
        if (until < 0 || until > 30) continue;

        const model::PatientRecord* patient = manager_.database().patientById(a.patientId());
        const QString text = tr("%1 - %2 (%3) in %4 min - %5")
                                 .arg(qs(a.time()), patient ? qs(patient->name()) : QString("?"), qs(a.tag()))
                                 .arg(until)
                                 .arg(qs(a.reason()));
        upcoming << text;
        if (remindedAppointments_.insert(a.id()).second) fresh << text;  // first time we see it
    }
    adminPanel_->setReminders(upcoming);
    if (!fresh.isEmpty()) statusBar()->showMessage(tr("REMINDER: ") + fresh.join("; "), 15000);
}

// ------------------------------------------------------------------ closing
void MainWindow::closeEvent(QCloseEvent* event) {
    if (askToSaveChanges()) event->accept();
    else event->ignore();
}

// ------------------------------------------------------------------ screenshots
void MainWindow::captureScreenshots(const QString& directory) {
    QDir().mkpath(directory);

    // Put the program into an interesting state first.
    try {
        // P0002 is an Emergency patient who joins third but is served first.
        for (const char* id : {"P0001", "P0003", "P0002", "P0005", "P0007"}) {
            try { manager_.addToQueue(id); } catch (const utils::ClinicException&) {}
        }
        manager_.findPatient("P0001");
        manager_.findPatient("P0002");
        manager_.findPatient("P0007");
        manager_.deletePatient("P0012");  // gives the dashboard an "undo" entry to show
    } catch (const std::exception&) {}
    refreshAll();
    patientPanel_->showDemoInput();
    searchPanel_->runDemo();

    const QStringList names = {"01_dashboard", "02_patient_registration", "03_appointments",
                               "04_waiting_queue", "05_search_and_sort", "06_reports"};
    for (int i = 0; i < tabs_->count() && i < names.size(); ++i) {
        tabs_->setCurrentIndex(i);
        QApplication::processEvents();
        grab().save(QDir(directory).filePath(names[i] + ".png"));
    }
    // The demo changes above are for the pictures only: never offer to save them over the data file.
    dirty_ = false;
}

} // namespace scapms::gui
