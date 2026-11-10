# Smart Clinic Appointment & Patient Management System (SCAPMS)

**Student:** Linda Obodo  **Course:** COS202 – Computer Programming II  **Assessment:** Lab Assessment Project
**Language / tools:** C++17, Qt 6 Widgets, CMake, MinGW-w64 g++ (MSYS2 UCRT64), JetBrains CLion

## 1. Project description

SCAPMS is a desktop application for a university clinic. Staff can register patients, schedule and cancel appointments, manage a waiting queue, search and sort patient records, undo administrative mistakes and produce simple reports. Data is saved to and loaded from a CSV text file (`data/clinic_data.csv`). The program has a tabbed graphical interface with six panels: Dashboard, Patient Records, Appointments, Waiting Queue, Search & Sort and Reports.

## 2. System features

- Register, edit and delete patients (General, Emergency, Follow-Up), with automatic IDs (`P0001`, `P0002`, …).
- Schedule, cancel and complete appointments. Rules depend on the patient type, and a time slot cannot be double-booked.
- Waiting queue. Emergency patients are called before normal patients; inside each group, first come, first served.
- Search by ID, name or category with Linear, Binary or Recursive search, or with "Auto".
- Sort by name, ID, age or appointment date with Insertion Sort or Merge Sort, or with "Auto".
- Undo the last administrative operation (delete patient, edit patient, cancel appointment). Undo can be repeated.
- Frequently-accessed-patients cache with 10 slots, shown on the dashboard.
- Four reports: Most Frequently Attended Patients, Patients with Pending Appointments, Patient Category Distribution and Daily Appointment Summary. Reports can be exported to a text file.
- Save / Load / New / Exit menu, file dialogs, keyboard shortcuts (Ctrl+S, Ctrl+F, Ctrl+Z, Ctrl+Q, Ctrl+O, Ctrl+N), tooltips and validation dialogs.
- Timers for the live clock, dashboard refresh, waiting-time alerts and appointment reminders.

## 3. Class hierarchy and program organisation

The code is split into four namespaces and folders (see `UML_Class_Diagram.md`):

| Layer | Namespace / folder | Main classes |
|---|---|---|
| Model (data) | `scapms::model` – `src/model` | `Appointable`, `PatientRecord`, `GeneralPatient`, `EmergencyPatient`, `FollowUpPatient`, `Appointment`, `ClinicDatabase`, `PatientFactory` |
| Controller (business logic) | `scapms::controller` – `src/controller` | `ClinicManager`, `AppointmentController`, `SearchEngine`, `SortEngine`, `UndoManager` (+ command classes), `RecentPatientCache`, `ReportGenerator` |
| GUI | `scapms::gui` – `src/gui` | `MainWindow`, `BasePanel`, `AdminPanel`, `PatientPanel`, `AppointmentPanel`, `QueuePanel`, `SearchPanel`, `ReportPanel` |
| Utilities | `scapms::utils` – `src/utils` | `IDGenerator`, `FileHandler`, `ClinicException` hierarchy, `Algorithms.h` |

- **Abstract class:** `PatientRecord` holds the ID, name, age and contact. It declares pure virtual functions such as `category()`, `priority()`, `consultationPlan()` and `clone()`.
- **Interface:** `Appointable` contains only pure virtual functions: `scheduleAppointment()`, `cancelAppointment()` and `appointmentIds()`.
- **Subclasses:** each subclass provides its own behaviour. An Emergency patient has a severity from 1 to 5, priority 10 + severity, and URGENT appointments. A Follow-Up patient cannot be booked before their previous visit. A General patient gets the standard booking.
- **Polymorphism:** `ClinicManager::processPatient(const PatientRecord&)` and `AppointmentController::schedule()` work only with the base class. The correct subclass function is chosen at run time through virtual dispatch. The Patient Records and Waiting Queue panels display the result.
- **Encapsulation and composition:** all fields are private, and setters validate their input. `ClinicDatabase` owns its patients (`std::vector<std::unique_ptr<PatientRecord>>`) and appointments. Each patient is associated with one or more appointments through their IDs.
- The GUI never changes data directly. Panels call `ClinicManager` and report changes through Qt **signals**, so presentation, business logic and file handling stay in separate classes.

## 4. Data structures used

| Structure | Where | Why |
|---|---|---|
| `std::vector` | `ClinicDatabase` stores all patients and appointments | Fast indexed access, which binary search and the sorting algorithms need |
| `std::queue` | `ClinicManager` waiting lists (one urgent, one normal) | FIFO: the patient who joins first is served first |
| `std::stack` | `UndoManager` history of `UndoCommand` objects | LIFO: the most recent operation is the first one undone |
| Fixed-size array | `RecentPatientCache`: `const PatientRecord* slots_[10]` | Small, constant-size cache. The most recently used record moves to slot 0 and the least recently used record drops out |

## 5. Searching algorithms (all hand-written, in `SearchEngine.cpp`)

- **Linear search** (by ID, name prefix or category) checks every record in turn, so it costs O(n). It works on unsorted data.
- **Binary search** (by ID, or by name prefix) halves the search range each step, so it costs O(log n). For a name prefix, it first finds the first name that is ≥ the prefix, then reads the neighbouring names that match. The program refuses binary search with a clear error message unless the records are sorted by the searched field.
- **Recursive search** checks record *i*, then calls itself for record *i + 1*. The base case is reaching the end of the list. It collects every match (ID, name or category).
- **Automatic choice:** "Auto" uses binary search when the data is sorted by the searched field and linear search otherwise. This follows the brief's rule of choosing the technique from the state of the data. After each search the GUI shows which algorithm ran and how many comparisons it made.

