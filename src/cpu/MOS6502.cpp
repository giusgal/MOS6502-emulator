#include "MOS6502.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

using BYTE = uint8_t;
using WORD = uint16_t;

MOS6502::MOS6502(fWrite const & w, fRead const & r):
    memoryWrite{w}, memoryRead{r}
{
    reset();
}

void MOS6502::IRQ() {
    if(SR[IF] != 1) {
        waitForCycles(7);
        push(PC/(16*16));
        push(PC);
        push(SR.to_ulong());
        PC = memoryRead(0xffff)*16*16+memoryRead(0xfffe);
        SR[IF] = 1;
    }
}
void MOS6502::NMI() {
    waitForCycles(7);
    push(PC/(16*16));
    push(PC);
    push(SR.to_ulong());
    PC = memoryRead(0xfffb)*16*16+memoryRead(0xfffa);
    SR[IF] = 1;
}

void MOS6502::reset() {
    PC = 0x0000; AC = 0x00; X  = 0x00;
    Y  = 0x00;   SR = 0x20; SP = 0xff;
}

void MOS6502::execute(WORD init_PC, WORD end_PC) {
    PC = init_PC;

    while(PC <= end_PC) {
        // Debugging
        if(breakpoint != 0 && PC == breakpoint) {
            break;
        }

        //Fetch instruction from memory
        BYTE inst{memoryRead(PC++)};

        //Execute
        callOpCode(inst);
    }
}

std::string MOS6502::info() const {
    std::ostringstream out;

    out << "SR:" << std::setw(8) << SR << " | "
        << "AC:" << std::hex << std::setw(2) << std::setfill('0') << +AC << " "
        << "X:"  << std::hex << std::setw(2) << std::setfill('0') << +X  << " "
        << "Y:"  << std::hex << std::setw(2) << std::setfill('0') << +Y  << " | "
        << "PC:" << std::hex << std::setw(4) << std::setfill('0') << +PC << " "
        << "SP:" << std::hex << std::setw(2) << std::setfill('0') << +SP << " "
        << "Cycles: " << std::hex << +cycles << "\n"
        << "   NV-BDIZC\n";
    return out.str();
}

// TODO: make possible to add more than one breakpoint
void MOS6502::setBreakpoint(uint16_t addr) {
    this->breakpoint = addr;
}

/**** Getter and Setter ****/
void MOS6502::setPC(uint16_t PC) {
    this->PC = PC;
}

void MOS6502::setAC(uint8_t AC) {
    this->AC = AC;
}

void MOS6502::setX(uint8_t X) {
    this->X = X;
}

void MOS6502::setY(uint8_t Y) {
    this->Y = Y;
}

void MOS6502::setSR(uint8_t SR) {
    this->SR = SR;
}

void MOS6502::setSP(uint8_t SP) {
    this->SP = SP;
}


uint16_t MOS6502::getPC() const {
    return PC;
}

uint8_t MOS6502::getAC() const {
    return AC;
}

uint8_t MOS6502::getX() const {
    return X;
}

uint8_t MOS6502::getY() const {
    return Y;
}

uint8_t MOS6502::getSR() const {
    return SR.to_ulong();
}

uint8_t MOS6502::getSP() const {
    return SP;
}
/***************************/


/**** Addressing Modes  ****/
uint16_t MOS6502::absolute() {
    BYTE LB = memoryRead(PC++);
    BYTE HB = memoryRead(PC++);
    return (HB*16*16+LB);
}

uint16_t MOS6502::absoluteX(bool& page_crossed) {
    BYTE LB = memoryRead(PC++);
    BYTE HB = memoryRead(PC++);
    page_crossed = (static_cast<uint8_t>(LB+X) < LB);
    return (HB*16*16+LB)+X;
}

uint16_t MOS6502::absoluteY(bool& page_crossed) {
    BYTE LB = memoryRead(PC++);
    BYTE HB = memoryRead(PC++);
    page_crossed = (static_cast<uint8_t>(LB+Y) < LB);
    return (HB*16*16+LB)+Y;
}

uint16_t MOS6502::indirect() {
    BYTE LB = memoryRead(PC++);
    BYTE HB = memoryRead(PC++);

    WORD target = HB*16*16+LB;

    BYTE LB_effective = memoryRead(target);
    BYTE HB_effective = memoryRead(target+1);

    return HB_effective*16*16+LB_effective;
}

