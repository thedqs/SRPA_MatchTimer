/*
 * File:   gui.c
 * Author: thedq
 *
 * Created on January 15, 2022, 12:38 PM
 */


#include <xc.h>
#include <string.h>
#include <stdlib.h>
#include "timemgr.h"
#include "buttons.h"
#include "ui.h"

unsigned char getLEDCode(char ch);

UiState * CreateUiComponent() {
    UiState * state = (UiState*)malloc(sizeof(UiState));
    if (state != NULL) {
        state->CurrentLED = 0;
        memset(state->LEDsDisplaying, 0, sizeof(state->LEDsDisplaying));
        memset(state->MessageBuffer, 0, sizeof(state->MessageBuffer));
        state->MessagePointer = state->MessageEndPointer = state->MessageBuffer;
        // Update on first call
        state->update_count = 500000;
    }
    return state;    
}

//  LED:  pgfedcba
static const unsigned char s_LEDCodes[] = {
//  Control characters
    0b00000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000,
    0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000,
    0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000,
    0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000,
    0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000,
//                          Space       !           "           #
    0b10000000, 0b10000000, 0b00000000, 0b10000110, 0b10000000, 0b10000000,
//  $           %           &           '           (           )
    0b10000000, 0b10000000, 0b10000000, 0b00000010, 0b00110000, 0b00000110,
//  *           +           ,           -           .           /
    0b10000000, 0b10000000, 0b10000000, 0b01000000, 0b10000000, 0b00010010,
//  0           1           2           3           4           5
    0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101,
//  6           7           8           9           :           ;
    0b01111101, 0b00000111, 0b01111111, 0b01100111, 0b01000001, 0b00001101,
//  <           =           >           ?           @           A
    0b10000000, 0b01000001, 0b10000000, 0b11010011, 0b10000000, 0b01110111,
//  B           C           D           E           F           G
    0b01111100, 0b00111001, 0b01011110, 0b01111001, 0b01110001, 0b00111101,
//  H           I           J           K           L           M
    0b01110110, 0b00110000, 0b00011110, 0b01110110, 0b00111000, 0b10000000,
//  N           O           P           Q           R           S
    0b01010100, 0b00111111, 0b01110011, 0b01100111, 0b01010000, 0b01101101,
//  T           U           V           W           X           Y
    0b01110000, 0b00011100, 0b01110010, 0b10000000, 0b10000000, 0b01101110,
//  Z           [           \           ]           ^           _
    0b10000000, 0b10111001, 0b10000000, 0b10001111, 0b10000000, 0b00001000,
//  `           a           b           c           d           e
    0b10000000, 0b01110111, 0b01111100, 0b01011000, 0b01011110, 0b01111001,
//  f           g           h           i           j           k
    0b01110001, 0b00111101, 0b01110100, 0b00110000, 0b00011110, 0b01110110,
//  l           m           n           o           p           q
    0b00111000, 0b10000000, 0b01010100, 0b01011100, 0b01110011, 0b01100111,
//  r           s           t           u           v           w
    0b01010000, 0b01101101, 0b01110000, 0b00011100, 0b01110010, 0b10000000,
//  x           y           z           {           |           }
    0b10000000, 0b01101110, 0b10000000, 0b10000000, 0b00110000, 0b01111100,
//  ~         
    0b01010010
};

unsigned char getLEDCode(char ch)
{
    unsigned char retValue = 0;
    
    if (ch < sizeof(s_LEDCodes))
    {
        retValue = s_LEDCodes[ch];
    }
    else
    {
        retValue = 0b10000000;
    }
    
    return retValue;
}

void PrintMessage(UiState * ui, char * msg)
{
    // Likely we'll have printed out our messages already.
    if (ui->MessagePointer == ui->MessageEndPointer)
    {
        ui->MessageEndPointer = ui->MessagePointer = ui->MessageBuffer;
    }
    
    unsigned int len = strlen(msg);
    int remainingSpace = (ui->MessageBuffer + sizeof(ui->MessageBuffer) - ui->MessageEndPointer);
    if (len <= remainingSpace)
    {
        memcpy(ui->MessageEndPointer, msg, len + 1);
    }
    ui->MessageEndPointer += len + 2;
}


void UiUpdate(UiState * ui, TimerManagerState * timer, 
        ButtonManagerState * buttons, ProgramManagerState * program)
{
    // 8-seg LED display control
    ui->update_count++;
    if (ui->update_count >= 0x200) {
        // Displaying a message when our message buffer has something
        if (ui->MessagePointer != ui->MessageEndPointer) {
            memset(ui->LEDsDisplaying, 0, sizeof(ui->LEDsDisplaying));
            unsigned char characters_copied = 0;
            while (ui->MessagePointer != ui->MessageEndPointer && 
                    characters_copied < 4) {
                ui->LEDsDisplaying[characters_copied] = 
                        ui->MessagePointer[characters_copied];
                characters_copied++;
            }
            ui->MessagePointer += 1;
        }
        else {
            // We are just going to display the timer time
            unsigned char minutes_ones = timer->_CurrentTime.Minute;
            unsigned char minutes_tens = 0;
            unsigned char seconds_ones = timer->_CurrentTime.Second;
            unsigned char seconds_tens = 0;
            
            while (minutes_ones >= 10) {
                minutes_tens++;
                minutes_ones -= 10;
            }
            
            while (seconds_ones >= 10) {
                seconds_tens++;
                seconds_ones -= 10;
            }

            ui->LEDsDisplaying[0] = '0' + minutes_tens;
            ui->LEDsDisplaying[1] = '0' + minutes_ones;
            ui->LEDsDisplaying[2] = '0' + seconds_tens;
            ui->LEDsDisplaying[3] = '0' + seconds_ones;
        }
        ui->update_count = 0;
    }
    
    // See which LED we are going to be controlling
    unsigned char nextLED = (ui->CurrentLED + 1) % 4;
    // Read the current screen buffer
    char ch = ui->LEDsDisplaying[nextLED];
    // Get the code for the character
    unsigned char code = getLEDCode(ch);
    // Select the LED (High is active)
    PORTC = (unsigned char)((PORTC & 0x0F) | (0x1 << (nextLED + 4)));
    // Set the segments
    PORTD = code;
    // Set current to the next one
    ui->CurrentLED = nextLED;
    
    // Timer Active LED PORTA[0]
    if (timer->_State == TimerState_Paused) {
        PORTA &= 0b11111110;
    }
    else {
        PORTA |= 0b00000001;
    }
    
    // Targets Active PORTA[1]
    if (program->TargetActive | program->TargetOverrideActive) {
        PORTA |= 0b00000010;
    }
    else {
        PORTA &= 0b11111101;
    }

    // Edit Mode LED Active PORTA[2]
    if (program->EditModeActive) {
        PORTA |= 0b00000100;
    }
    else {
        PORTA &= 0b11111011;
    }

    
}

