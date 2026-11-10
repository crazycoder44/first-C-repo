// MainWindow.h - the application window: menus, tabs, status bar and timers.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include <set>

#include <QMainWindow>
#include <QString>

#include "controller/ClinicManager.h"

class QAction;
class QCloseEvent;
class QLabel;
class QTabWidget;
class QTimer;

namespace scapms::gui {

class AdminPanel;
class AppointmentPanel;
class BasePanel;
class PatientPanel;
class QueuePanel;
class ReportPanel;
class SearchPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

    /// Saves a PNG of every tab into `directory` (used to produce the screenshots for the report).
    void captureScreenshots(const QString& directory);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // menu / shortcut events
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onUndo();
    void onFind();
    void onAbout();
    // panel events
    void onDataChanged();   // clinic data changed -> mark unsaved and refresh everything
    void onViewChanged();   // only the view changed -> refresh everything
    // timer events
    void onTick();

private:
    void buildMenus();
    void buildTabs();
    void refreshAll();
    void updateTitleAndStatus();
    bool saveTo(const QString& path);
    bool askToSaveChanges();
    void checkReminders();
    void connectPanel(BasePanel* panel);

    controller::ClinicManager manager_;
    QString currentPath_;
    bool dirty_ = false;
    int tickCount_ = 0;
    std::set<int> remindedAppointments_;

    QTabWidget* tabs_ = nullptr;
    AdminPanel* adminPanel_ = nullptr;
    PatientPanel* patientPanel_ = nullptr;
    AppointmentPanel* appointmentPanel_ = nullptr;
    QueuePanel* queuePanel_ = nullptr;
    SearchPanel* searchPanel_ = nullptr;
    ReportPanel* reportPanel_ = nullptr;

    QAction* undoAction_ = nullptr;
    QLabel* statusSummary_ = nullptr;
    QTimer* timer_ = nullptr;
};

} // namespace scapms::gui