## 6. Sorting algorithms (hand-written templates in `Algorithms.h`)

- **Insertion Sort** is O(n²) in the worst case but close to O(n) on nearly sorted data. It is stable, needs no extra memory and has very little overhead. *Chosen for* small lists and lists that are already almost in order, which describes most clinic lists.
- **Merge Sort** is O(n log n) in every case and stable, and it needs O(n) extra memory. *Chosen for* large or badly ordered lists, where insertion sort becomes slow.
- **Auto mode** counts how many neighbouring records are out of order. It uses Insertion Sort when the list has ≤ 20 records or fewer than 10 % of neighbours are out of order, and Merge Sort otherwise.
- After each sort the GUI shows the algorithm used, the number of comparisons, the time taken and a sentence explaining why that algorithm suits the data. Both algorithms are **stable**. This matters because sorting by age after sorting by name keeps patients of the same age in alphabetical order.
- Merge Sort is also reused to rank patients in the "Most Frequently Attended" report.

## 7. Recursive algorithm

The program has two recursive components:
- `SearchEngine::recursiveSearch()`, the recursive scan described in section 5.
- `detail::mergeSortRange()`, which sorts the left half and the right half by calling itself and then merges them. Its base case is a range of 0 or 1 elements.

## 8. GUI design

- **Components:** the GUI uses
  - tables (`QTableWidget`)
  - buttons and text fields
  - combo boxes, including the algorithm dropdowns
  - labels and a spin box
  - list views
  - message and file dialogs
  - a menu bar and a status bar
- **Layouts:** at least five Qt layouts are used, and none of the positioning is fixed:
  - `QVBoxLayout` stacks each panel's sections top to bottom.
  - `QHBoxLayout` places button rows and the search/sort boxes side by side.
  - `QFormLayout` holds the label–field pairs in the registration and search forms, the natural layout for forms.
  - `QGridLayout` arranges the appointment booking fields in columns and the 2×2 dashboard grid. Both resize with the window.
  - `QStackedLayout` swaps the keyword box for a category chooser when searching by category.
- **Event-driven programming:** button clicks, menu actions, shortcuts, combo-box changes and text edits are connected to slots with Qt **signals and slots**. Text edits include the live "Patient ID already exists" warning and Enter-to-search. A one-second `QTimer` updates waiting times. Every 5 s it refreshes the dashboard, and every 30 s it checks appointment reminders.
- **Advanced techniques:**
  - (a) Custom table presentation: Emergency rows are red and bold, General rows green and Follow-Up rows blue. Appointment rows are coloured by status.
  - (b) Dynamic components: the registration form adds and removes fields to match the selected patient type.
  - (c) File dialogs for loading, saving and exporting.
  - (d) Timers.
  - (e) Input-validation dialogs.
  - (f) Keyboard shortcuts.
  - (g) Tooltips on all important controls.

## 9. Exception handling approach

- All deliberate errors derive from a custom base class `utils::ClinicException`. The subclasses are `InvalidPatientException`, `InvalidAgeException`, `DuplicateIdException`, `InvalidAppointmentException`, `NotFoundException`, `FileException` and `InvalidSearchException`.
- The model and controller **throw**. The GUI wraps every user action in `runGuarded()`, a helper with `try { … } catch (const ClinicException&) … catch (const std::exception&) … catch (...)`. A meaningful dialog appears, for example "Error: Patient age must be between 0 and 120." or "Error: Patient ID 'P0001' already exists.", and the program keeps running.
- Operations are **safe on failure**:
  - An edit is applied to a copy of the record first.
  - A file is loaded into a separate database and only replaces the current data if the whole file is valid. Otherwise the error message gives the bad line number.
  - An ID is only used up once a record has actually been stored.

## 10. Challenges encountered and how they were resolved

1. **Compiler crash on Windows.** The MinGW compiler first failed with no error message. The cause was PostgreSQL, which is on the system PATH and ships its own older copies of `zlib1.dll` and `libzstd.dll`. The compiler loaded those instead of the MSYS2 versions. *Resolved* by putting `C:\msys64\ucrt64\bin` first on PATH in the `build.bat`/`run.bat` scripts and in the CLion toolchain.
2. **Dangling pointers in the cache.** The array cache holds raw pointers into the database, so deleting or editing a patient could leave a pointer to freed memory. *Resolved* by having `ClinicManager` invalidate the cache before every delete or replace, and clear it after an undo.
3. **Undo that keeps data consistent.** Deleting a patient also removes their appointments and queue entry. *Resolved* with the Command pattern: each undo command stores everything needed to reverse the operation, including the patient's original position and appointments.
4. **Gaps in generated IDs.** At first, a registration that failed validation still used up an ID. *Resolved* by previewing the next ID and committing it only after the record is stored.
5. **Binary search on unsorted data gives wrong answers.** *Resolved* by tracking which field the list is sorted by. Any change to the data resets that state, binary search is only allowed when the data is sorted by the searched field, and "Auto" falls back to linear search.

## 11. Testing

- The full application builds with no warnings, and the model, controller and utility code also compiles with `-Wall -Wextra` with no warnings.
- The core logic was checked with a test program that covered:
  - validation errors
  - duplicate IDs
  - double bookings
  - undo of delete, edit and cancel
  - queue order
  - agreement between the search algorithms
  - identical results from both sorting algorithms
  - CSV save/load round-trips, including commas and quotes in fields
  - rejection of corrupt files
- The GUI was run and every tab was captured (see the screenshots).
