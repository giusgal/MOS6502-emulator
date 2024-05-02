#ifndef MOS6502_H
#define MOS6502_H

#include <string>
#include <bitset>

typedef void (*fWrite)(uint16_t, uint8_t);
typedef uint8_t (*fRead)(uint16_t);

class MOS6502 {
public:
    MOS6502(fWrite w, fRead r);
    void IRQ();
    void NMI();
    void execute(uint16_t init_PC, uint16_t end_PC);
    void reset();

    std::string info() const;
    void setBreakpoint(uint16_t addr);

    void setPC(uint16_t PC);
    void setAC(uint8_t AC);
    void setX(uint8_t X);
    void setY(uint8_t Y);
    void setSR(uint8_t SR);
    void setSP(uint8_t SP);

    uint16_t getPC();
    uint8_t getAC();
    uint8_t getX();
    uint8_t getY();
    uint8_t getSR();
    uint8_t getSP();

private:
    /**** Registers and Memory ****/
    uint16_t PC;                        //Program Counter
    uint8_t AC;                         //Accumulator
    uint8_t X;                          //X Register
    uint8_t Y;                          //Y Register
    std::bitset<8> SR;                  //Status Register
    uint8_t SP;                         //Stack Pointer

    const uint8_t CF{0};                //Carry flag index
    const uint8_t ZF{1};                //Zero flag index
    const uint8_t IF{2};                //Interrupt flag index
    const uint8_t DF{3};                //Decimal flag index
    const uint8_t BF{4};                //Break flag index
    const uint8_t VF{6};                //Overflow flag index
    const uint8_t NF{7};                //Negative flag index

    /**** Others ****/
    uint64_t cycles = 0;                //Cycles counter
    uint8_t currentOpCodeCycles = 0;    //Cycles for the current opCode
    uint16_t breakpoint = 0;            //Breakpoint (for debugging)

    /**** Function pointers for memory read and write operations ****/
    fWrite memoryWrite;
    fRead memoryRead;

    /**** Istructions ****
     *  The first three characters represent the real name of the instruction.
     *  The last three characters indicate the addressing mode:
     *      ---abs: absolute
     *      ---abx: absolute,X indexed
     *      ---aby: absolute,Y indexed
     *      ---imm: immediate
     *      ---ind: indirect
     *      ---xin: X indexed,indirect
     *      ---iny: indirect,Y indexed
     *      ---rel: relative
     *      ---zpg: zeropage
     *      ---zpx: zeropage,X indexed
     *      ---zpy: zeropage,Y indexed
     * 
     *  Note: OPCill stands for "illegal opcode" (NOPimp())
    */
    void BRKimp(); void ORAxin();
    void ORAzpg(); void ASLzpg(); 
    void PHPimp(); void ORAimm(); 
    void ASLimp(); void ORAabs(); 
    void ASLabs();

    void BPLrel(); void ORAiny(); 
    void ORAzpx(); void ASLzpx(); 
    void CLCimp(); void ORAaby(); 
    void ORAabx(); void ASLabx();

    void JSRabs(); void ANDxin(); 
    void BITzpg(); void ANDzpg(); 
    void ROLzpg(); void PLPimp(); 
    void ANDimm(); void ROLimp(); 
    void BITabs(); void ANDabs(); 
    void ROLabs();

    void BMIrel(); void ANDiny(); 
    void ANDzpx(); void ROLzpx(); 
    void SECimp(); void ANDaby(); 
    void ANDabx(); void ROLabx();

    void RTIimp(); void EORxin(); 
    void EORzpg(); void LSRzpg(); 
    void PHAimp(); void EORimm(); 
    void LSRimp(); void JMPabs(); 
    void EORabs(); void LSRabs();

    void BVCrel(); void EORiny(); 
    void EORzpx(); void LSRzpx(); 
    void CLIimp(); void EORaby(); 
    void EORabx(); void LSRabx();

    void RTSimp(); void ADCxin(); 
    void ADCzpg(); void RORzpg(); 
    void PLAimp(); void ADCimm(); 
    void RORimp(); void JMPind(); 
    void ADCabs(); void RORabs();

    void BVSrel(); void ADCiny(); 
    void ADCzpx(); void RORzpx(); 
    void SEIimp(); void ADCaby(); 
    void ADCabx(); void RORabx();

    void STAxin(); void STYzpg(); 
    void STAzpg(); void STXzpg(); 
    void DEYimp(); void TXAimp(); 
    void STYabs(); void STAabs(); 
    void STXabs();

    void BCCrel(); void STAiny(); 
    void STYzpx(); void STAzpx(); 
    void STXzpy(); void TYAimp(); 
    void STAaby(); void TXSimp(); 
    void STAabx();