uint16_t MOS6502::Xindirect() {
    BYTE LB = memoryRead(PC++);

    //target remain in zeropage
    BYTE target = LB + X;

    BYTE LB_effective = memoryRead(target);
    //(target+1) remain in zeropage
    BYTE HB_effective = memoryRead(static_cast<uint8_t>(target+1));

    return HB_effective*16*16+LB_effective;
}

uint16_t MOS6502::indirectY(bool& page_crossed) {
    BYTE LB = memoryRead(PC++);

    BYTE LB_effective = memoryRead(LB);
    //(LB+1) remain in zeropage
    BYTE HB_effective = memoryRead(static_cast<uint8_t>(LB+1));

    page_crossed = (static_cast<uint8_t>(LB_effective+Y) < LB_effective);

    return (HB_effective*16*16+LB_effective)+Y;
}

uint16_t MOS6502::relative(bool& page_crossed) {
    BYTE REL = memoryRead(PC++);

    WORD effective_address;

    //REL is a two's complement number:
    //  0xFF => -1
    //  0xFE => -2
    //  0x01 =>  1
    if((REL & (1U<<7))) {
        //If negative
        
        //Bitwise not then +1 to obtain the abs value of REL
        effective_address = PC - ( static_cast<uint8_t>( (~REL) + 1 ) ); 
    } else {
        //If positive
        effective_address = PC + REL;
    }

    page_crossed = 
        (static_cast<uint8_t>(effective_address/(16*16)) != static_cast<uint8_t>(PC/(16*16)) );

    return effective_address;
}

uint8_t MOS6502::zeropage() {
    return memoryRead(PC++);
}

uint8_t MOS6502::zeropageX() {
    //remain in zeropage
    return memoryRead(PC++) + X;
}

uint8_t MOS6502::zeropageY() {
    //remain in zeropage
    return memoryRead(PC++) + Y;
}
/***************************/

/**** Utility ****/
void MOS6502::callOpCode(BYTE index) {
    (this->*OPCODES[index])();
}

void MOS6502::waitForCycles(BYTE c) {
    #ifndef _NO_DELAY_
    cycles += c;
    std::this_thread::sleep_for(std::chrono::nanoseconds(500*c));
    #endif
}
/*****************/

/**** Stack Operations ****/
void MOS6502::push(uint8_t data) {
    memoryWrite(0x0100+(SP--), data);
}

uint8_t MOS6502::pull() {
    return memoryRead(0x0100+(++SP));
}
/**************************/

/**** Comparison ****/
void MOS6502::compareRM(uint8_t reg, uint8_t memory) {
    if(reg < memory) {
        SR[ZF] = 0;
        SR[CF] = 0;
        SR[NF] = ( static_cast<uint8_t>(reg - memory) & (1U<<7) );
    } else if(reg > memory) {
        SR[ZF] = 0;
        SR[CF] = 1;
        SR[NF] = ( static_cast<uint8_t>(reg - memory) & (1U<<7) );    
    } else {
        //reg == memory
        SR[ZF] = 1;
        SR[CF] = 1;
        SR[NF] = 0;
    }
}
/********************/

/**** Addition and Subtraction ****/
void MOS6502::addWithCarry(uint8_t memory) {
    if(SR[DF] == 0) { //Binary Mode
        //The result is saved on a 16 bits unsigned integer (WORD) to check
        //for a possible carry
        WORD tmp = AC + memory + SR[CF];
        //Overflow check (if AC and memory have the same sign, but tmp don't => overflow)
        SR[VF] = (AC^static_cast<uint8_t>(tmp))&(memory^static_cast<uint8_t>(tmp))&(1U<<7);
        // | Downcast to 8 bits
        // v
        AC = tmp;
        SR[ZF] = (AC==0);
        SR[NF] = (AC & (1U<<7));
        //Carry check
        SR[CF] = (tmp & (1U<<8));
    } else { //Decimal Mode
        //To be implemented
    }
}

void MOS6502::subWithBorrow(uint8_t memory) {
    /*
     How does it work?
      1. In two's complement a negative number is obtained
         by complementing the abs. value of the number and
         by adding 1 at the end.
          Example:
           A - memory = A + (-memory) = A + (~memory+1)
      2. The operation done by the MOS6502 during an SBC
         is (C: carry flag):
          A - memory - (~C) = A + ~memory + (1 - ~C)
            = A + ~memory + C
      3. So, basically an ADC with ~memory in place of
         memory.
    */
    addWithCarry(~memory);
}
/********************************/

