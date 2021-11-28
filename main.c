/*
 * File:   main.c
 * Author: thedqs
 *
 * Created on January 6, 2020, 1:13 PM
 * Target Timer V2 for Seattle Rifle and Pistol Association
 */

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
#include "memory.h"

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


volatile int g_CurrentLED = 0;
volatile char g_LEDsDisplaying[4] = {};
volatile Time g_CurrentTimerTime = {0, 0};
volatile unsigned char g_isRunning = 0;
volatile char g_MessageBuffer[128] = {};
volatile char * g_MessagePointer = g_MessageBuffer;
volatile char * g_MessageEndPointer = g_MessageBuffer;
volatile int g_Timer1Counter = 0;
volatile int g_DisplayCounter = 0;
unsigned short g_ButtonCache = 0;
unsigned short g_ButtonDuration = 0;
unsigned char g_ProgramNumber = 0; 

void StartProgram();
void StopProgram();
void LoadProgram(unsigned int programNumber);
void guiUpdate();
void clockUpdate();

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

unsigned int strlen(char * str)
{
    int retVal = 0;
    char * cur = str;
    if (cur != NULL)
    {
        while (*cur != '\0')
        {
            retVal++;
            cur++;
        }
    }
    return retVal;
}

void memcpy(volatile char * dst, char * src, unsigned int count)
{
    while (count > 0)
    {
        *dst = *src;
        dst++;
        src++;
        count--;
    }
}

void PrintMessage(char * msg)
{
    // Likely we'll have printed out our messages already.
    if (g_MessagePointer == g_MessageEndPointer)
    {
        g_MessageEndPointer = g_MessagePointer = g_MessageBuffer;
    }
    
    unsigned int len = strlen(msg);
    int remainingSpace = (g_MessageBuffer + sizeof(g_MessageBuffer) - g_MessageEndPointer);
    if (len <= remainingSpace)
    {
        memcpy(g_MessageEndPointer, msg, len + 1);
    }
    g_MessageEndPointer += len + 2;
}

void __interrupt() irqHandler()
{
    if (TMR0IF)
    {
        TMR0IF = 0;
        guiUpdate();
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
            clockUpdate();
            g_Timer1Counter = 0;
        }
    }
}

void guiUpdate()
{
    // See which LED we are going to be controlling
    int nextLED = (g_CurrentLED + 1) % 4;
    // Read the current screen buffer
    char ch = g_LEDsDisplaying[nextLED];
    // Get the code for the character
    unsigned char code = getLEDCode(ch);
    // Select the LED (High is active)
    PORTC = (PORTC & 0x0F) | (0x1 << (nextLED + 4));
    // Set the segments
    PORTD = code;
    // Set current to the next one
    g_CurrentLED = nextLED;
}

void clockUpdate()
{
    //if (g_isRunning)
    {
        g_CurrentTimerTime.Seconds++;
        if (g_CurrentTimerTime.Seconds == 60)
        {
            g_CurrentTimerTime.Minutes++;
            if (g_CurrentTimerTime.Minutes == 100)
            {
                g_CurrentTimerTime.Minutes = 0;
            }
            g_CurrentTimerTime.Seconds = 0;
        }
        
        //decrementClock(g_CurrentTimerTime);
    }
}

void SleepEx(unsigned int ms)
{
    // We are at 8 MHz so we need 8000 nops per ms (so one increment, one compare and one nop)
    for (unsigned int currentMs = 0; currentMs < ms; ++currentMs)
    {
        for (unsigned short tick = 0; tick < 3000; ++tick)
        {
            __nop();
        }
    }
}

unsigned short ReadButtons()
{
    // Bit layout of Return Value
    // [Min Up][Min Down][Sec Up][Sec Down][Reset][Start/Pause][Bypass][0:4 Selector Value]
    unsigned short button_value = 0;
    button_value |= ((PORTB >> 2) & 0xF);
    button_value |= (PORTE & 0x7) << 4;
    button_value |= ((PORTA >> 4) & 0x3) << 7;
    button_value |= ((PORTC >> 2) & 0x3) << 9;
    return button_value;
}

