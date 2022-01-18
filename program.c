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
            state->ProgramNumber = ProgramId;
            ProgramManager_LoadProgram(state, timer);
        }
    }
}

void ProgramManager_ToggleEditState(ProgramManagerState * state) {
    state->EditModeActive = !state->EditModeActive;
}

void ProgramManager_LoadProgram(ProgramManagerState * state, 
        TimerManagerState * timer) {
    Program potential_program;
    Stage potential_stages[MAX_NUM_OF_STAGES];
    potential_program.CountOfStages = MAX_NUM_OF_STAGES;
    potential_program.StageArray = potential_stages;
    ReadProgramStatus status = MemoryManager_ReadProgram(state->ProgramNumber, 
            &potential_program);
    
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
        memcpy(active_program.Name, "Manual", 7);
        active_program.CountOfStages = 1;
        memcpy(active_program_stages[0].Name, "Fire", 5);
        active_program_stages[0].Duration.Minutes = 0;
        active_program_stages[0].Duration.Seconds = 0;
        active_program_stages[0].OnStart = ActivateTarget;
        active_program_stages[0].OnEnd = DeactivateTarget;
        TimerManager_SetTime(timer, 0, 0);
    }
}

void ProgramManager_PauseProgram(ProgramManagerState * state, 
        TimerManagerState * timer) {
    TimerManager_Pause(timer);
}

void ProgramManager_ResumeProgram(ProgramManagerState * state, 
        TimerManagerState * timer) {
    TimerManager_Start(timer);
}

void ProgramManager_SignalStageComplete(ProgramManagerState * state, 
        TimerManagerState * timer, UiState * ui) {
    // Perform the final action of the stage
    // Advance to next stage (if any)
    // Perform the start action
    // Set Timer and Start
}
