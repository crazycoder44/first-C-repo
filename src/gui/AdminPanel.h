// AdminPanel.h - "Dashboard" tab: overview numbers, undo, recent-patient cache and reminders.
// Author: Linda Obodo (COS202 Lab Assessment)
#pragma once

#include <QStringList>

#include "gui/BasePanel.h"

class QLabel;
class QListWidget;
class QPushButton;

namespace scapms::gui {

class AdminPanel : public BasePanel {
    Q_OBJECT
public:
    explicit AdminPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);
    void refresh() override;

    /// The main window's reminder timer supplies the text of the upcoming appointments.
    void setReminders(const QStringList& reminders);

signals:
    void undoRequested();

private:
    void buildUi();

    QLabel* clockLabel_ = nullptr;
    QLabel* totalLabel_ = nullptr;
    QLabel* categoryLabel_ = nullptr;
    QLabel* appointmentLabel_ = nullptr;
    QLabel* queueLabel_ = nullptr;
    QLabel* undoLabel_ = nullptr;
    QPushButton* undoButton_ = nullptr;
    QLabel* cacheLabel_ = nullptr;
    QListWidget* cacheList_ = nullptr;
    QListWidget* reminderList_ = nullptr;
};

} // namespace scapms::gui
