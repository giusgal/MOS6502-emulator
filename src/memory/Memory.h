#ifndef MEMORY_H
#define MEMORY_H

#include <string>
#include <cstdint>

#define SIZE 0x10000 // 64KiB

void memoryWrite(uint16_t addr, uint8_t data);
uint8_t memoryRead(uint16_t addr);

/*
    Load the memory content from a file containing
    whitespace-separated hex bytes at the specified
    address.

    EXAMPLE (File content):
     A9 10 A5 30
*/
void loadFromFileHex(uint16_t addr, std::string fileName);

/*
    Load the memory content from a binary file at the
    specified address.
*/
void loadFromFileBin(uint16_t addr, std::string fileName);

std::string memoryDump(uint16_t start, uint16_t end);

#endif
