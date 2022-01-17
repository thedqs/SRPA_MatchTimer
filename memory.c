#include "memory.h"
#include <stdlib.h>
#include <string.h>

// EEprom memory layout
/*
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
 */

__EEPROM_DATA(0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x38, 'R', 'a', 'p', 'i', 'd', ' ', 'F');
__EEPROM_DATA('i', 'r', 'e', 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x02, 'P', 'r', 'e', 'p', ' ', 'T');
__EEPROM_DATA('i', 'm', 'e', 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x1E, 0x12, 'F', 'i', 'r');
__EEPROM_DATA('e', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x12, 0x00);

unsigned char eeprom18_read(unsigned short address) {
    EECON1bits.EEPGD = 0; //accesses data EEPROM memory
    EECON1bits.CFGS = 0; //accesses data EEPROM memory

    EEADRH = (unsigned char)((address >> 8) & 0xFF);
    EEADR = (unsigned char)(address & 0xFF);

    EECON1bits.RD = 1; //initiates an EEPROM read
    Nop(); //it can be read after one NOP instruction
    unsigned char data = EEDATA;
    // Restore EEPGD as the compiler believes the proc is in this state
    EECON1bits.EEPGD = 1; 
    
    return data;
}

unsigned char eeprom18_write(unsigned short address, unsigned char value) {
    unsigned char successful_write = 1;
    EECON1bits.EEPGD = 0; //accesses data EEPROM memory
    EECON1bits.CFGS = 0; //accesses data EEPROM memory

    EEADRH = (unsigned char)((address >> 8) & 0xFF);
    EEADR = (unsigned char)(address & 0xFF);

    EEDATA = value;

    EECON1bits.WREN = 1; //allows write cycles
    unsigned char oldGIE = GIE; //interrupts be disabled during this code segment

    EECON2 = 0x55; //write sequence unlock
    EECON2 = 0xAA; //write sequence unlock

    EECON1bits.WR = 1; //initiates a data EEPROM erase/write cycle
    while(EECON1bits.WR); //waits for write cycle to complete
    
    GIE = (oldGIE != 0); //restore interrupts
    
    if (EECON1bits.WRERR) {
        successful_write = 0;
    }

    EECON1bits.WREN = 0; //disable write
    EECON1bits.EEPGD = 1; 
    return successful_write;
}

ReadProgramStatus MemoryManager_ReadProgram(unsigned char program_id, 
        Program * program_info) {
    if (program_id > 0xD || program_info == NULL) {
        return ReadProgramStatus_NoProgram;
    }
    
    unsigned char preallocated_stages = program_info->CountOfStages;
    unsigned short program_address = (unsigned short)(eeprom18_read((unsigned short)(program_id << 1)) << 8 | 
        eeprom18_read((unsigned short)(program_id << 1) + 1));
    if (program_address == 0) {
        return ReadProgramStatus_NoProgram;
    }
    
    unsigned char program_size = eeprom18_read(program_address);
    if (program_size < MAX_STAGE_NAME_LEN + 3) {
        // At the min the size should include the size byte, the name and the 
        // number of stages (0 for min).
        return ReadProgramStatus_Error;
    }
    
    unsigned short pointer = program_address + 1;
    for (unsigned char name_index = 0; name_index <= MAX_STAGE_NAME_LEN; 
            ++name_index) {
        program_info->Name[name_index] = eeprom18_read(pointer);
        pointer++;
    }
    
    unsigned char stage_count = eeprom18_read(pointer);
    pointer++;
    program_info->CountOfStages = stage_count;
    if (stage_count > preallocated_stages) {
        return ReadProgramStatus_NeedRoomForStages;
    }
    
    for (unsigned char stage_index = 0; stage_index < stage_count; 
        ++stage_index) {
        for (unsigned char name_index = 0; name_index <= MAX_STAGE_NAME_LEN; 
            ++name_index) {
            program_info->StageArray[stage_index].Name[name_index] = 
                eeprom18_read(pointer);
            pointer++;
        }
        program_info->StageArray[stage_index].Duration.Minutes = 
            eeprom18_read(pointer);
        pointer++;
        program_info->StageArray[stage_index].Duration.Seconds = 
            eeprom18_read(pointer);
        pointer++;
        unsigned char actions = eeprom18_read(pointer);
        pointer++;
        program_info->StageArray[stage_index].OnStart = (actions >> 4) & 0xF;
        program_info->StageArray[stage_index].OnEnd = actions & 0xF;
    }
    
    return ReadProgramStatus_Success;
}