    void LDYimm(); void LDAxin(); 
    void LDXimm(); void LDYzpg(); 
    void LDAzpg(); void LDXzpg(); 
    void TAYimp(); void LDAimm(); 
    void TAXimp(); void LDYabs(); 
    void LDAabs(); void LDXabs();

    void BCSrel(); void LDAiny();
    void LDYzpx(); void LDAzpx(); 
    void LDXzpy(); void CLVimp(); 
    void LDAaby(); void TSXimp(); 
    void LDYabx(); void LDAabx(); 
    void LDXaby();

    void CPYimm(); void CMPxin(); 
    void CPYzpg(); void CMPzpg(); 
    void DECzpg(); void INYimp(); 
    void CMPimm(); void DEXimp(); 
    void CPYabs(); void CMPabs(); 
    void DECabs();

    void BNErel(); void CMPiny(); 
    void CMPzpx(); void DECzpx(); 
    void CLDimp(); void CMPaby(); 
    void CMPabx(); void DECabx();

    void CPXimm(); void SBCxin(); 
    void CPXzpg(); void SBCzpg(); 
    void INCzpg(); void INXimp(); 
    void SBCimm(); void NOPimp(); 
    void CPXabs(); void SBCabs(); 
    void INCabs();

    void BEQrel(); void SBCiny(); 
    void SBCzpx(); void INCzpx(); 
    void SEDimp(); void SBCaby(); 
    void SBCabx(); void INCabx();

    void OPCill();

    void AND(uint16_t);
    void ASL(uint16_t);
    void BIT(uint16_t);
    void DEC(uint16_t);
    void EOR(uint16_t);
    void INC(uint16_t);
    void LDA(uint16_t);
    void LDX(uint16_t);
    void LDY(uint16_t);
    void LSR(uint16_t);
    void ORA(uint16_t);
    void ROL(uint16_t);
    void ROR(uint16_t);

    /**** Addressing Modes ****/
    uint16_t absolute();
    uint16_t absoluteX(bool&);
    uint16_t absoluteY(bool&);
    uint16_t indirect();
    uint16_t Xindirect();
    uint16_t indirectY(bool&);
    uint16_t relative(bool&);
    uint8_t zeropage();
    uint8_t zeropageX();
    uint8_t zeropageY();

