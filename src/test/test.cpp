#include <iostream>
#include "../cpu/MOS6502.h"
#include "../memory/Memory.h"

#define SUCCESS 0x36b9

int main(void) {

    loadFromFileBin(0x0400, "./6502_functional_test.bin");
    MOS6502 cpu = MOS6502(memoryWrite, memoryRead);

    std::cout << cpu.info() << "\n";

    cpu.setBreakpoint(SUCCESS);
    cpu.execute(0x0400, 0x3a19);

    std::cout << cpu.info() << "\n";

    return 0;
}
