/*
 * File:   main.c
 * Author: David Springgay
 *
 * Created on January 6, 2020, 1:13 PM
 * Target Timer V2 for Seattle Rifle and Pistol Association
 */

#include "memory.h"


#define DEBUG
#define USE_TMR1_INT

// CONFIG1H
#ifdef DEBUG
#pragma config OSC = IRCIO67    // Internal oscillator block, RA6 and RA7 enabled
#else
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator)
#endif
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = ON        // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bits (Brown-out Reset disabled in hardware and software)
#pragma config BORV = 2         // Brown-out Reset Voltage bits (VBOR set to 2.8V)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer 1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config BBSIZ = 1024     // Boot Block Size Select bits (1K words (2K bytes) Boot Block)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-003FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (004000-007FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (008000-00BFFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-003FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (004000-007FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (008000-00BFFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

/*
 * Dev Design
 * Purpose: The device comes with preset firing courses and will allow the
 * operator to select the course of fire and start the timer. The timer will
 * then run the course of fire. The system must have the ability to edit the
 * 14 preset courses of fire.
 * 
 * Course of fire: Sequence of Events - Prep Time (optional); Main Fire; Stop,
 * Repeat (with wait time and number of times to repeat), or advance to another
 * course of fire.
 * 
 * Setup Mode: Moving the rotary switch to the 16th position and pushing the
 * start button will enable the setup mode. To leave the setup mode make sure 
 * the rotary switch is in the 16th position and push the reset button. In the
 * setup mode you can move the rotary switch to any of the preset (1-14) 
 * positions to edit by pressing the start button. While in the setup phase you
 * can at any time push the reset button to exit (push reset again to discard 
 * changes, press start to save changes).
 * 
 * Software Debouncing: Each switch needs to be held continously for 10 ms.
 * 
 * Setup Progress:
 * Prep Time 00:00 = No, otherwise set
 * Firing Time
 * End Mode: Stop, Repeat, New Course
 *  
 * 
 * Types Firing courses (14 available)
 * Rapid Fire ()
 * 
 * Rotary Switch Special Positions:
 * 1 - 14 (Preset Options)
 * 15 - Manual Clock
 * 16 - Setup Mode
 */

#define BUTTON_COUNT 7

#include <xc.h>
#include "ui.h"
#include "timemgr.h"
#include "buttons.h"
#include "program.h"

//#define CLK_AT_4MHz 1

#ifndef USE_TMR1_INT
#define TMR1OFF_H 0x7F
#define TMR1OFF_L 0xFF
#else
// This gives us 25000 ticks per interrupt 
// 40535
#define TMR1OFF_H 0x9E
#define TMR1OFF_L 0x57
#endif

volatile int g_Timer1Counter = 0;
volatile unsigned char g_ClockTicks = 0;
volatile unsigned char g_UpdateGUIFlag = 0;

void __interrupt() irqHandler()
{
    if (TMR0IF)
    {
        TMR0IF = 0;
        g_UpdateGUIFlag = 1;
    }
    
    if (TMR1IF)
    {
        TMR1IF = 0;
        TMR1H = TMR1OFF_H;
        TMR1L = TMR1OFF_L;
#ifndef CLK_AT_4MHz        
        if (++g_Timer1Counter >= 10)
#else
        if (++g_Timer1Counter >= 5)
#endif
        {
            g_Timer1Counter = 0;
            g_ClockTicks++;
        }
    }
}

void SleepEx(unsigned int ms)
{
    // We are at 8 MHz so we need 8000 nops per ms (so one increment, one 
    // compare and one nop)
    for (unsigned int currentMs = 0; currentMs < ms; ++currentMs)
    {
        for (unsigned short tick = 0; tick < 3000; ++tick)
        {
            __nop();
        }
    }
}

// [Min Up][Min Down][Sec Up][Sec Down][Reset][Start/Pause][Bypass][0:4 Selector Value]

