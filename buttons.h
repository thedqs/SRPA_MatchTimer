/* 
 * File:   buttons.h
 * Author: David Springgay
 *
 * Created on January 15, 2022, 2:49 PM
 */

#ifndef BUTTONS_H
#define	BUTTONS_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef unsigned short ButtonMap;
typedef enum { 
    ButtonStatus_ButtonPressed, 
    ButtonStatus_ButtonNotPressed 
} ButtonStatus;    

typedef enum { 
    ButtonEnum_SecondUp, 
    ButtonEnum_SecondDown, 
    ButtonEnum_MinuteUp, 
    ButtonEnum_MinuteDown, 
    ButtonEnum_StartPause, 
    ButtonEnum_Reset, 
    ButtonEnum_Bypass 
} ButtonEnum;

typedef struct {
    ButtonMap buttons_state;
    int ticks_since_button_change;
} ButtonManagerState;

ButtonManagerState * CreateButtonManager();
ButtonStatus ButtonManager_ButtonStatus(ButtonManagerState * state, 
        ButtonEnum button);
void ButtonManager_ReadButtons(ButtonManagerState * state, unsigned char portA, 
        unsigned char portB, unsigned char portC, unsigned char portE);
unsigned char ButtonManager_GetProgramCode(ButtonManagerState * state);

#ifdef	__cplusplus
}
#endif

#endif	/* BUTTONS_H */