/**** Istructions ****/
void MOS6502::BRKimp() { //
    waitForCycles(7);

    //Push HB first
    push((PC+1)/(16*16));
    //Push LB
    push(PC+1);
    //Set Bflag
    SR[BF] = 1;
    //Push Status register
    push(SR.to_ulong());
    //Unset BFlag (BFlag must be set only in the copy of the SR into the stack)
    SR[BF] = 0;
    //Modify the program counter to jump at the istruction poninted by the IRQ vector
    PC = memoryRead(0xffff)*16*16+memoryRead(0xfffe);
        /*     HB      */      /*     LB      */
    //Set the IFlag
    SR[IF] = 1;
}
void MOS6502::ORAxin() { //
    waitForCycles(6);
    ORA(Xindirect());
}
void MOS6502::ORAzpg() { //
    waitForCycles(3);
    ORA(zeropage());
}
void MOS6502::ASLzpg() { //
    waitForCycles(5);
    ASL(zeropage());
} 
void MOS6502::PHPimp() { //
    waitForCycles(3);
    BYTE tmpBF{SR[BF]};
    SR[BF] = 1;
    push(SR.to_ulong());
    SR[BF] = tmpBF;
}
void MOS6502::ORAimm() { //
    waitForCycles(2);
    ORA(PC++);
} 
void MOS6502::ASLimp() { //
    waitForCycles(2);
    SR[CF] = (AC & (1U<<7));
    AC*=2;
    SR[ZF] = (AC==0);
    SR[NF] = (AC & (1U<<7));
}
void MOS6502::ORAabs() { //
    waitForCycles(4);
    ORA(absolute());
} 
void MOS6502::ASLabs() {
    waitForCycles(6);
    ASL(absolute());
}

void MOS6502::BPLrel() { //
    waitForCycles(2);
    bool page_cross{false};

    if(SR[NF] == 0) {
        PC = relative(page_cross);
        if(page_cross) {
            waitForCycles(2);
        } else {
            waitForCycles(1);
        }
    } else {
        ++PC;
    }
}
void MOS6502::ORAiny() { //
    bool page_cross{false};
    WORD effective_address{indirectY(page_cross)};
    if(page_cross) waitForCycles(6);
    else           waitForCycles(5);
    ORA(effective_address);
} 
void MOS6502::ORAzpx() { //
    waitForCycles(4);
    ORA(zeropageX());
}
void MOS6502::ASLzpx() { //
    waitForCycles(6);
    ASL(zeropageX());
} 
void MOS6502::CLCimp() { //
    waitForCycles(2);
    SR[CF] = 0;
}
void MOS6502::ORAaby() { //
    bool page_cross{false};
    WORD effective_address{absoluteY(page_cross)};
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    ORA(effective_address);
} 
void MOS6502::ORAabx() { //
    bool page_cross{false};
    WORD effective_address{absoluteX(page_cross)};
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    ORA(effective_address);
}
void MOS6502::ASLabx() { //
    waitForCycles(7);
    bool page_cross;
    ASL(absoluteX(page_cross));
}

void MOS6502::JSRabs() { //
    waitForCycles(6);
    push((PC+1)/(16*16)); //push HB first
    push(PC+1);           //push LB
    PC = absolute();
}
void MOS6502::ANDxin() { //
    waitForCycles(6);
    AND(Xindirect());
} 
void MOS6502::BITzpg() { //
    waitForCycles(3);
    BIT(zeropage());
}
void MOS6502::ANDzpg() { //
    waitForCycles(3);
    AND(zeropage());
} 
void MOS6502::ROLzpg() { //
    waitForCycles(5);
    ROL(zeropage());
}
void MOS6502::PLPimp() { //
    waitForCycles(4);

    BYTE tmpBF{SR[BF]};
    BYTE tmpBit5{SR[5]};
    SR = pull();
    //ignore break flag
    SR[BF] = tmpBF;
    //ignore bit 5
    SR[5] = tmpBit5;
} 
void MOS6502::ANDimm() { //
    waitForCycles(2);
    AND(PC++);
}
void MOS6502::ROLimp() { //
    waitForCycles(2);
    BYTE tmpCF{SR[CF]};
    SR[CF] = (AC & (1U<<7));
    AC <<= 1;
    AC+=tmpCF;
    SR[ZF] = (AC==0);
    SR[NF] = (AC & (1U<<7));
} 
void MOS6502::BITabs() { //
    waitForCycles(4);
    BIT(absolute());
}
void MOS6502::ANDabs() { //
    waitForCycles(4);
    AND(absolute());
} 
void MOS6502::ROLabs() { //
    waitForCycles(6);
    ROL(absolute());
}

