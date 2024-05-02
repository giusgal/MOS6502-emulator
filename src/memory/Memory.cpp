#include "Memory.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

static uint8_t memory[SIZE] = {0};

/*
    Convert an hex byte string to an integer
*/
static uint8_t strToByte(std::string str) {
    uint8_t byte = 0x00;

    for(int i = 0; i < 2; ++i) {
        if(str[i] >= '0' && str[i] <= '9') {
            byte += (str[i] - '0')*((1-i)*15+1);
        }
        else if(str[i] >= 'A' && str[i] <= 'F') {
            byte += (str[i] - 'A' + 10)*((1-i)*15+1);
        }
        else {
            std::cout << "ERROR: couldn't load program\n";
            exit(1);
        }
    }

    return byte;
}

void memoryWrite(uint16_t addr, uint8_t data) {
    memory[addr] = data;
}

uint8_t memoryRead(uint16_t addr) {
    return memory[addr];
}

void loadFromFileHex(uint16_t addr, std::string fileName) {
    std::ifstream file(fileName);
    
    if(!file) {
        std::cout << "ERROR: couldn't open file\n";
        exit(1);
    }

    std::string charByte;

    while(file >> charByte) {
        memory[addr++] = strToByte(charByte);
    }

    file.close();
}

void loadFromFileBin(uint16_t addr, std::string fileName) {
    std::ifstream file(fileName, std::ios::binary);

    if(!file) {
        std::cout << "ERROR: couldn't open file\n";
        exit(1);
    }

    uint8_t buffer[SIZE];

    file.read((char*)buffer,SIZE);
    size_t byteRead = file.gcount();

    for(size_t i = 0; i < byteRead; ++i) {
        memory[addr++] = buffer[i];
    }

    file.close();
}

std::string memoryDump(uint16_t start, uint16_t end) {
    std::ostringstream out;

    for(uint16_t i = start; i <= end; ++i) {
        if(i % 16 == 0) {
            out << "\n" << std::hex
                << std::setw(4) << std::setfill('0')
                << +i << ": ";
        }
        out << std::hex 
            << std::setw(2) << std::setfill('0')
            << +memory[i] << " ";
    }
    out << "\n";

    return out.str();
}
