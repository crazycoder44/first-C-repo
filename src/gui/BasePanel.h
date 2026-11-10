// BasePanel.h - common base for every tab of the main window.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// Each panel shows one part of the clinic and talks to the ClinicManager. Panels tell the
// main window about changes through Qt SIGNALS (dataChanged / statusMessage); the main window
// connects them to SLOTS that refresh the other panels. This is Qt's event-driven design.
#pragma once

#include <QWidget>

#include "controller/ClinicManager.h"

namespace scapms::gui {

class BasePanel : public QWidget {
    Q_OBJECT
public:
    explicit BasePanel(controller::ClinicManager& manager, QWidget* parent = nullptr)
        : QWidget(parent), manager_(manager) {}

    /// Re-read the data from the manager and redraw. Must NOT emit dataChanged().
    virtual void refresh() = 0;

signals:
    /// Emitted after the panel changed clinic data (patients, appointments, queue ...).
    void dataChanged();
    /// Something visible changed but no saved data did (e.g. the recent-patient cache).
    void viewChanged();
    /// A short message for the status bar.
    void statusMessage(const QString& message);

protected:
    controller::ClinicManager& manager_;
};

} // namespace scapms::gui
