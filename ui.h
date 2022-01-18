/* 
 * File:   ui.h
 * Author: David Springgay
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef UI_H
#define	UI_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "timemgr.h"
#include "buttons.h"
#include "program.h"

// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate

// TODO Insert declarations

#define MAX_MESSAGE_LENGTH 128

typedef struct _UiState {
    char LEDsDisplaying[4];
    char MessageBuffer[MAX_MESSAGE_LENGTH + 1];
    int CurrentLED;
    char * MessagePointer;
    char * MessageEndPointer;
    long update_count;
} UiState;

void InitializeUiComponentState(UiState * state);
void PrintMessage(UiState * gui, char * msg);
void UiUpdate(UiState * gui, TimerManagerState * timer, 
        ButtonManagerState * buttons, ProgramManagerState * program);

// Comment a function and leverage automatic documentation with slash star star
/**
    <p><b>Function prototype:</b></p>
  
    <p><b>Summary:</b></p>

    <p><b>Description:</b></p>

    <p><b>Precondition:</b></p>

    <p><b>Parameters:</b></p>

    <p><b>Returns:</b></p>

    <p><b>Example:</b></p>
    <code>
 
    </code>

    <p><b>Remarks:</b></p>
 */
// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