unsigned short DebounceButtons(unsigned short buttons)
{
    // Bit layout of Return Value
    // [Min Up][Min Down][Sec Up][Sec Down][Reset][Start/Pause][Bypass][0:4 Selector Value]
    if (g_ButtonCache == buttons)
    {
        g_ButtonDuration++;
    }
    else
    {
        g_ButtonCache = buttons;
        g_ButtonDuration = 0;
    }
    
    if (g_ButtonDuration >= 1000)
    {
        return buttons;
    }
    return 0;
}

// [Min Up][Min Down][Sec Up][Sec Down][Reset][Start/Pause][Bypass][0:4 Selector Value]
#define IsMinUpPressed(btn)     (((btn >> 10) & 1) == 1)
#define IsMinDownPressed(btn)   (((btn >> 9) & 1) == 1)
#define IsSecUpPressed(btn)     (((btn >> 8) & 1) == 1)
#define IsSecDownPressed(btn)   (((btn >> 7) & 1) == 1)
#define IsResetPressed(btn)     (((btn >> 6) & 1) == 1)
#define IsStartPressed(btn)     (((btn >> 5) & 1) == 1)
#define IsBypassPressed(btn)    (((btn >> 4) & 1) == 1)
#define GetSelectorValue(btn)   ((unsigned char)(btn & 0xF))