void MOS6502::BMIrel() { //
    waitForCycles(2);
    bool page_cross{false};

    if(SR[NF] == 1) {
        PC = relative(page_cross);
        if(page_cross) {
            waitForCycles(2);
        } else {
            waitForCycles(1);
        }
    } else {
        ++PC;
    }
}
void MOS6502::ANDiny() { //
    bool page_cross{false};
    WORD effective_address{indirectY(page_cross)};
    if(page_cross) waitForCycles(6);
    else           waitForCycles(5);
    AND(effective_address);
} 
void MOS6502::ANDzpx() { //
    waitForCycles(4);
    AND(zeropageX());
}
void MOS6502::ROLzpx() { //
    waitForCycles(6);
    ROL(zeropageX());
}
void MOS6502::SECimp() { //
    waitForCycles(2);
    SR[CF] = 1;
}
void MOS6502::ANDaby() { //
    bool page_cross{false};
    WORD effective_address{absoluteY(page_cross)};
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    AND(effective_address);
} 
void MOS6502::ANDabx() { //
    bool page_cross{false};
    WORD effective_address{absoluteX(page_cross)};
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    AND(effective_address);
}
void MOS6502::ROLabx() { //
    waitForCycles(7);
    bool page_cross;
    ROL(absoluteX(page_cross));
}

void MOS6502::RTIimp() { //
    waitForCycles(6);

    BYTE tmpBF{SR[BF]};
    SR = pull();
    SR[BF] = tmpBF;

    BYTE LB{pull()};
    BYTE HB{pull()};
    PC = HB*16*16+LB;
}
void MOS6502::EORxin() { //
    waitForCycles(6);
    EOR(Xindirect());
} 
void MOS6502::EORzpg() { //
    waitForCycles(3);
    EOR(zeropage());
}
void MOS6502::LSRzpg() { //
    waitForCycles(5);
    LSR(zeropage());
} 
void MOS6502::PHAimp() { //
    waitForCycles(3);
    push(AC);
}
void MOS6502::EORimm() { //
    waitForCycles(2);
    EOR(PC++);
} 
void MOS6502::LSRimp() { //
    waitForCycles(2);
    SR[CF] = (AC & 1U);
    AC/=2;
    SR[ZF] = (AC==0);
    SR[NF] = 0;
}
void MOS6502::JMPabs() { //
    waitForCycles(3);
    PC = absolute();
} 
void MOS6502::EORabs() { //
    waitForCycles(4);
    EOR(absolute());
}
void MOS6502::LSRabs() { //
    waitForCycles(6);
    LSR(absolute());
}

void MOS6502::BVCrel() { //
    waitForCycles(2);
    bool page_cross{false};

    if(SR[VF] == 0) {
        PC = relative(page_cross);
        if(page_cross) {
            waitForCycles(2);
        } else {
            waitForCycles(1);
        }
    } else {
        ++PC;
    }
}
void MOS6502::EORiny() { //
    bool page_cross{false};
    WORD effective_address{indirectY(page_cross)};
    if(page_cross) waitForCycles(6);
    else           waitForCycles(5);
    EOR(effective_address);  
} 
void MOS6502::EORzpx() { //
    waitForCycles(4);
    EOR(zeropageX());
}
void MOS6502::LSRzpx() { //
    waitForCycles(6);
    LSR(zeropageX());
} 
void MOS6502::CLIimp() { //
    waitForCycles(2);
    SR[IF] = 0;
}
void MOS6502::EORaby() { //
    bool page_cross{false};
    WORD effective_address{absoluteY(page_cross)};
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    EOR(effective_address);
} 
void MOS6502::EORabx() { //
    bool page_cross{false};
    WORD effective_address{absoluteX(page_cross)};
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    EOR(effective_address);
}
void MOS6502::LSRabx() { //
    waitForCycles(7);
    bool page_cross;
    LSR(absoluteX(page_cross));
}

