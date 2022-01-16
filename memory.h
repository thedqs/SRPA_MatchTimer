/* 
 * File:   memory.h
 * Author: David Springgay
 *
 * Created on January 15, 2022, 2:49 PM
 *
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MEMORY_H
#define	MEMORY_H

#include <xc.h> // include processor files - each processor file is guarded.  

typedef struct _Time
{
    unsigned char Minutes;
    unsigned char Seconds;
} Time;

typedef enum _Action {
    Nothing,
    ActivateTarget,
    DeactivateTarget
} Action;

#define MAX_NUM_OF_STAGES 4
#define MAX_STAGE_NAME_LEN 15

typedef struct _Stage
{
    char Name[MAX_STAGE_NAME_LEN + 1];
    Time Duration;
    Action OnStart;
    Action OnEnd;
} Stage;

typedef struct _Program
{
    char Name[MAX_STAGE_NAME_LEN + 1];
    unsigned char CountOfStages;
    Stage * StageArray;
} Program;

typedef enum {
    ReadProgramStatus_Success,
    ReadProgramStatus_NoProgram,
    ReadProgramStatus_Error,
    ReadProgramStatus_NeedRoomForStages
} ReadProgramStatus;

ReadProgramStatus MemoryManager_ReadProgram(unsigned char ProgramID, 
        Program * program_info);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* MEMORY_H */