void main(void) {
    // C7-4 are the LED selection pins, C3-2 input (min down, min up) C1-0 is TMR1 Clock
    TRISC =  0b00001100;
    // D7-0 are the LED segments 7 (dp), 6 (g), 5 (f), 4 (e), 3 (d), 2 (c), 1 (b), 0 (a)
    TRISD =  0x0;
    // B6-7 are left for programming and debugging, B2-5 are the hex selector input
    TRISB |= 0b11111100;
    // A5 (Sec Down) A4 (Sec up) A3 (N/C) A2 (Edit LED) A1 (SS Relay) A0 (Timer Active LED)
    TRISA =  0b00111000;
    // Disable A/D converters
    ADCON1 = 0b00001111;
    // Disable comparators
    CMCON |= 0b00000111;
    // E2-0 are button inputs
    PORTE = 0;
    LATE = 0;
    TRISE = 0b00000111;
    
    // PORTD pgfedcba
    // PORTC [Ones Seconds][Tens Seconds][Ones Minutes][Tens Minutes]
    //return;
    
    OSCCONbits.IRCF = 0x7;
    T1CON = 0b11001001;

    // T0 fires 976 times a second (roughly 1 ms)
    T0CONbits.TMR0ON = 1;
    T0CONbits.PSA = 0;
    T0CONbits.T0CS = 0;
    // 0b100 = 1:32 for 20 MHz, 0b010 = 1:8 for 4 MHz
    T0CONbits.T0PS = 0b010;
    T0CONbits.T08BIT = 1;
    
#ifdef USE_TMR1_INT
    // TMR1 is OSC/4 so 1MHz It overflows and fires the interrupt at 65536.
    // We set the prescaler to 8:1 which is 125000/sec so we ask for the
    // interrupt ever 25000 ticks and after 5 interrupts increment the second
    
    T1CONbits.T1CKPS = 0b11;
    // Use the 32.768 kHz Clock Osc
    T1CONbits.TMR1CS = 0;
#else
    // T1 fires at 32.768 kHz. So 1 second = 32768 and the overflow is at
    // 65535 so we need to set the start at 32767.
    // Prescaler value 1:1 
    T1CONbits.T1CKPS = 0b00;
    // Use the 32.768 kHz Clock Osc
    T1CONbits.TMR1CS = 0;
#endif
    T1CONbits.T1RD16 = 1;
    T1CONbits.T1OSCEN = 1;
    T1CONbits.T1SYNC = 1;
    T1CONbits.TMR1ON = 1;
    
    TMR1H = TMR1OFF_H;
    TMR1L = TMR1OFF_L;
    
    PORTD = 0b11111111;
    PORTC = 0b11100000;    
    
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
    TMR0IE = 1;
    TMR1IE = 1;
    UiState display_state;
    TimerManagerState timer_state;
    ButtonManagerState buttons_state;
    ProgramManagerState program_state;
    
    UiState * display = &display_state;
    TimerManagerState * timer = &timer_state;
    ButtonManagerState * buttons = &buttons_state;
    ProgramManagerState * program = &program_state;
    
    InitializeUiComponentState(display);
    PrintMessage(display, "Start Up");

    InitializeTimerManager(timer);
    InitializeButtonManager(buttons);
    InitializeProgramManager(program);
    
    // Load up the program so that it is valid
    ProgramManager_LoadProgram(program, timer, display);

    // Main program
    while (1) {
        while (g_ClockTicks > 0) {
            g_ClockTicks--;
            if (TimerManager_TickSecond(timer) == 
                TimerStatus_TimerCompleted) {
                // Timer has completed
                ProgramManager_SignalStageComplete(program, timer, display);
            }
        }

        if (g_UpdateGUIFlag != 0) {
            g_UpdateGUIFlag = 0;
            UiUpdate(display, timer, buttons, program);
        }

        ButtonManager_ReadButtons(buttons, PORTA, PORTB, PORTC, PORTE);
        if (ButtonManager_ShouldProcessButtonClick(buttons)) {
            if (ButtonManager_ButtonStatus(buttons, ButtonEnum_SecondDown) == 
                    ButtonStatus_ButtonPressed) {
                TimerManager_AdjustSeconds(timer, -1);
            }
            else if (ButtonManager_ButtonStatus(buttons, ButtonEnum_SecondUp) == 
                    ButtonStatus_ButtonPressed) {
                TimerManager_AdjustSeconds(timer, 1);
            }
            else if (ButtonManager_ButtonStatus(buttons, ButtonEnum_MinuteUp) == 
                    ButtonStatus_ButtonPressed) {
                TimerManager_AdjustMinutes(timer, 1);
            }
            else if (ButtonManager_ButtonStatus(buttons, ButtonEnum_MinuteDown) == 
                    ButtonStatus_ButtonPressed) {
                TimerManager_AdjustMinutes(timer, -1);
            }
            else if (ButtonManager_ButtonStatus(buttons, ButtonEnum_StartPause) == 
                    ButtonStatus_ButtonPressed) {
                if (ButtonManager_GetProgramCode(buttons) == 0xE ||
                        program->EditModeActive != 0) {
                    ProgramManager_ToggleEditState(program);
                } else {
                    if (timer->_State == TimerState_Paused) {
                        ProgramManager_ResumeProgram(program, timer, display);
                    }
                    else {
                        ProgramManager_PauseProgram(program, timer);
                    }
                }
            }
            else if (ButtonManager_ButtonStatus(buttons, ButtonEnum_Reset) == 
                    ButtonStatus_ButtonPressed) {
                // Stop the timer and Reload the program
                TimerManager_Pause(timer);
                ProgramManager_LoadProgram(program, timer, display);
            }
        }

        // Bypass switch is once the switch is in the on position
        ProgramManager_SetBypassSwitchState(program, 
            ButtonManager_ButtonStatus(buttons, ButtonEnum_Bypass) == 
            ButtonStatus_ButtonPressed);

        ProgramManager_SetProgramSwitchState(program, timer, display,
            ButtonManager_GetProgramCode(buttons));
    }

    _Exit(0);
}