void MOS6502::RTSimp() { //
    waitForCycles(6);

    BYTE LB{pull()};
    BYTE HB{pull()};
    WORD address = HB*16*16+LB;

    PC = address+1;
}
void MOS6502::ADCxin() { //
    waitForCycles(6);
    addWithCarry(memoryRead(Xindirect()));
}
void MOS6502::ADCzpg() { //
    waitForCycles(3);
    addWithCarry(memoryRead(zeropage()));
}
void MOS6502::RORzpg() { //
    waitForCycles(5);
    ROR(zeropage());
}
void MOS6502::PLAimp() { //
    waitForCycles(4);

    AC = pull();

    SR[ZF] = (AC==0);
    SR[NF] = (AC & (1U<<7));
}
void MOS6502::ADCimm() { //
    waitForCycles(2);
    addWithCarry(memoryRead(PC++));
}
void MOS6502::RORimp() { //
    waitForCycles(2);
    BYTE tmpCF{SR[CF]};
    SR[CF] = (AC & 1U);
    AC >>= 1;
    AC += (tmpCF<<7);
    SR[ZF] = (AC==0);
    SR[NF] = (AC & (1U<<7));
}
void MOS6502::JMPind() { //
    waitForCycles(5);
    PC = indirect();
} 
void MOS6502::ADCabs() { //
    waitForCycles(4);
    addWithCarry(memoryRead(absolute()));
}
void MOS6502::RORabs() { //
    waitForCycles(6);
    ROR(absolute());
}

void MOS6502::BVSrel() { //
    waitForCycles(2);
    bool page_cross{false};

    if(SR[VF] == 1) {
        PC = relative(page_cross);
        if(page_cross) {
            waitForCycles(2);
        } else {
            waitForCycles(1);
        }
    } else {
        ++PC;
    }
}
void MOS6502::ADCiny() { //
    bool page_cross{false};
    BYTE memory{memoryRead(indirectY(page_cross))};

    if(page_cross) waitForCycles(6);
    else           waitForCycles(5);

    addWithCarry(memory);
}
void MOS6502::ADCzpx() { //
    waitForCycles(4);
    addWithCarry(memoryRead(zeropageX()));
}
void MOS6502::RORzpx() { //
    waitForCycles(6);
    ROR(zeropageX());
} 
void MOS6502::SEIimp() { //
    waitForCycles(2);
    SR[IF] = 1;
}
void MOS6502::ADCaby() { //
    bool page_cross{false};
    BYTE memory{memoryRead(absoluteY(page_cross))};
    
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);

    addWithCarry(memory);
}
void MOS6502::ADCabx() { //
    bool page_cross{false};
    BYTE memory{memoryRead(absoluteX(page_cross))};
    
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);

    addWithCarry(memory);
}
void MOS6502::RORabx() { //
    waitForCycles(7);
    bool page_cross;
    ROR(absoluteX(page_cross));
}

void MOS6502::STAxin() { //
    waitForCycles(6);
    memoryWrite(Xindirect(), AC);
}
void MOS6502::STYzpg() { //
    waitForCycles(3);
    memoryWrite(zeropage(), Y);
} 
void MOS6502::STAzpg() { //
    waitForCycles(3);
    memoryWrite(zeropage(), AC);
}
void MOS6502::STXzpg() { //
    waitForCycles(3);
    memoryWrite(zeropage(), X);
} 
void MOS6502::DEYimp() { //
    waitForCycles(2);
    --Y;
    SR[ZF] = (Y==0);
    SR[NF] = (Y & (1U<<7));
}
void MOS6502::TXAimp() { //
    waitForCycles(2);
    AC = X;

    SR[ZF] = (AC==0);
    SR[NF] = (AC & (1U<<7));
}
void MOS6502::STYabs() { //
    waitForCycles(4);
    memoryWrite(absolute(), Y);
}
void MOS6502::STAabs() { //
    waitForCycles(4);
    memoryWrite(absolute(), AC);
} 
void MOS6502::STXabs() { //
    waitForCycles(4);
    memoryWrite(absolute(), X);
}

