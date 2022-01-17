/*
 * File:   gui.c
 * Author: thedq
 *
 * Created on January 15, 2022, 12:38 PM
 */

#include "program.h"
#include "memory.h"

void InitializeProgramManager(ProgramManagerState * state) {
    state->ProgramNumber = 0;
    state->InProgrammingMode = 0;
    state->TargetActive = 0;
    state->TargetOverrideActive = 0;
    state->EditModeActive = 0;
}

void ProgramManager_SetBypassSwitchState(ProgramManagerState * state, 
        unsigned char IsPressed){
    state->TargetOverrideActive = IsPressed;
}

void ProgramManager_SetProgramSwitchState(ProgramManagerState * state, 
        TimerManagerState * timer, unsigned char ProgramId){
    if (state->ProgramNumber != ProgramId) {
        Program potential_program;
        Stage potential_stages[MAX_NUM_OF_STAGES];
        potential_program.CountOfStages = MAX_NUM_OF_STAGES;
        potential_program.StageArray = potential_stages;
        ReadProgramStatus status = MemoryManager_ReadProgram(ProgramId, 
                &potential_program);
    }
}

void ProgramManager_ToggleEditState(ProgramManagerState * state) {
    state->EditModeActive = !state->EditModeActive;
}

void ProgramManager_LoadProgram(ProgramManagerState * state, 
        TimerManagerState * timer) {
    
}