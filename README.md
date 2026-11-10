# SCAPMS – Smart Clinic Appointment & Patient Management System

COS202 Computer Programming II – Lab Assessment · Author: **Linda Obodo**
C++17 · Qt 6 Widgets · CMake · MinGW-w64 (MSYS2 UCRT64)

## Build and run (no IDE needed)

Double-click, or run from a terminal in this folder:

```
build.bat      (compiles; the first run takes a minute)
run.bat        (starts the program)
```

The program loads `build\data\clinic_data.csv` (sample data: 12 patients, 16 appointments) when it starts.

> Why the .bat files? PostgreSQL is installed on this PC and its folder on the system PATH contains older copies of `zlib1.dll` and `libzstd.dll`. Windows found those first and the compiler crashed. Both scripts put `C:\msys64\ucrt64\bin` first on PATH for that window only, so nothing on your system is changed.

## Open in CLion (the IDE from the lab manual)

1. **File → Settings → Build, Execution, Deployment → Toolchains.**
2. Click **+** and choose **MinGW**.
3. In **Toolset**, click the folder icon and select `C:\msys64\ucrt64`. CLion fills in the compiler, CMake and debugger by itself. Click **Apply**.
4. Use the arrows to move this toolchain to the top of the list, so it becomes the default.
5. **File → Open** this `SCAPMS` folder, which contains `CMakeLists.txt`.
6. **Run → Edit Configurations → SCAPMS → Environment variables:** add `PATH=C:\msys64\ucrt64\bin;$PATH$` (this avoids the PostgreSQL DLL clash).
7. Press the green **Run** ▶ button.

We don't use CLion's *Bundled MinGW* because Qt was built with the MSYS2 compiler. Mixing two different compilers causes link errors.

## Shortcuts

Ctrl+S save · Ctrl+O load · Ctrl+N new · Ctrl+F search · Ctrl+Z undo · Ctrl+Q exit

## Screenshots for the report

`run.bat --screenshots docs\screenshots` puts the program in a demo state, saves one PNG per tab, and exits without changing the data file.

## Folder layout

```
SCAPMS/
├── CMakeLists.txt, build.bat, run.bat
├── data/clinic_data.csv            sample data (copied next to the .exe on build)
├── docs/
│   ├── Project_Report.md           2–3 page report
│   ├── UML_Class_Diagram.md        Mermaid class diagram
│   └── screenshots/                01_dashboard.png … 06_reports.png
└── src/
    ├── main.cpp
    ├── model/       PatientRecord (abstract), General/Emergency/FollowUpPatient, Appointable,
    │                Appointment, ClinicDatabase, PatientFactory
    ├── controller/  ClinicManager, AppointmentController, SearchEngine, SortEngine,
    │                UndoManager, RecentPatientCache, ReportGenerator
    ├── gui/         MainWindow, BasePanel, AdminPanel, PatientPanel, AppointmentPanel,
    │                QueuePanel, SearchPanel, ReportPanel, GuiHelpers
    └── utils/       IDGenerator, FileHandler, ClinicException, Algorithms.h (sorts),
                     StringUtils.h, DateUtils.h
```

## Data file format (CSV)

```
PATIENT,<category>,<id>,<name>,<age>,<contact>,<extra1>,<extra2>
APPOINTMENT,<number>,<patientId>,<YYYY-MM-DD>,<HH:MM>,<reason>,<Scheduled|Completed|Cancelled>,<tag>
```
Extra fields: General = complaint · Emergency = severity (1–5), triage note · Follow-Up = previous visit date, doctor.