void MOS6502::BCCrel() { //
    waitForCycles(2);
    bool page_cross{false};

    if(SR[CF] == 0) {
        PC = relative(page_cross);
        if(page_cross) {
            waitForCycles(2);
        } else {
            waitForCycles(1);
        }
    } else {
        ++PC;
    }
}
void MOS6502::STAiny() { //
    waitForCycles(6);
    bool page_cross;
    memoryWrite(indirectY(page_cross), AC);
}
void MOS6502::STYzpx() { //
    waitForCycles(4);
    memoryWrite(zeropageX(), Y);
}
void MOS6502::STAzpx() { //
    waitForCycles(4);
    memoryWrite(zeropageX(), AC);
} 
void MOS6502::STXzpy() { //
    waitForCycles(4);
    memoryWrite(zeropageY(), X);
}
void MOS6502::TYAimp() { //
    waitForCycles(2);
    AC = Y;

    SR[ZF] = (AC==0);
    SR[NF] = (AC & (1U<<7));
}
void MOS6502::STAaby() { //
    waitForCycles(5);
    bool page_cross;
    memoryWrite(absoluteY(page_cross), AC);
}
void MOS6502::TXSimp() { //
    waitForCycles(2);
    SP = X;
}
void MOS6502::STAabx() { //
    waitForCycles(5);
    bool page_cross;
    memoryWrite(absoluteX(page_cross), AC);
}

void MOS6502::LDYimm() { //
    waitForCycles(2);
    LDY(PC++);
} 
void MOS6502::LDAxin() { //
    waitForCycles(6);
    LDA(Xindirect());
}
void MOS6502::LDXimm() { //
    waitForCycles(2);
    LDX(PC++);
}
void MOS6502::LDYzpg() { //
    waitForCycles(3);
    LDY(zeropage());
} 
void MOS6502::LDAzpg() { //
    waitForCycles(3);
    LDA(zeropage());
} 
void MOS6502::LDXzpg() { //
    waitForCycles(3);
    LDX(zeropage());
} 
void MOS6502::TAYimp() { //
    waitForCycles(2);
    Y = AC;
    SR[ZF] = (Y==0);
    SR[NF] = (Y & (1U<<7));
}
void MOS6502::LDAimm() { //
    waitForCycles(2);
    LDA(PC++);
} 
void MOS6502::TAXimp() { //
    waitForCycles(2);
    X = AC;
    SR[ZF] = (X==0);
    SR[NF] = (X & (1U<<7));
}
void MOS6502::LDYabs() { //
    waitForCycles(4);
    LDY(absolute());
} 
void MOS6502::LDAabs() { //
    waitForCycles(4);
    LDA(absolute());
}
void MOS6502::LDXabs() { //
    waitForCycles(4);
    LDX(absolute());
}

void MOS6502::BCSrel() { //
    waitForCycles(2);
    bool page_cross{false};

    if(SR[CF] == 1) {
        PC = relative(page_cross);
        if(page_cross) {
            waitForCycles(2);
        } else {
            waitForCycles(1);
        }
    } else {
        ++PC;
    }
}
void MOS6502::LDAiny() { //
    bool page_cross{false};
    WORD effective_address = indirectY(page_cross);
    if(page_cross) waitForCycles(6);
    else           waitForCycles(5);
    LDA(effective_address);
}
void MOS6502::LDYzpx() { //
    waitForCycles(4);
    LDY(zeropageX());
} 
void MOS6502::LDAzpx() { //
    waitForCycles(4);
    LDA(zeropageX());
} 
void MOS6502::LDXzpy() { //
    waitForCycles(4);
    LDX(zeropageY());
} 
void MOS6502::CLVimp() { //
    waitForCycles(2);
    SR[VF] = 0;
}
void MOS6502::LDAaby() { //
    bool page_cross{false};
    WORD effective_address = absoluteY(page_cross);
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    LDA(effective_address);
} 
void MOS6502::TSXimp() { //
    waitForCycles(2);
    X = SP;
    SR[ZF] = (X==0);
    SR[NF] = (X & (1U<<7));
}
void MOS6502::LDYabx() { //
    bool page_cross{false};
    WORD effective_address = absoluteX(page_cross);
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    LDY(effective_address);
} 
void MOS6502::LDAabx() { //
    bool page_cross{false};
    WORD effective_address = absoluteX(page_cross);
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    LDA(effective_address);
}
void MOS6502::LDXaby() { //
    bool page_cross{false};
    WORD effective_address = absoluteY(page_cross);
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    LDX(effective_address);
}

