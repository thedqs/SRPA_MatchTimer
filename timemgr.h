/* 
 * File:   timemgr.h
 * Author: David Springgay
 *
 * Created on January 15, 2022, 2:49 PM
 *
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_TIMEMGR_H
#define	XC_TIMEMGR_H

// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate

typedef enum { TimerState_Paused, TimerState_Starting, TimerState_Running } TimerState;

typedef struct TimerManagerState {
    struct {
        unsigned char Minute;
        unsigned char Second;
    } _CurrentTime;
    
    TimerState _State;
} TimerManagerState;

typedef enum { TimerStatus_Ok, TimerStatus_TimerCompleted, TimerStatus_Error } TimerStatus;

TimerManagerState * CreateTimerManager();
TimerStatus TimerManager_TickSecond(TimerManagerState * state);
void TimerManager_SetTime(TimerManagerState * state, unsigned char minutes, unsigned char seconds);
void TimerManager_AdjustSeconds(TimerManagerState * state, signed char delta_seconds);
void TimerManager_AdjustMinutes(TimerManagerState * state, signed char delta_minutes);
void TimerManager_SetTime(TimerManagerState * state, unsigned char minutes, unsigned char seconds);
void TimerManager_SetTime(TimerManagerState * state, unsigned char minutes, unsigned char seconds);
void TimerManager_Start(TimerManagerState * state);
void TimerManager_Pause(TimerManagerState * state);

// TODO Insert declarations

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

