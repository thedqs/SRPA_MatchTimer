/*
 * File:   ui.c
 * Author: thedq
 *
 * Created on January 15, 2022, 12:38 PM
 */

#include <string.h>

#include "program.h"
#include "ui.h"
#include "memory.h"

Program active_program;
Stage active_program_stages[MAX_NUM_OF_STAGES];
Stage * active_stage = NULL;
unsigned char active_stage_index = 0;
unsigned char program_started = 0;

void InitializeProgramManager(ProgramManagerState * state) {
    state->ProgramNumber = 0;
    state->InProgrammingMode = 0;
    state->TargetActive = 0;
    state->TargetOverrideActive = 0;
    state->EditModeActive = 0;
    active_program.StageArray = active_program_stages;
    active_program.CountOfStages = 0;
    active_stage_index = 0;
    active_stage = NULL;
    program_started = 0;
}

void ProgramManager_SetBypassSwitchState(ProgramManagerState * state, 
        unsigned char IsPressed){
    state->TargetOverrideActive = IsPressed;
}

void ProgramManager_SetProgramSwitchState(ProgramManagerState * state, 
        TimerManagerState * timer, UiState * ui, unsigned char ProgramId){
    if (state->ProgramNumber != ProgramId) {
        
        if (state->EditModeActive) {
            // Todo: Handle editing
        }
        else if (timer->_State == TimerState_Paused) {
            ClearMessage(ui);
            state->ProgramNumber = ProgramId;
            ProgramManager_LoadProgram(state, timer, ui);
        }
    }
}

void ProgramManager_ToggleEditState(ProgramManagerState * state) {
    state->EditModeActive = !state->EditModeActive;
}

void ProgramManager_LoadProgram(ProgramManagerState * state, 
        TimerManagerState * timer, UiState * ui) {
    Program potential_program;
    Stage potential_stages[MAX_NUM_OF_STAGES];
    potential_program.CountOfStages = MAX_NUM_OF_STAGES;
    potential_program.StageArray = potential_stages;
    ReadProgramStatus status = MemoryManager_ReadProgram(state->ProgramNumber, 
            &potential_program);
    
    active_stage_index = 0;
    active_stage = NULL;
    program_started = 0;
    state->TargetActive = 0;
    if (status == ReadProgramStatus_Success) {
        memcpy(&active_program, &potential_program, sizeof(potential_program));
        memcpy(active_program_stages, potential_stages, 
            sizeof(potential_stages));
        active_program.StageArray = active_program_stages;
        active_stage = active_program.CountOfStages > 0 ?
            active_program.StageArray : NULL;
        
        if (active_stage != NULL) {
            TimerManager_SetTime(timer, active_stage->Duration.Minutes, 
                    active_stage->Duration.Seconds);
        }
        else {
            TimerManager_SetTime(timer, 0, 0);
        }
    }
    else if (status == ReadProgramStatus_NoProgram) {
        TimerManager_SetTime(timer, 0, 0);
        if (state->ProgramNumber == 0xE)
        {
            memcpy(active_program.Name, "Configuration", 13);
            active_program.CountOfStages = 0;
            active_program_stages[0].Name[0] = '\0';
        }
        else
        {
            memcpy(active_program.Name, "Manual", 7);
            active_program.CountOfStages = 1;
            memcpy(active_program_stages[0].Name, "Fire", 5);
        }
        active_program_stages[0].Duration.Minutes = 0;
        active_program_stages[0].Duration.Seconds = 0;
        active_program_stages[0].OnStart = ActivateTarget;
        active_program_stages[0].OnEnd = DeactivateTarget;
        active_stage = active_program_stages;
        TimerManager_SetTime(timer, 0, 0);
    }
    else {
        PrintMessage(ui, "Error in Program loading");
        active_program.CountOfStages = 1;
        active_program.Name[0] = '\0';
        active_program_stages[0].Name[0] = '\0';
        active_program_stages[0].Duration.Minutes = 0;
        active_program_stages[0].Duration.Seconds = 0;
        active_program_stages[0].OnStart = ActivateTarget;
        active_program_stages[0].OnEnd = DeactivateTarget;
        active_stage = active_program_stages;
        TimerManager_SetTime(timer, 0, 0);
    }
    PrintMessage(ui, active_program.Name);
}

void ProgramManager_PauseProgram(ProgramManagerState * state, 
        TimerManagerState * timer) {
    TimerManager_Pause(timer);
    state->TargetActive = 0;
}

void ProgramManager_ResumeProgram(ProgramManagerState * state, 
        TimerManagerState * timer, UiState * ui) {
    if (active_stage != NULL) {
        if (!program_started) {
            // Display the stage name and run the action, if any
            program_started = 1;
            PrintMessage(ui, active_stage->Name);
            ProgramManager_ExecuteAction(state, active_stage->OnStart);
        }
        
        if (active_stage->OnStart == ActivateTarget) {
            // When we paused the program we turned the targets away
            state->TargetActive = 1;
        }
        
        TimerManager_Start(timer);
    }
}

void ProgramManager_SignalStageComplete(ProgramManagerState * state, 
        TimerManagerState * timer, UiState * ui) {
    // Perform the final action of the stage
    if (active_stage == NULL) {
        return;
    }
    
    ProgramManager_ExecuteAction(state, active_stage->OnEnd);
    // Advance to next stage (if any)
    if (active_stage_index < active_program.CountOfStages - 1) {
        // Perform the start action
        active_stage = &active_program.StageArray[++active_stage_index];
        // Set Timer and Start
        TimerManager_SetTime(timer, active_stage->Duration.Minutes, 
                active_stage->Duration.Seconds);
        PrintMessage(ui, active_stage->Name);
        ProgramManager_ExecuteAction(state, active_stage->OnStart);
        TimerManager_Start(timer);
    }
}

void ProgramManager_ExecuteAction(ProgramManagerState * state, Action action) {
    switch (action) {
        case ActivateTarget:
            state->TargetActive = 1;
            break;
        case DeactivateTarget:
            state->TargetActive = 0;
            break;
        case Nothing:
        default:
            break;
    }
}