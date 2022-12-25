#include <xc.h> // include processor files - each processor file is guarded.  

// EEprom memory layout - copied from memory.c
/*
 * MAX_STAGE_NAME_LEN = 15
 * 
 * 14 shorts [for each program] that is the EEProm address to the start of the
 * program structure with program 0 starting at byte 32. Value 0 = Not written
 * Start of program structure
 * byte - Length of Program Structure
 * MAX_STAGE_NAME_LEN + 1 - Printable String of Program Name
 * byte - Count of Stage Structures following
 * Start of Stage Structure
 * MAX_STAGE_NAME_LEN + 1 - Printable String of Stage Name
 * byte - Minutes of Stage
 * byte - Seconds of Stage
 * byte - 4 bits for Start Action, 4 bits for End Action
 * 
 * Action values 
 *  0 = Nothing, 
 *  1 = Activate Target, 
 *  2 = Deactivate Target
 */


// These are all the preset firing programs for the timer (short, 16-bits each)
__EEPROM_DATA(0x00, 0x20, 0x00, 0x58, 0x00, 0x90, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

// Program 0 - Rapid Fire - 10 s Prep Time, 90 s Fire Time
__EEPROM_DATA(0x38, 'R', 'a', 'p', 'i', 'd', ' ', 'F');
__EEPROM_DATA('i', 'r', 'e', 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x02, 'P', 'r', 'e', 'p', 'a', 'r');
__EEPROM_DATA('a', 't', 'i', 'o', 'n', 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x0A, 0x20, 'F', 'i', 'r');
__EEPROM_DATA('e', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1E, 0x12);

// Program 1 - Standard Target - 60 s Prep Time, 10 minute Fire Time
__EEPROM_DATA(0x38, 'N', 'o', 'r', 'm', 'a', 'l', 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x02, 'P', 'r', 'e', 'p', 'a', 'r');
__EEPROM_DATA('a', 't', 'i', 'o', 'n', 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x3C, 0x20, 'F', 'i', 'r');
__EEPROM_DATA('e', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x12);

// Program 2 - Champion Match Run
// Stage 1 - 60 second Prep Time
// Stage 2 - 10 minute Fire Time
// Stage 3 - 30 second Prep Time
// Stage 4 - 3 minute Fire Time
__EEPROM_DATA(0x5E, 'C', 'h', 'a', 'm', 'p', 'i', 'o');
__EEPROM_DATA('n', ' ', 'M', 'a', 't', 'c', 'h', 0x00);
__EEPROM_DATA(0x00, 0x04, 'P', 'r', 'e', 'p', 'a', 'r');
__EEPROM_DATA('a', 't', 'i', 'o', 'n', ' ', '1', 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x3C, 0x20, 'F', 'i', 'r');
__EEPROM_DATA('e', ' ', '1', 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x12);
__EEPROM_DATA('P', 'r', 'e', 'p', 'a', 'r', 'a', 't');
__EEPROM_DATA('i', 'o', 'n', ' ', '2', 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x1E, 0x20, 'F', 'i', 'r', 'e', ' ');
__EEPROM_DATA('2', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x03, 0x00, 0x12, 0x00, 0x00);
