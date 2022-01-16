/*
 * File:   gui.c
 * Author: thedq
 *
 * Created on January 15, 2022, 12:38 PM
 */

#include "program.h"
#include "memory.h"

ProgramManagerState * CreateProgramManager() {
    ProgramManagerState * state = (ProgramManagerState*)malloc(sizeof(ProgramManagerState));
    if (state != NULL) {
        state->ProgramNumber = 0;
        state->InProgrammingMode = 0;
        state->TargetActive = 0;
        state->TargetOverrideActive = 0;
        state->EditModeActive = 0;
    }
    return state;    
}

void ProgramManager_SetBypassSwitchState(ProgramManagerState * state, 
        unsigned char IsPressed){
    state->TargetOverrideActive = IsPressed;
}

ProgramStatus ProgramManager_SetProgramSwitchState(ProgramManagerState * state,
                unsigned char ProgramId){
    if (state->ProgramNumber != ProgramId) {
        Program potential_program;
        Stage potential_stages[MAX_NUM_OF_STAGES];
        potential_program.CountOfStages = MAX_NUM_OF_STAGES;
        potential_program.StageArray = potential_stages;
        ReadProgramStatus status = MemoryManager_ReadProgram(ProgramId, 
                &potential_program);
    }
    return PROGRAM_STATUS_OK;
}

