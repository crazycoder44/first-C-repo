// main.cpp - entry point of the Smart Clinic Appointment & Patient Management System (SCAPMS).
// Author: Linda Obodo (COS202 Lab Assessment)
//
// Run normally:                    SCAPMS.exe
// Create documentation screenshots: SCAPMS.exe --screenshots <folder>
#include <QApplication>
#include <QStringList>
#include <QTimer>

#include "gui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("SCAPMS");
    QApplication::setStyle("Fusion");  // same look on every machine

    scapms::gui::MainWindow window;
    window.show();

    const QStringList args = app.arguments();
    const int option = args.indexOf("--screenshots");
    if (option >= 0 && option + 1 < args.size()) {
        const QString directory = args.at(option + 1);
        QTimer::singleShot(400, &window, [&window, directory, &app] {
            window.captureScreenshots(directory);
            app.quit();
        });
    }
    return app.exec();
}
