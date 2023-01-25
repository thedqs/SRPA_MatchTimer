/*
 * File:   gui.c
 * Author: thedq
 *
 */

#include "timemgr.h"
#include <stdlib.h>

void InitializeTimerManager(TimerManagerState * state) {
    state->_CurrentTime.Minute = 0;
    state->_CurrentTime.Second = 0;
    state->_State = TimerState_Paused;
}

TimerStatus TimerManager_TickSecond(TimerManagerState * state){
    // We have a starting state to make sure that we get an entire second before
    // removing a second, better to give an extra 999 milliseconds instead of
    // losing the 999 milliseconds
    if (state->_State == TimerState_Starting) {
        state->_State = TimerState_Running;
    }
    else if (state->_State == TimerState_Running) {
        signed char seconds = ((signed char)state->_CurrentTime.Second) - 1;
        signed char minutes = (signed char)state->_CurrentTime.Minute;
        if (seconds == 0 && minutes == 0) {
            state->_CurrentTime.Second = 0;
            state->_CurrentTime.Minute = 0;
            state->_State = TimerState_Paused;
            return TimerStatus_TimerCompleted;
        }
        else if (seconds < 0) {
            minutes--;
            if (minutes < 0) {
                // Should never hit, but just in case
                state->_CurrentTime.Second = 0;
                state->_CurrentTime.Minute = 0;
                state->_State = TimerState_Paused;
                return TimerStatus_TimerCompleted;
            }
            else {
                seconds = 59;
            }                
        }
        
        state->_CurrentTime.Second = (unsigned char)seconds;
        state->_CurrentTime.Minute = (unsigned char)minutes;
    }
    return TimerStatus_Ok;
}

void TimerManager_AdjustSeconds(TimerManagerState * state, signed char delta_seconds) {
    // Prevent the adjusting of time while the timer is running
    if (state->_State == TimerState_Running || 
            state->_State == TimerState_Starting) {
        return;
    }
    
    // We use a short for seconds to prevent overflow arithmetic
    short seconds = (short)state->_CurrentTime.Second;
    signed char delta_minutes = 0;
    seconds += delta_seconds;
    while (seconds >= 60) {
        seconds -= 60;
        delta_minutes += 1;
    }
    
    while (seconds < 0) {
        seconds += 60;
        delta_minutes -= 1;
    }
    
    if (delta_minutes != 0) {
        TimerManager_AdjustMinutes(state, delta_minutes);
    }
    
    // seconds is guaranteed to be between 0 and 60
    state->_CurrentTime.Second = (unsigned char)seconds;
}

void TimerManager_AdjustMinutes(TimerManagerState * state, signed char delta_minutes){
    // Prevent the adjusting of time while the timer is running
    if (state->_State == TimerState_Running || 
            state->_State == TimerState_Starting) {
        return;
    }
    
    short minutes = (short)state->_CurrentTime.Minute;
    minutes += delta_minutes;
    
    // We max it out at 99 minutes because we only have 2 digits
    if (minutes > 99) {
        minutes = 99;
    }
    
    if (minutes >= 0) {
        state->_CurrentTime.Minute = (unsigned char)minutes;
    }
}

void TimerManager_SetTime(TimerManagerState * state, unsigned char minutes, unsigned char seconds){
    TimerManager_Pause(state);
    state->_CurrentTime.Second = seconds;
    state->_CurrentTime.Minute = minutes;
}

void TimerManager_Start(TimerManagerState * state){
    if (state->_State == TimerState_Paused) {
        state->_State = TimerState_Starting;
    }
}

void TimerManager_Pause(TimerManagerState * state){
    if (state->_State == TimerState_Running || state->_State == TimerState_Starting) {
        state->_State = TimerState_Paused;
    }
}

