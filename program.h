/* 
 * File:   program.h
 * Author: David Springgay
 *
 * Created on January 15, 2022, 9:42 PM
 */

#ifndef PROGRAM_H
#define	PROGRAM_H

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

typedef enum { 
    PROGRAM_STATUS_OK, 
    PROGRAM_STATUS_RELOAD
} ProgramStatus;

ProgramManagerState * CreateProgramManager();
void ProgramManager_SetBypassSwitchState(ProgramManagerState * state, 
        unsigned char IsPressed);
ProgramStatus ProgramManager_SetProgramSwitchState(ProgramManagerState * state,
                unsigned char ProgramId);

#ifdef	__cplusplus
}
#endif

#endif	/* PROGRAM_H */

