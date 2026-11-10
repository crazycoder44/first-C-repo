// UndoManager.h - "Undo Last Administrative Operation" built on std::stack.
// Author: Linda Obodo (COS202 Lab Assessment)
//
// Every administrative operation (delete patient, edit patient, cancel appointment) pushes a
// small command object that knows how to reverse it. A stack is the natural choice because
// undo is Last-In-First-Out: the most recent operation is always the first one reversed.
#pragma once

#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "model/ClinicDatabase.h"

namespace scapms::controller {

/// Abstract command: one reversible administrative operation.
class UndoCommand {
public:
    virtual ~UndoCommand() = default;
    virtual void undo(model::ClinicDatabase& database) = 0;
    virtual std::string description() const = 0;
};

/// Reverses "delete patient": puts the patient and their appointments back where they were.
class DeletePatientCommand : public UndoCommand {
public:
    DeletePatientCommand(std::size_t index, std::unique_ptr<model::PatientRecord> patient,
                         std::vector<model::Appointment> appointments);
    void undo(model::ClinicDatabase& database) override;
    std::string description() const override;

private:
    std::size_t index_;
    std::unique_ptr<model::PatientRecord> patient_;
    std::vector<model::Appointment> appointments_;
};

/// Reverses "edit patient": puts the previous version of the record back.
class EditPatientCommand : public UndoCommand {
public:
    explicit EditPatientCommand(std::unique_ptr<model::PatientRecord> previous);
    void undo(model::ClinicDatabase& database) override;
    std::string description() const override;

private:
    std::unique_ptr<model::PatientRecord> previous_;
};

/// Reverses "cancel appointment": the appointment becomes Scheduled again.
class CancelAppointmentCommand : public UndoCommand {
public:
    explicit CancelAppointmentCommand(int appointmentId);
    void undo(model::ClinicDatabase& database) override;
    std::string description() const override;

private:
    int appointmentId_;
};

class UndoManager {
public:
    void push(std::unique_ptr<UndoCommand> command);
    bool canUndo() const { return !history_.empty(); }
    std::size_t size() const { return history_.size(); }
    /// Text such as "Delete patient P0003 (Ada Obi)" or "" when there is nothing to undo.
    std::string nextDescription() const;
    /// Reverses the most recent operation and returns its description. Throws ClinicException if empty.
    std::string undoLast(model::ClinicDatabase& database);
    void clear();

private:
    std::stack<std::unique_ptr<UndoCommand>> history_;  // the STACK
};

} // namespace scapms::controller
