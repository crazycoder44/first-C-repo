cmake_minimum_required(VERSION 3.16)
project(SCAPMS VERSION 1.0 LANGUAGES CXX)

# Smart Clinic Appointment & Patient Management System (COS202 Lab Assessment)
# Author: Linda Obodo

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(SCAPMS_SOURCES
    src/main.cpp
    # utils
    src/utils/IDGenerator.h
    src/utils/IDGenerator.cpp
    src/utils/FileHandler.h
    src/utils/FileHandler.cpp
    src/utils/ClinicException.h
    src/utils/Algorithms.h
    src/utils/StringUtils.h
    src/utils/DateUtils.h
    # model
    src/model/Appointable.h
    src/model/Appointment.h
    src/model/Appointment.cpp
    src/model/PatientRecord.h
    src/model/PatientRecord.cpp
    src/model/GeneralPatient.h
    src/model/GeneralPatient.cpp
    src/model/EmergencyPatient.h
    src/model/EmergencyPatient.cpp
    src/model/FollowUpPatient.h
    src/model/FollowUpPatient.cpp
    src/model/PatientFactory.h
    src/model/PatientFactory.cpp
    src/model/ClinicDatabase.h
    src/model/ClinicDatabase.cpp
    # controller
    src/controller/RecentPatientCache.h
    src/controller/RecentPatientCache.cpp
    src/controller/UndoManager.h
    src/controller/UndoManager.cpp
    src/controller/SearchEngine.h
    src/controller/SearchEngine.cpp
    src/controller/SortEngine.h
    src/controller/SortEngine.cpp
    src/controller/ReportGenerator.h
    src/controller/ReportGenerator.cpp
    src/controller/AppointmentController.h
    src/controller/AppointmentController.cpp
    src/controller/ClinicManager.h
    src/controller/ClinicManager.cpp
    # gui
    src/gui/GuiHelpers.h
    src/gui/GuiHelpers.cpp
    src/gui/BasePanel.h
    src/gui/AdminPanel.h
    src/gui/AdminPanel.cpp
    src/gui/PatientPanel.h
    src/gui/PatientPanel.cpp
    src/gui/AppointmentPanel.h
    src/gui/AppointmentPanel.cpp
    src/gui/QueuePanel.h
    src/gui/QueuePanel.cpp
    src/gui/SearchPanel.h
    src/gui/SearchPanel.cpp
    src/gui/ReportPanel.h
    src/gui/ReportPanel.cpp
    src/gui/MainWindow.h
    src/gui/MainWindow.cpp
)

add_executable(SCAPMS ${SCAPMS_SOURCES})
target_include_directories(SCAPMS PRIVATE src)
target_link_libraries(SCAPMS PRIVATE Qt6::Widgets)

if(WIN32)
    set_target_properties(SCAPMS PROPERTIES WIN32_EXECUTABLE ON)
endif()

# Copy the sample data file next to the executable so the program can load it.
add_custom_command(TARGET SCAPMS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:SCAPMS>/data
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/data/clinic_data.csv
            $<TARGET_FILE_DIR:SCAPMS>/data/clinic_data.csv
)