void MOS6502::CPYimm() { //
    waitForCycles(2);
    compareRM(Y, memoryRead(PC++));
}
void MOS6502::CMPxin() { //
    waitForCycles(6);
    compareRM(AC, memoryRead(Xindirect()));
} 
void MOS6502::CPYzpg() { //
    waitForCycles(3);
    compareRM(Y, memoryRead(zeropage()));
} 
void MOS6502::CMPzpg() { //
    waitForCycles(3);
    compareRM(AC, memoryRead(zeropage()));
} 
void MOS6502::DECzpg() { //
    waitForCycles(5);
    DEC(zeropage());
}
void MOS6502::INYimp() { //
    waitForCycles(2);
    ++Y;
    SR[ZF] = (Y==0);
    SR[NF] = (Y & (1U<<7));
} 
void MOS6502::CMPimm() { //
    waitForCycles(2);
    compareRM(AC, memoryRead(PC++));
} 
void MOS6502::DEXimp() { //
    waitForCycles(2);
    --X;
    SR[ZF] = (X==0);
    SR[NF] = (X & (1U<<7));
} 
void MOS6502::CPYabs() { //
    waitForCycles(2);
    compareRM(Y, memoryRead(absolute()));
} 
void MOS6502::CMPabs() { //
    waitForCycles(4);
    compareRM(AC, memoryRead(absolute()));
} 
void MOS6502::DECabs() { //
    waitForCycles(6);
    DEC(absolute());
}

void MOS6502::BNErel() { //
    waitForCycles(2);
    bool page_cross{false};

    if(SR[ZF] == 0) {
        PC = relative(page_cross);
        if(page_cross) {
            waitForCycles(2);
        } else {
            waitForCycles(1);
        }
    } else {
        ++PC;
    }
}
void MOS6502::CMPiny() { //
    bool page_cross{false};
    BYTE memory{memoryRead(indirectY(page_cross))};
    if(page_cross) waitForCycles(6);
    else           waitForCycles(5);
    compareRM(AC, memory);
}
void MOS6502::CMPzpx() { //
    waitForCycles(4);
    compareRM(AC, memoryRead(zeropageX()));
} 
void MOS6502::DECzpx() { //
    waitForCycles(6);
    DEC(zeropageX());
}
void MOS6502::CLDimp() { //
    waitForCycles(2);
    SR[DF] = 0;
}
void MOS6502::CMPaby() { //
    bool page_cross{false};
    BYTE memory{memoryRead(absoluteY(page_cross))};
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    compareRM(AC, memory);
} 
void MOS6502::CMPabx() { //
    bool page_cross{false};
    BYTE memory{memoryRead(absoluteX(page_cross))};
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    compareRM(AC, memory);
} 
void MOS6502::DECabx() { //
    waitForCycles(7);
    bool page_cross;
    DEC(absoluteX(page_cross));
}

void MOS6502::CPXimm() { //
    waitForCycles(2);
    compareRM(X, memoryRead(PC++));
} 
void MOS6502::SBCxin() { //
    waitForCycles(6);
    subWithBorrow(memoryRead(Xindirect()));
}
void MOS6502::CPXzpg() { //
    waitForCycles(3);
    compareRM(X, memoryRead(zeropage()));
} 
void MOS6502::SBCzpg() { //
    waitForCycles(3);
    subWithBorrow(memoryRead(zeropage()));
}
void MOS6502::INCzpg() { //
    waitForCycles(5);
    INC(zeropage());
}
void MOS6502::INXimp() { //
    waitForCycles(2);
    ++X;
    SR[ZF] = (X==0);
    SR[NF] = (X & (1U<<7));
}
void MOS6502::SBCimm() { //
    waitForCycles(2);
    subWithBorrow(memoryRead(PC++));
}
void MOS6502::NOPimp() { //
    waitForCycles(2);
} 
void MOS6502::CPXabs() { //
    waitForCycles(4);
    compareRM(X, memoryRead(absolute()));
} 
void MOS6502::SBCabs() { //
    waitForCycles(4);
    subWithBorrow(memoryRead(absolute()));
}
void MOS6502::INCabs() { //
    waitForCycles(6);
    INC(absolute());
}