    /**** Jump Table ****/
    //https://www.masswerk.at/6502/6502_instruction_set.html
    typedef void (MOS6502::*opc)();
    opc OPCODES[0x100] = {
               /*    -0      */  /*    -1      */  /*    -2      */  /*    -3      */  /*    -4      */  /*    -5      */  /*    -6      */  /*    -7      */  /*    -8      */  /*    -9      */  /*    -A      */  /*    -B      */  /*    -C      */  /*    -D      */  /*    -E      */  /*    -F      */
        /*0-*/ &MOS6502::BRKimp, &MOS6502::ORAxin, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::ORAzpg, &MOS6502::ASLzpg, &MOS6502::OPCill, &MOS6502::PHPimp, &MOS6502::ORAimm, &MOS6502::ASLimp, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::ORAabs, &MOS6502::ASLabs, &MOS6502::OPCill,
        /*1-*/ &MOS6502::BPLrel, &MOS6502::ORAiny, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::ORAzpx, &MOS6502::ASLzpx, &MOS6502::OPCill, &MOS6502::CLCimp, &MOS6502::ORAaby, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::ORAabx, &MOS6502::ASLabx, &MOS6502::OPCill, 
        /*2-*/ &MOS6502::JSRabs, &MOS6502::ANDxin, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::BITzpg, &MOS6502::ANDzpg, &MOS6502::ROLzpg, &MOS6502::OPCill, &MOS6502::PLPimp, &MOS6502::ANDimm, &MOS6502::ROLimp, &MOS6502::OPCill, &MOS6502::BITabs, &MOS6502::ANDabs, &MOS6502::ROLabs, &MOS6502::OPCill, 
        /*3-*/ &MOS6502::BMIrel, &MOS6502::ANDiny, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::ANDzpx, &MOS6502::ROLzpx, &MOS6502::OPCill, &MOS6502::SECimp, &MOS6502::ANDaby, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::ANDabx, &MOS6502::ROLabx, &MOS6502::OPCill, 
        /*4-*/ &MOS6502::RTIimp, &MOS6502::EORxin, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::EORzpg, &MOS6502::LSRzpg, &MOS6502::OPCill, &MOS6502::PHAimp, &MOS6502::EORimm, &MOS6502::LSRimp, &MOS6502::OPCill, &MOS6502::JMPabs, &MOS6502::EORabs, &MOS6502::LSRabs, &MOS6502::OPCill, 
        /*5-*/ &MOS6502::BVCrel, &MOS6502::EORiny, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::EORzpx, &MOS6502::LSRzpx, &MOS6502::OPCill, &MOS6502::CLIimp, &MOS6502::EORaby, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::EORabx, &MOS6502::LSRabx, &MOS6502::OPCill, 
        /*6-*/ &MOS6502::RTSimp, &MOS6502::ADCxin, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::ADCzpg, &MOS6502::RORzpg, &MOS6502::OPCill, &MOS6502::PLAimp, &MOS6502::ADCimm, &MOS6502::RORimp, &MOS6502::OPCill, &MOS6502::JMPind, &MOS6502::ADCabs, &MOS6502::RORabs, &MOS6502::OPCill, 
        /*7-*/ &MOS6502::BVSrel, &MOS6502::ADCiny, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::ADCzpx, &MOS6502::RORzpx, &MOS6502::OPCill, &MOS6502::SEIimp, &MOS6502::ADCaby, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::ADCabx, &MOS6502::RORabx, &MOS6502::OPCill, 
        /*8-*/ &MOS6502::OPCill, &MOS6502::STAxin, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::STYzpg, &MOS6502::STAzpg, &MOS6502::STXzpg, &MOS6502::OPCill, &MOS6502::DEYimp, &MOS6502::OPCill, &MOS6502::TXAimp, &MOS6502::OPCill, &MOS6502::STYabs, &MOS6502::STAabs, &MOS6502::STXabs, &MOS6502::OPCill, 
        /*9-*/ &MOS6502::BCCrel, &MOS6502::STAiny, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::STYzpx, &MOS6502::STAzpx, &MOS6502::STXzpy, &MOS6502::OPCill, &MOS6502::TYAimp, &MOS6502::STAaby, &MOS6502::TXSimp, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::STAabx, &MOS6502::OPCill, &MOS6502::OPCill, 
        /*A-*/ &MOS6502::LDYimm, &MOS6502::LDAxin, &MOS6502::LDXimm, &MOS6502::OPCill, &MOS6502::LDYzpg, &MOS6502::LDAzpg, &MOS6502::LDXzpg, &MOS6502::OPCill, &MOS6502::TAYimp, &MOS6502::LDAimm, &MOS6502::TAXimp, &MOS6502::OPCill, &MOS6502::LDYabs, &MOS6502::LDAabs, &MOS6502::LDXabs, &MOS6502::OPCill, 
        /*B-*/ &MOS6502::BCSrel, &MOS6502::LDAiny, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::LDYzpx, &MOS6502::LDAzpx, &MOS6502::LDXzpy, &MOS6502::OPCill, &MOS6502::CLVimp, &MOS6502::LDAaby, &MOS6502::TSXimp, &MOS6502::OPCill, &MOS6502::LDYabx, &MOS6502::LDAabx, &MOS6502::LDXaby, &MOS6502::OPCill, 
        /*C-*/ &MOS6502::CPYimm, &MOS6502::CMPxin, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::CPYzpg, &MOS6502::CMPzpg, &MOS6502::DECzpg, &MOS6502::OPCill, &MOS6502::INYimp, &MOS6502::CMPimm, &MOS6502::DEXimp, &MOS6502::OPCill, &MOS6502::CPYabs, &MOS6502::CMPabs, &MOS6502::DECabs, &MOS6502::OPCill, 
        /*D-*/ &MOS6502::BNErel, &MOS6502::CMPiny, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::CMPzpx, &MOS6502::DECzpx, &MOS6502::OPCill, &MOS6502::CLDimp, &MOS6502::CMPaby, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::CMPabx, &MOS6502::DECabx, &MOS6502::OPCill, 
        /*E-*/ &MOS6502::CPXimm, &MOS6502::SBCxin, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::CPXzpg, &MOS6502::SBCzpg, &MOS6502::INCzpg, &MOS6502::OPCill, &MOS6502::INXimp, &MOS6502::SBCimm, &MOS6502::NOPimp, &MOS6502::OPCill, &MOS6502::CPXabs, &MOS6502::SBCabs, &MOS6502::INCabs, &MOS6502::OPCill, 
        /*F-*/ &MOS6502::BEQrel, &MOS6502::SBCiny, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::SBCzpx, &MOS6502::INCzpx, &MOS6502::OPCill, &MOS6502::SEDimp, &MOS6502::SBCaby, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::OPCill, &MOS6502::SBCabx, &MOS6502::INCabx, &MOS6502::OPCill
    };

    /**** Utility ****/
    void callOpCode(uint8_t);
    void waitForCycles(uint8_t);

    /**** Stack Operations ****/
    void push(uint8_t);
    uint8_t pull();

    /**** Comparison ****/
    //Compare register with memory (Set SR flags)
    void compareRM(uint8_t reg, uint8_t memory);

    /**** Addition and Subtraction ****/
    //Add memory to AC with carry (and set SR flags)
    void addWithCarry(uint8_t memory);
    //Subtract memory to AC with borrow (and set SR flags)
    void subWithBorrow(uint8_t memory);
};

#endif
