/*
 * File:   buttons.c
 * Author: David Springgay
 *
 * Created on January 15, 2022, 12:38 PM
 */

#include "buttons.h"
#include <stdlib.h>

// [Min Up][Min Down][Sec Up][Sec Down][Reset][Start/Pause][Bypass][0:4 Selector Value]
#define MIN_UP      10
#define MIN_DOWN    9
#define SEC_UP      8
#define SEC_DOWN    7
#define RESET       6
#define START_PAUSE 5
#define BYPASS      4

#define BUTTON_PRESSED(state, btn) ((state >> btn) & 0x1)

void InitializeButtonManager(ButtonManagerState * state){
    state->buttons_state = 0;
    state->bypass_state = 0;
    state->ticks_since_bypass_change = 0;
    state->ticks_since_button_change = 0;
    state->ticks_since_previous_click = 0;
}

void ButtonManager_ReadButtons(ButtonManagerState * state, unsigned char portA, 
        unsigned char portB, unsigned char portC, unsigned char portE) {
    // Bit layout of Return Value
    // [Min Up][Min Down][Sec Up][Sec Down][Reset][Start/Pause][Bypass][0:4 Selector Value]
    ButtonMap button_value = 0;
    button_value |= (((unsigned int)portB >> 2) & 0xF);
    button_value |= ((unsigned int)portE & 0x7) << 4;
    button_value |= (((unsigned int)portA >> 4) & 0x3) << 7;
    button_value |= (((unsigned int)portC >> 2) & 0x3) << 9;
    
    if (state->bypass_state != BUTTON_PRESSED(button_value, BYPASS)) {
        state->ticks_since_bypass_change = 0;
        state->bypass_state = BUTTON_PRESSED(button_value, BYPASS);
    }
    else {
        state->ticks_since_bypass_change += 1;
    }        
    
    if (state->buttons_state == button_value) {
        state->ticks_since_button_change += 1;
    }
    else {
        state->buttons_state = button_value;
        state->ticks_since_button_change = 0;
    }
}

ButtonStatus ButtonManager_ButtonStatus(ButtonManagerState * state, 
        ButtonEnum button){
    // Special case for bypass switch to ignore any other button presses
    if (button == ButtonEnum_Bypass && state->ticks_since_bypass_change > 500) {
        return state->bypass_state ? ButtonStatus_ButtonPressed : 
            ButtonStatus_ButtonNotPressed;
    }
    else if (state->ticks_since_button_change < 500) {
        return ButtonStatus_ButtonNotPressed;
    }
    else {
        switch (button) {
            case ButtonEnum_SecondUp:
                return BUTTON_PRESSED(state->buttons_state, SEC_UP) ? 
                    ButtonStatus_ButtonPressed : ButtonStatus_ButtonNotPressed;
            case ButtonEnum_SecondDown:
                return BUTTON_PRESSED(state->buttons_state, SEC_DOWN) ? 
                    ButtonStatus_ButtonPressed : ButtonStatus_ButtonNotPressed;
            case ButtonEnum_MinuteUp:
                return BUTTON_PRESSED(state->buttons_state, MIN_UP) ? 
                    ButtonStatus_ButtonPressed : ButtonStatus_ButtonNotPressed;
            case ButtonEnum_MinuteDown:
                return BUTTON_PRESSED(state->buttons_state, MIN_DOWN) ? 
                    ButtonStatus_ButtonPressed : ButtonStatus_ButtonNotPressed;
            case ButtonEnum_StartPause:
                return BUTTON_PRESSED(state->buttons_state, START_PAUSE) ? 
                    ButtonStatus_ButtonPressed : ButtonStatus_ButtonNotPressed;
            case ButtonEnum_Reset:
                return BUTTON_PRESSED(state->buttons_state, RESET) ? 
                    ButtonStatus_ButtonPressed : ButtonStatus_ButtonNotPressed;
            case ButtonEnum_Bypass:
                return BUTTON_PRESSED(state->buttons_state, BYPASS) ? 
                    ButtonStatus_ButtonPressed : ButtonStatus_ButtonNotPressed;
            default:
                return ButtonStatus_ButtonNotPressed;
        }
    }
}

unsigned char ButtonManager_GetProgramCode(ButtonManagerState * state) {
    return (unsigned char)(state->buttons_state & 0xF);
}

unsigned char ButtonManager_ShouldProcessButtonClick(
    ButtonManagerState * state) {
    unsigned char should_click = 0;
    state->ticks_since_previous_click += 1;
    if (state->ticks_since_button_change >= 40000) {
        if (state->ticks_since_previous_click > 1000) {
            should_click = 1;
            state->ticks_since_previous_click = 0;
        }
    }
    else if (state->ticks_since_button_change >= 20000) {
        if (state->ticks_since_previous_click > 2000) {
            should_click = 1;
            state->ticks_since_previous_click = 0;
        }
    }
    else if (state->ticks_since_button_change >= 10000) {
        if (state->ticks_since_previous_click > 3000) {
            should_click = 1;
            state->ticks_since_previous_click = 0;
        }
    }
    else if (state->ticks_since_button_change >= 5000) {
        if (state->ticks_since_previous_click > 4000) {
            should_click = 1;
            state->ticks_since_previous_click = 0;
        }
    }

    return should_click;
}