void MOS6502::BEQrel() { //
    waitForCycles(2);
    bool page_cross{false};

    if(SR[ZF] == 1) {
        PC = relative(page_cross);
        if(page_cross) {
            waitForCycles(2);
        } else {
            waitForCycles(1);
        }
    } else {
        ++PC;
    }
} 
void MOS6502::SBCiny() { //
    bool page_cross{false};
    BYTE memory{memoryRead(indirectY(page_cross))};
    if(page_cross) waitForCycles(6);
    else           waitForCycles(5);
    subWithBorrow(memory);
}
void MOS6502::SBCzpx() { //
    waitForCycles(4);
    subWithBorrow(memoryRead(zeropageX()));
}
void MOS6502::INCzpx() { //
    waitForCycles(6);
    INC(zeropageX());
} 
void MOS6502::SEDimp() { //
    waitForCycles(2);
    SR[DF] = 1;
}
void MOS6502::SBCaby() { //
    bool page_cross{false};
    BYTE memory{memoryRead(absoluteY(page_cross))};
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    subWithBorrow(memory);
}
void MOS6502::SBCabx() { //
    bool page_cross{false};
    BYTE memory{memoryRead(absoluteX(page_cross))};
    if(page_cross) waitForCycles(5);
    else           waitForCycles(4);
    subWithBorrow(memory);
}
void MOS6502::INCabx() { //
    waitForCycles(7);
    bool page_cross;
    INC(absoluteX(page_cross));
}

void MOS6502::OPCill() {
    NOPimp();
}


void MOS6502::AND(WORD address) {
    AC &= memoryRead(address);
    SR[ZF] = (AC==0);
    SR[NF] = (AC & (1U<<7));
}

void MOS6502::ASL(WORD address) {
    BYTE data{memoryRead(address)};
    SR[CF] = (data & (1U<<7));
    data*=2;
    SR[ZF] = (data==0);
    SR[NF] = (data & (1U<<7));
    memoryWrite(address, data);
}

void MOS6502::BIT(WORD address) {
    BYTE data{memoryRead(address)};
    SR[NF] = (data & (1U<<7));
    SR[VF] = (data & (1U<<6));
    SR[ZF] = ((data&AC)==0);  
}

void MOS6502::DEC(WORD address) {
    BYTE data{memoryRead(address)};
    --data;
    memoryWrite(address, data);
    SR[ZF] = (data==0);
    SR[NF] = (data & (1U<<7));
}

void MOS6502::EOR(WORD address) {
    AC ^= memoryRead(address);
    SR[ZF] = (AC==0);
    SR[NF] = (AC & (1U<<7));
}

void MOS6502::INC(WORD address) {
    BYTE data{memoryRead(address)};
    ++data;
    memoryWrite(address, data);
    SR[ZF] = (data==0);
    SR[NF] = (data & (1U<<7));
}

void MOS6502::LDA(WORD address) {
    AC = memoryRead(address);
    SR[ZF] = (AC==0);
    SR[NF] = (AC & (1U<<7));
}

void MOS6502::LDX(WORD address) {
    X = memoryRead(address);
    SR[ZF] = (X==0);
    SR[NF] = (X & (1U<<7));
}

void MOS6502::LDY(WORD address) {
    Y = memoryRead(address);
    SR[ZF] = (Y==0);
    SR[NF] = (Y & (1U<<7));
}

void MOS6502::LSR(WORD address) {
    BYTE data{memoryRead(address)};
    SR[CF] = (data & 1U);
    data/=2;
    SR[ZF] = (data==0);
    SR[NF] = 0;
    memoryWrite(address, data);
}

void MOS6502::ORA(WORD address) {
    AC |= memoryRead(address);
    SR[ZF] = (AC==0);
    SR[NF] = (AC & (1U<<7));
}

void MOS6502::ROL(WORD address) {
    BYTE tmpCF{SR[CF]};
    BYTE data{memoryRead(address)};
    SR[CF] = (data & (1U<<7));
    data <<= 1;
    data+=tmpCF;
    memoryWrite(address, data);
    SR[ZF] = (data==0);
    SR[NF] = (data & (1U<<7)); 
}

void MOS6502::ROR(WORD address) {
    BYTE tmpCF{SR[CF]};
    BYTE data{memoryRead(address)};
    SR[CF] = (data & 1U);
    data >>= 1;
    data += (tmpCF<<7);
    memoryWrite(address, data);
    SR[ZF] = (data == 0);
    SR[NF] = (data & (1U << 7));
}
/*********************/
