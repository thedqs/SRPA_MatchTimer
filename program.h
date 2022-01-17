/* 
 * File:   program.h
 * Author: David Springgay
 *
 * Created on January 15, 2022, 9:42 PM
 */

#ifndef PROGRAM_H
#define	PROGRAM_H

#include "timemgr.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char ProgramNumber;
    unsigned char InProgrammingMode;
    unsigned char TargetActive;
    unsigned char TargetOverrideActive;
    unsigned char EditModeActive;
} ProgramManagerState;

void InitializeProgramManager(ProgramManagerState * state);
void ProgramManager_SetBypassSwitchState(ProgramManagerState * state, 
        unsigned char IsPressed);
void ProgramManager_SetProgramSwitchState(ProgramManagerState * state, 
        TimerManagerState * timer, unsigned char ProgramId);
void ProgramManager_ToggleEditState(ProgramManagerState * state);
void ProgramManager_LoadProgram(ProgramManagerState * state, 
        TimerManagerState * timer);

#ifdef	__cplusplus
}
#endif

#endif	/* PROGRAM_H */

