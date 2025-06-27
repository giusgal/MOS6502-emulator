# MOS6502
Mos 6502 emulator in C++.

### Overview
- Jump table based
- All legal opcodes implemented and [tested](https://github.com/Klaus2m5/6502_65C02_functional_tests)
	- Decimal mode not implemented yet
- All addressing modes

### Project structure
- `./src/cpu/*`: Main files
- `./src/memory/*`: Memory utility functions used for debugging (loading, hex dump, write, read...)
- `./src/test/*`: Test suite

## Getting started

The MOS6502 class exposes the following functions:
- `MOS6502(fWrite w, fRead r)`: The class constructor takes as arguments two function pointers, namely `void (*fWrite)(uint16_t, uint8_t)` and `uint8_t (*fRead)(uint16_t)`. These functions are used by the MOS6502 object to access memory (or virtual memory-mapped devices), see below for an example
- `void IRQ()`: Generates a maskable interrupt
- `void NMI()`: Generates a non-maskable interrupt
- `void execute(uint16_t init_PC, uint16_t end_PC)`: Executes code in the address range [init_PC, end_PC] (endpoints included)
- `void reset()`: Processor reset
- `std::string info()`: Returns a string containing information about the processor (Registers, Status Register, Number of cycles)
- `void setBreakpoint(uint16_t addr)`: Set a breakpoint at the specified address (At the moment breakpoints can only be specified for addresses related to memory locations containing an opcode)
- `uint8_t/uint16_t get*()`: getters
- `void set*(uint8_t/uint16_t)`: setters

### Clock emulation
To disable clock speed emulation use the `-D _NO_DELAY_` option during the compilation step
    
### Example

```cpp
#include <iostream>
#include "MOS6502.h"

#define SIZE 0x10000

uint8_t memory[SIZE] = {0};

void memoryWrite(uint16_t addr, uint8_t data) {
    memory[addr] = data;
}

uint8_t memoryRead(uint16_t addr) {
    return memory[addr];
}

int main(void) {

    MOS6502 cpu = MOS6502(memoryWrite, memoryRead);

    // ...
    // load program into memory
    // ...

    uint16_t startAddr = // Set start address
    uint16_t endAddr   = // Set end address
    cpu.execute(startAddr, endAddr);

    return 0;
}
```

## Resources

- <https://www.masswerk.at/6502/6502_instruction_set.html>
- <http://6502.org/>
- <https://github.com/Klaus2m5/6502_65C02_functional_tests>