void main(void) {
    // C7-4 are the LED selection pins, C3-2 input (min down, min up) C1-0 is TMR1 Clock
    TRISC = 0b00001100;
    // D7-0 Are the LED segments 7 (dp), 6 (g), 5 (f), 4 (e), 3 (d), 2 (c), 1 (b), 0 (a)
    TRISD = 0x0;
    // B6-7 are left for programming and debugging, B2-5 are the hex selector input
    TRISB |= 0b11111100;
    // A5 (Sec Down) A4 (Sec up) A3 (N/C) A2 (Edit LED) A1 (SS Relay) A0 (Timer Active LED)
    TRISA |= 0b00110000;
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
    
    PrintMessage("Start Up");
    
    /*while (1)
    {
        // [Min Up][Min Down][Sec Up][Sec Down][Reset][Start/Pause][Bypass][0:4 Selector Value]
        unsigned short button_io = ReadButtons();
        // PORTB 2-5 Hex Switch
        char item = GetSelectorValue(button_io);
        if (item > 9)
        {
            item = (item - 10) + 'A';
        }
        else
        {
            item = '0' + item;
        }
        g_LEDsDisplaying[0] = item;
        
        item = PORTE & 0x7;
        g_LEDsDisplaying[1] = '0' + item;
    }*/
    // Button polling code
    while (1)
    {
        // Update what is being displayed
        ++g_DisplayCounter;
        if ((g_DisplayCounter >= 1000) && g_MessagePointer != g_MessageEndPointer)
        {
            g_DisplayCounter = 0;
            g_LEDsDisplaying[0] = g_MessagePointer[0];
            if (g_MessagePointer[0] == '\0')
            {
                g_LEDsDisplaying[1] = 0;
                g_LEDsDisplaying[2] = 0;
                g_LEDsDisplaying[3] = 0;
            }
            else
            {
                g_LEDsDisplaying[1] = g_MessagePointer[1];
                if (g_MessagePointer[1] == '\0')
                {
                    g_LEDsDisplaying[2] = 0;
                    g_LEDsDisplaying[3] = 0;
                }
                else
                {
                    g_LEDsDisplaying[2] = g_MessagePointer[2];
                    if (g_MessagePointer[2] == '\0')
                    {
                        g_LEDsDisplaying[3] = 0;
                    }
                    else
                    {
                        g_LEDsDisplaying[3] = g_MessagePointer[3];
                    }
                }
            }
            g_MessagePointer++;
        }
        else if (g_MessagePointer == g_MessageEndPointer)
        {
            // Display the Timer info
            g_LEDsDisplaying[0] = '0' + (g_CurrentTimerTime.Minutes / 10);
            g_LEDsDisplaying[1] = '0' + (g_CurrentTimerTime.Minutes % 10);
            g_LEDsDisplaying[2] = '0' + (g_CurrentTimerTime.Seconds / 10);
            g_LEDsDisplaying[3] = '0' + (g_CurrentTimerTime.Seconds % 10);
        }

        
        unsigned short buttons = ReadButtons();
        buttons = DebounceButtons(buttons);
        
        if (g_ButtonDuration % 500 == 0)
        {
            if (IsSecDownPressed(buttons))
            {
                g_CurrentTimerTime.Seconds--;
            }

            if (IsSecUpPressed(buttons))
            {
                g_CurrentTimerTime.Seconds++;
            }

            if (IsMinDownPressed(buttons))
            {
                g_CurrentTimerTime.Minutes--;
            }

            if (IsMinUpPressed(buttons))
            {
                g_CurrentTimerTime.Minutes++;
            }
            /*
            // Setup mode - Hold down minute up and down
            if (!g_isRunning)
            {
                if (g_buttonDurations[BTN_MIN_DOWN] > 10000 && g_buttonDurations[BTN_MIN_UP] > 20000)
                {
                    // Need to hold for 1/2 a second before entering setup mode
                    //EnterSetupMode();
                }
                else
                {
                    if (g_buttonDurations[BTN_MIN_DOWN] > 2500 && g_buttonDurations[BTN_MIN_DOWN] % 500 == 0 && g_buttonDurations[BTN_MIN_UP] == 0)
                    {
                        if (g_buttonDurations[BTN_MIN_DOWN] < 10000)
                        {
                            g_CurrentTimerTime.Minutes--;
                        }
                        else if (g_buttonDurations[BTN_MIN_DOWN] < 30000)
                        {
                            g_CurrentTimerTime.Minutes -= 5;
                        }
                        else
                        {
                            g_CurrentTimerTime.Minutes -= 10;
                        }
                    }

                    if (g_buttonDurations[BTN_MIN_UP] > 20000 && g_buttonDurations[BTN_MIN_UP] % 5000 == 0 && g_buttonDurations[BTN_MIN_DOWN] == 0)
                    {
                        if (g_buttonDurations[BTN_MIN_UP] < 40000)
                        {
                            g_CurrentTimerTime.Minutes++;
                        }
                        else if (g_buttonDurations[BTN_MIN_UP] < 80000)
                        {
                            g_CurrentTimerTime.Minutes += 5;
                        }
                        else
                        {
                            g_CurrentTimerTime.Minutes += 10;
                        }
                    }

                    if (g_buttonDurations[BTN_SEC_DOWN] > 20000 && g_buttonDurations[BTN_SEC_DOWN] % 5000 == 0 && g_buttonDurations[BTN_SEC_UP] == 0)
                    {
                        if (g_buttonDurations[BTN_SEC_DOWN] < 40000)
                        {
                            g_CurrentTimerTime.Seconds--;
                        }
                        else if (g_buttonDurations[BTN_SEC_DOWN] < 80000)
                        {
                            g_CurrentTimerTime.Seconds -= 5;
                        }
                        else
                        {
                            g_CurrentTimerTime.Seconds -= 10;
                        }
                    }

                    if (g_buttonDurations[BTN_SEC_UP] > 20000 && g_buttonDurations[BTN_SEC_UP] % 5000 == 0 && g_buttonDurations[BTN_SEC_DOWN] == 0)
                    {
                        if (g_buttonDurations[BTN_SEC_UP] < 40000)
                        {
                            g_CurrentTimerTime.Seconds++;
                        }
                        else if (g_buttonDurations[BTN_SEC_UP] < 80000)
                        {
                            g_CurrentTimerTime.Seconds += 5;
                        }
                        else
                        {
                            g_CurrentTimerTime.Seconds += 10;
                        }
                    }
                }
            }*/

            if (IsResetPressed(buttons))
            {
                g_CurrentTimerTime.Minutes = 0;
                g_CurrentTimerTime.Seconds = 0;
                StopProgram();
                LoadProgram(GetSelectorValue(buttons));
            }
            else if (IsStartPressed(buttons))
            {
                StartProgram();
            }
        }
    }

    return;
}

void StartProgram()
{
    char starting_message[] = "Starting Program   ";
    
    if (g_ProgramNumber < 10)
    {
        starting_message[18] = '0' + g_ProgramNumber;
    }
    else if (g_ProgramNumber < 100)
    {
        starting_message[18] = '0' + (g_ProgramNumber / 10);
        starting_message[19] = '0' + (g_ProgramNumber % 10);
    }
    
    PrintMessage(starting_message);
}

void StopProgram()
{
    PrintMessage("Stopping Program");
}

void LoadProgram(unsigned int programNumber)
{
    g_ProgramNumber = (unsigned char)programNumber;
}