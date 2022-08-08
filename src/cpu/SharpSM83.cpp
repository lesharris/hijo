#include "SharpSM83.h"
#include "Interrupts.h"
#include "system/Gameboy.h"
#include <spdlog/spdlog.h>
#include <regex>

namespace hijo {
  SharpSM83::SharpSM83() {
    Reset();
  }

  void SharpSM83::Reset() {
    regs.pc = 0x100;
    regs.sp = 0xFFFE;
    *((short *) &regs.a) = 0xB001;
    *((short *) &regs.b) = 0x1300;
    *((short *) &regs.d) = 0xD800;
    *((short *) &regs.h) = 0x4D01;
    m_IE = 0;
    m_IF = 0;
    m_InterruptMasterEnabled = false;
    m_EnablingIME = false;
  }

  uint16_t SharpSM83::Reg(const Register &t) {
    switch (t) {
      case Register::A:
        return regs.a;
      case Register::F:
        return regs.f;
      case Register::B:
        return regs.b;
      case Register::C:
        return regs.c;
      case Register::D:
        return regs.d;
      case Register::E:
        return regs.e;
      case Register::H:
        return regs.h;
      case Register::L:
        return regs.l;

      case Register::AF:
        return reverse(*((uint16_t *) &regs.a));
      case Register::BC:
        return reverse(*((uint16_t *) &regs.b));
      case Register::DE:
        return reverse(*((uint16_t *) &regs.d));
      case Register::HL:
        return reverse(*((uint16_t *) &regs.h));

      case Register::PC:
        return regs.pc;
      case Register::SP:
        return regs.sp;
      default:
        return 0;
    }
  }

  void SharpSM83::Reg(const Register &t, uint16_t value) {
    switch (t) {
      case Register::A:
        regs.a = value & 0xFF;
        break;
      case Register::F:
        regs.f = value & 0xFF;
        break;
      case Register::B:
        regs.b = value & 0xFF;
        break;
      case Register::C:
        regs.c = value & 0xFF;
        break;
      case Register::D:
        regs.d = value & 0xFF;
        break;
      case Register::E:
        regs.e = value & 0xFF;
        break;
      case Register::H:
        regs.h = value & 0xFF;
        break;
      case Register::L:
        regs.l = value & 0xFF;
        break;

      case Register::AF:
        *((uint16_t *) &regs.a) = reverse(value);
        break;
      case Register::BC:
        *((uint16_t *) &regs.b) = reverse(value);
        break;
      case Register::DE:
        *((uint16_t *) &regs.d) = reverse(value);
        break;
      case Register::HL: {
        *((uint16_t *) &regs.h) = reverse(value);
        break;
      }

      case Register::PC:
        regs.pc = value;
        break;
      case Register::SP:
        regs.sp = value;
        break;
      case Register::NONE:
        break;
    }
  }

  uint8_t SharpSM83::Reg8(const Register &t) {
    auto &bus = Gameboy::Get();
    switch (t) {
      case Register::A:
        return regs.a;
      case Register::F:
        return regs.f;
      case Register::B:
        return regs.b;
      case Register::C:
        return regs.c;
      case Register::D:
        return regs.d;
      case Register::E:
        return regs.e;
      case Register::H:
        return regs.h;
      case Register::L:
        return regs.l;
      case Register::HL: {
        return bus.cpuRead(Reg(Register::HL));
      }
      default:
        spdlog::get("console")->warn("Invalid Reg8 Read");
        return 0;
        break;
    }
  }

  void SharpSM83::Reg8(const Register &t, uint8_t value) {
    auto &bus = Gameboy::Get();

    switch (t) {
      case Register::A:
        regs.a = value & 0xFF;
        break;
      case Register::F:
        regs.f = value & 0xFF;
        break;
      case Register::B:
        regs.b = value & 0xFF;
        break;
      case Register::C:
        regs.c = value & 0xFF;
        break;
      case Register::D:
        regs.d = value & 0xFF;
        break;
      case Register::E:
        regs.e = value & 0xFF;
        break;
      case Register::H:
        regs.h = value & 0xFF;
        break;
      case Register::L:
        regs.l = value & 0xFF;
        break;
      case Register::HL:
        bus.cpuWrite(Reg(Register::HL), value);
        break;
      default:
        break;
    }
  }

  SharpSM83::InstructionProc SharpSM83::GetProcessor(Instruction type) {
    return m_Processors[type];
  }

  void SharpSM83::FetchData() {
    auto &bus = Gameboy::Get();

    m_MemoryDestination = 0;
    DestinationIsMemory = false;

    if (m_CurrentInstruction == NULL) {
      spdlog::get("console")->warn("FetchData:  Current Instruction is Null!");
      return;
    }

    switch (m_CurrentInstruction->mode) {
      case AddressMode::IMP:
        return;

      case AddressMode::R:
        m_FetchedData = Reg(m_CurrentInstruction->reg1);
        return;

      case AddressMode::R_R:
        m_FetchedData = Reg(m_CurrentInstruction->reg2);
        return;

      case AddressMode::R_D8:
        m_FetchedData = bus.cpuRead(regs.pc);
        Cycle(1);
        regs.pc++;
        return;

      case AddressMode::R_D16:
      case AddressMode::D16: {
        uint16_t lo = bus.cpuRead(regs.pc);
        Cycle(1);

        uint16_t hi = bus.cpuRead(regs.pc + 1);
        Cycle(1);

        m_FetchedData = lo | (hi << 8);

        regs.pc += 2;

        return;
      }

      case AddressMode::MR_R:
        m_FetchedData = Reg(m_CurrentInstruction->reg2);
        m_MemoryDestination = Reg(m_CurrentInstruction->reg1);
        DestinationIsMemory = true;

        if (m_CurrentInstruction->reg1 == Register::C) {
          m_MemoryDestination |= 0xFF00;
        }

        return;

      case AddressMode::R_MR: {
        uint16_t addr = Reg(m_CurrentInstruction->reg2);

        if (m_CurrentInstruction->reg2 == Register::C) {
          addr |= 0xFF00;
        }

        m_FetchedData = bus.cpuRead(addr);
        Cycle(1);

      }
        return;

      case AddressMode::R_HLI:
        m_FetchedData = bus.cpuRead(Reg(m_CurrentInstruction->reg2));
        Reg(Register::HL, Reg(Register::HL) + 1);
        Cycle(1);
        return;

      case AddressMode::R_HLD:
        m_FetchedData = bus.cpuRead(Reg(m_CurrentInstruction->reg2));
        Reg(Register::HL, Reg(Register::HL) - 1);
        Cycle(1);
        return;

      case AddressMode::HLI_R:
        m_FetchedData = Reg(m_CurrentInstruction->reg2);
        m_MemoryDestination = Reg(m_CurrentInstruction->reg1);
        DestinationIsMemory = true;
        Reg(Register::HL, Reg(Register::HL) + 1);
        return;

      case AddressMode::HLD_R:
        m_FetchedData = Reg(m_CurrentInstruction->reg2);
        m_MemoryDestination = Reg(m_CurrentInstruction->reg1);
        DestinationIsMemory = true;
        Reg(Register::HL, Reg(Register::HL) - 1);
        return;

      case AddressMode::R_A8:
        m_FetchedData = bus.cpuRead(regs.pc);
        regs.pc++;
        Cycle(1);
        return;

      case AddressMode::A8_R:
        m_MemoryDestination = bus.cpuRead(regs.pc) | 0xFF00;
        DestinationIsMemory = true;
        regs.pc++;
        Cycle(1);
        return;

      case AddressMode::HL_SPR:
        m_FetchedData = bus.cpuRead(regs.pc);
        regs.pc++;
        Cycle(1);
        return;

      case AddressMode::D8:
        m_FetchedData = bus.cpuRead(regs.pc);
        regs.pc++;
        Cycle(1);
        return;

      case AddressMode::A16_R:
      case AddressMode::D16_R: {
        uint16_t lo = bus.cpuRead(regs.pc);
        Cycle(1);

        uint16_t hi = bus.cpuRead(regs.pc + 1);
        Cycle(1);

        m_MemoryDestination = lo | (hi << 8);
        DestinationIsMemory = true;

        regs.pc += 2;
        m_FetchedData = Reg(m_CurrentInstruction->reg2);

      }
        return;

      case AddressMode::MR_D8:
        m_FetchedData = bus.cpuRead(regs.pc);
        m_MemoryDestination = Reg(m_CurrentInstruction->reg1);
        DestinationIsMemory = true;
        Cycle(1);
        regs.pc++;
        return;

      case AddressMode::MR:
        m_MemoryDestination = Reg(m_CurrentInstruction->reg1);
        DestinationIsMemory = true;
        m_FetchedData = bus.cpuRead(Reg(m_CurrentInstruction->reg1));
        Cycle(1);
        return;

      case AddressMode::R_A16: {
        uint16_t lo = bus.cpuRead(regs.pc);
        Cycle(1);

        uint16_t hi = bus.cpuRead(regs.pc + 1);
        Cycle(1);

        uint16_t addr = lo | (hi << 8);

        regs.pc += 2;
        m_FetchedData = bus.cpuRead(addr);
        Cycle(1);

        return;
      }

      default:
        printf("Unknown Addressing Mode! %d (%02X)\n", m_CurrentInstruction->mode, m_CurrentOpcode);
        exit(-7);
    }
  }

  void SharpSM83::FetchInstruction() {
    auto &bus = Gameboy::Get();

    m_CurrentOpcode = bus.cpuRead(regs.pc++);
    m_CurrentInstruction = &instrs.OpcodeByByte(m_CurrentOpcode);
    Cycle(1);
  }

  void SharpSM83::Execute() {
    auto proc = GetProcessor(m_CurrentInstruction->type);

    if (!proc) {
      spdlog::get("console")->warn("No processor for current instruction: {:02X}", m_CurrentInstruction->code);
      return;
    }

    (this->*proc)();
  }

  bool SharpSM83::Step() {
    m_CurrentCycles = 0;

    auto &bus = Gameboy::Get();

    if (!m_Halted) {
      FetchInstruction();
      m_CurrentCycles++;

      FetchData();

      if (m_CurrentInstruction == nullptr) {
        m_Halted = true;
        spdlog::get("console")->warn("No instructions for opcode {}", m_CurrentOpcode);
      }

      Execute();
    } else {
      Cycle(1);

      if (m_IF) {
        m_Halted = false;
      }
    }

    if (m_InterruptMasterEnabled) {
      Interrupts::HandleInterrupts(*this);
      m_EnablingIME = false;
    }

    if (m_EnablingIME) {
      m_InterruptMasterEnabled = true;
    }

    return true;
  }

  void SharpSM83::SetFlags(int8_t z, int8_t n, int8_t h, int8_t c) {
    if (z != -1) {
      BIT_SET(regs.f, 7, z);
    }

    if (n != -1) {
      BIT_SET(regs.f, 6, n);
    }

    if (h != -1) {
      BIT_SET(regs.f, 5, h);
    }

    if (c != -1) {
      BIT_SET(regs.f, 4, c);
    }
  }

  Register SharpSM83::DecodeRegister(uint8_t reg) {
    if (reg > 0x7) {
      return Register::NONE;
    }

    return m_RegisterTypes[reg];
  }

  bool SharpSM83::Is16Bit(const Register &t) {
    return static_cast<uint8_t>(t) > static_cast<uint8_t>(Register::AF);
  }

  bool SharpSM83::CheckCondition() {
    uint8_t z = CPU_FLAG_Z();
    uint8_t c = CPU_FLAG_C();

    switch (m_CurrentInstruction->cond) {
      case Condition::NONE:
        return true;
      case Condition::C:
        return c;
      case Condition::NC:
        return !c;
      case Condition::Z:
        return z;
      case Condition::NZ:
        return !z;
    }

    return false;
  }

  void SharpSM83::GotoAddress(uint16_t addr, bool pushPC) {
    auto &bus = Gameboy::Get();

    if (CheckCondition()) {
      if (pushPC) {
        Cycle(2);
        Stack::Push16(regs.pc);
      }

      /* if (addr >= 0xC000 && addr < 0xFE00) {
         Disassemble(0, 0xFFFF);
       }*/

      regs.pc = addr;
      Cycle(1);
    }
  }

  void SharpSM83::ProcNone() {

  }

  void SharpSM83::ProcNOP() {

  }

  void SharpSM83::ProcCB() {
    auto &bus = Gameboy::Get();

    uint8_t op = m_FetchedData;
    Register reg = DecodeRegister(op & 0b111);
    uint8_t bit = (op >> 3) & 0b111;
    uint8_t bit_op = (op >> 6) & 0b11;
    uint8_t reg_val = Reg8(reg);

    Cycle(1);

    if (reg == Register::HL) {
      Cycle(2);
    }

    switch (bit_op) {
      case 1:
        //BIT
        SetFlags(!(reg_val & (1 << bit)), 0, 1, -1);
        return;

      case 2:
        //RST
        reg_val &= ~(1 << bit);
        Reg8(reg, reg_val);
        return;

      case 3:
        //SET
        reg_val |= (1 << bit);
        Reg8(reg, reg_val);
        return;
    }

    bool flagC = CPU_FLAG_C();

    switch (bit) {
      case 0: {
        //RLC
        bool setC = false;
        uint8_t result = (reg_val << 1) & 0xFF;

        if ((reg_val & (1 << 7)) != 0) {
          result |= 1;
          setC = true;
        }

        Reg8(reg, result);
        SetFlags(result == 0, false, false, setC);
      }
        return;

      case 1: {
        //RRC
        uint8_t old = reg_val;
        reg_val >>= 1;
        reg_val |= (old << 7);

        Reg8(reg, reg_val);
        SetFlags(!reg_val, false, false, old & 1);
      }
        return;

      case 2: {
        //RL
        uint8_t old = reg_val;
        reg_val <<= 1;
        reg_val |= flagC;

        Reg8(reg, reg_val);
        SetFlags(!reg_val, false, false, !!(old & 0x80));
      }
        return;

      case 3: {
        //RR
        uint8_t old = reg_val;
        reg_val >>= 1;

        reg_val |= (flagC << 7);

        Reg8(reg, reg_val);
        SetFlags(!reg_val, false, false, old & 1);
      }
        return;

      case 4: {
        //SLA
        uint8_t old = reg_val;
        reg_val <<= 1;

        Reg8(reg, reg_val);
        SetFlags(!reg_val, false, false, !!(old & 0x80));
      }
        return;

      case 5: {
        //SRA
        uint8_t u = (int8_t) reg_val >> 1;
        Reg8(reg, u);
        SetFlags(!u, 0, 0, reg_val & 1);
      }
        return;

      case 6: {
        //SWAP
        reg_val = ((reg_val & 0xF0) >> 4) | ((reg_val & 0xF) << 4);
        Reg8(reg, reg_val);
        SetFlags(reg_val == 0, false, false, false);
      }
        return;

      case 7: {
        //SRL
        uint8_t u = reg_val >> 1;
        Reg8(reg, u);
        SetFlags(!u, 0, 0, reg_val & 1);
      }
        return;
    }
  }

  void SharpSM83::ProcRLCA() {
    uint8_t u = regs.a;
    bool c = (u >> 7) & 1;
    u = (u << 1) | c;
    regs.a = u;

    SetFlags(0, 0, 0, c);
  }

  void SharpSM83::ProcRRCA() {
    uint8_t b = regs.a & 1;
    regs.a >>= 1;
    regs.a |= (b << 7);

    SetFlags(0, 0, 0, b);
  }

  void SharpSM83::ProcRLA() {
    uint8_t u = regs.a;
    uint8_t cf = CPU_FLAG_C();
    uint8_t c = (u >> 7) & 1;

    regs.a = (u << 1) | cf;
    SetFlags(0, 0, 0, c);
  }

  void SharpSM83::ProcSTOP() {
    spdlog::get("console")->info("Stop Called");
  }

  void SharpSM83::ProcDAA() {
    uint8_t u = 0;
    int fc = 0;

    if (CPU_FLAG_H() || (!CPU_FLAG_N() && (regs.a & 0xF) > 9)) {
      u = 6;
    }

    if (CPU_FLAG_C() || (!CPU_FLAG_N() && regs.a > 0x99)) {
      u |= 0x60;
      fc = 1;
    }

    regs.a += CPU_FLAG_N() ? -u : u;

    SetFlags(regs.a == 0, -1, 0, fc);
  }

  void SharpSM83::ProcCPL() {
    regs.a = ~regs.a;
    SetFlags(-1, 1, 1, -1);
  }

  void SharpSM83::ProcSCF() {
    SetFlags(-1, 0, 0, 1);
  }

  void SharpSM83::ProcCCF() {
    SetFlags(-1, 0, 0, CPU_FLAG_C() ^ 1);
  }

  void SharpSM83::ProcHALT() {
    m_Halted = true;
  }

  void SharpSM83::ProcRRA() {
    uint8_t carry = CPU_FLAG_C();
    uint8_t new_c = regs.a & 1;

    regs.a >>= 1;
    regs.a |= (carry << 7);

    SetFlags(0, 0, 0, new_c);
  }

  void SharpSM83::ProcAND() {
    regs.a &= m_FetchedData & 0xFF;
    SetFlags(regs.a == 0, 0, 1, 0);
  }

  void SharpSM83::ProcXOR() {
    regs.a ^= m_FetchedData & 0xFF;
    SetFlags(regs.a == 0, 0, 0, 0);
  }

  void SharpSM83::ProcOR() {
    regs.a |= m_FetchedData & 0xFF;
    SetFlags(regs.a == 0, 0, 0, 0);
  }

  void SharpSM83::ProcCP() {
    int n = (int) regs.a - (int) m_FetchedData;

    SetFlags(n == 0, 1,
             ((int) regs.a & 0x0F) - ((int) m_FetchedData & 0x0F) < 0, n < 0);
  }

  void SharpSM83::ProcDI() {
    m_InterruptMasterEnabled = false;
  }

  void SharpSM83::ProcEI() {
    m_EnablingIME = true;
  }

  void SharpSM83::ProcLD() {
    auto &bus = Gameboy::Get();
    if (DestinationIsMemory) {
      if (Is16Bit(m_CurrentInstruction->reg2)) {
        Cycle(1);
        bus.cpuWrite16(m_MemoryDestination, m_FetchedData);
      } else {
        bus.cpuWrite(m_MemoryDestination, m_FetchedData);
      }

      Cycle(1);

      return;
    }

    if (m_CurrentInstruction->mode == AddressMode::HL_SPR) {
      uint8_t hflag = (Reg(m_CurrentInstruction->reg2) & 0xF) +
                      (m_FetchedData & 0xF) >= 0x10;

      uint8_t cflag = (Reg(m_CurrentInstruction->reg2) & 0xFF) +
                      (m_FetchedData & 0xFF) >= 0x100;

      SetFlags(0, 0, hflag, cflag);
      Reg(m_CurrentInstruction->reg1,
          Reg(m_CurrentInstruction->reg2) + (int8_t) m_FetchedData);

      return;
    }

    Reg(m_CurrentInstruction->reg1, m_FetchedData);
  }

  void SharpSM83::ProcLDH() {
    auto &bus = Gameboy::Get();
    if (m_CurrentInstruction->reg1 == Register::A) {
      Reg(m_CurrentInstruction->reg1, bus.cpuRead(0xFF00 | m_FetchedData));
    } else {
      bus.cpuWrite(m_MemoryDestination, regs.a);
    }

    Cycle(1);
  }

  void SharpSM83::ProcJP() {
    GotoAddress(m_FetchedData, false);
  }

  void SharpSM83::ProcJR() {
    int8_t rel = (int8_t) (m_FetchedData & 0xFF);
    uint16_t addr = regs.pc + rel;
    GotoAddress(addr, false);
  }

  void SharpSM83::ProcCALL() {
    GotoAddress(m_FetchedData, true);
  }

  void SharpSM83::ProcRST() {
    GotoAddress(m_CurrentInstruction->param, true);
  }

  void SharpSM83::ProcRET() {
    auto &bus = Gameboy::Get();
    if (m_CurrentInstruction->cond != Condition::NONE) {
      Cycle(1);
    }

    if (CheckCondition()) {
      uint16_t lo = Stack::Pop();
      Cycle(1);
      uint16_t hi = Stack::Pop();
      Cycle(1);

      uint16_t n = (hi << 8) | lo;
      regs.pc = n;

      Cycle(1);
    }
  }

  void SharpSM83::ProcRETI() {
    m_InterruptMasterEnabled = true;
    ProcRET();
  }

  void SharpSM83::ProcPOP() {
    auto &bus = Gameboy::Get();

    uint16_t lo = Stack::Pop();
    Cycle(1);
    uint16_t hi = Stack::Pop();
    Cycle(1);

    uint16_t n = (hi << 8) | lo;

    if (m_CurrentInstruction->reg1 == Register::AF) {
      Reg(m_CurrentInstruction->reg1, n & 0xFFF0);
    } else {
      Reg(m_CurrentInstruction->reg1, n);
    }
  }

  void SharpSM83::ProcPUSH() {
    auto &bus = Gameboy::Get();

    // Internal
    Cycle(1);

    uint16_t hi = (Reg(m_CurrentInstruction->reg1) >> 8) & 0xFF;
    Cycle(1);
    Stack::Push(hi);

    uint16_t lo = Reg(m_CurrentInstruction->reg1) & 0xFF;
    Cycle(1);
    Stack::Push(lo);
  }

  void SharpSM83::ProcINC() {
    auto &bus = Gameboy::Get();

    uint16_t val = Reg(m_CurrentInstruction->reg1) + 1;

    if (Is16Bit(m_CurrentInstruction->reg1)) {
      Cycle(1);
    }

    if (m_CurrentInstruction->reg1 == Register::HL && m_CurrentInstruction->mode == AddressMode::MR) {
      val = m_FetchedData;
      val++;
      val &= 0xFF;
      bus.cpuWrite(Reg(Register::HL), val);
      Cycle(1);
    } else {
      Reg(m_CurrentInstruction->reg1, val);
      val = Reg(m_CurrentInstruction->reg1);
    }

    if ((m_CurrentOpcode & 0x03) == 0x03) {
      return;
    }

    SetFlags(val == 0, 0, (val & 0x0F) == 0, -1);
  }

  void SharpSM83::ProcDEC() {
    auto &bus = Gameboy::Get();

    uint16_t val = Reg(m_CurrentInstruction->reg1) - 1;

    if (Is16Bit(m_CurrentInstruction->reg1)) {
      Cycle(1);
    }

    if (m_CurrentInstruction->reg1 == Register::HL && m_CurrentInstruction->mode == AddressMode::MR) {
      val = m_FetchedData;
      val--;
      val &= 0xFF;
      bus.cpuWrite(Reg(Register::HL), val);
      Cycle(1);
    } else {
      Reg(m_CurrentInstruction->reg1, val);
      val = Reg(m_CurrentInstruction->reg1);
    }

    if ((m_CurrentOpcode & 0x0B) == 0x0B) {
      return;
    }

    SetFlags(val == 0, 1, (val & 0x0F) == 0x0F, -1);
  }

  void SharpSM83::ProcSUB() {
    uint16_t val = Reg(m_CurrentInstruction->reg1) - m_FetchedData;

    int z = val == 0;
    int h = ((int) Reg(m_CurrentInstruction->reg1) & 0xF) - ((int) m_FetchedData & 0xF) < 0;
    int c = ((int) Reg(m_CurrentInstruction->reg1)) - ((int) m_FetchedData) < 0;

    Reg(m_CurrentInstruction->reg1, val);
    SetFlags(z, 1, h, c);
  }

  void SharpSM83::ProcSBC() {
    uint8_t val = m_FetchedData + CPU_FLAG_C();

    int z = Reg(m_CurrentInstruction->reg1) - val == 0;

    int h = ((int) Reg(m_CurrentInstruction->reg1) & 0xF)
            - ((int) m_FetchedData & 0xF) - ((int) CPU_FLAG_C()) < 0;
    int c = ((int) Reg(m_CurrentInstruction->reg1))
            - ((int) m_FetchedData) - ((int) CPU_FLAG_C()) < 0;

    Reg(m_CurrentInstruction->reg1, Reg(m_CurrentInstruction->reg1) - val);
    SetFlags(z, 1, h, c);
  }

  void SharpSM83::ProcADC() {
    uint16_t u = m_FetchedData;
    uint16_t a = regs.a;
    uint16_t c = CPU_FLAG_C();

    regs.a = (a + u + c) & 0xFF;

    SetFlags(regs.a == 0, 0,
             (a & 0xF) + (u & 0xF) + c > 0xF,
             a + u + c > 0xFF);
  }

  void SharpSM83::ProcADD() {
    uint32_t val = Reg(m_CurrentInstruction->reg1) + m_FetchedData;

    bool is_16bit = Is16Bit(m_CurrentInstruction->reg1);

    if (is_16bit) {
      Cycle(1);
    }

    if (m_CurrentInstruction->reg1 == Register::SP) {
      val = Reg(m_CurrentInstruction->reg1) + (int8_t) m_FetchedData;
    }

    int z = (val & 0xFF) == 0;
    int h = (Reg(m_CurrentInstruction->reg1) & 0xF) + (m_FetchedData & 0xF) >= 0x10;
    int c = (int) (Reg(m_CurrentInstruction->reg1) & 0xFF) + (int) (m_FetchedData & 0xFF) >= 0x100;

    if (is_16bit) {
      z = -1;
      h = (Reg(m_CurrentInstruction->reg1) & 0xFFF) + (m_FetchedData & 0xFFF) >= 0x1000;
      uint32_t n = ((uint32_t) Reg(m_CurrentInstruction->reg1)) + ((uint32_t) m_FetchedData);
      c = n >= 0x10000;
    }

    if (m_CurrentInstruction->reg1 == Register::SP) {
      z = 0;
      h = (Reg(m_CurrentInstruction->reg1) & 0xF) + (m_FetchedData & 0xF) >= 0x10;
      c = (int) (Reg(m_CurrentInstruction->reg1) & 0xFF) + (int) (m_FetchedData & 0xFF) >= 0x100;
    }

    Reg(m_CurrentInstruction->reg1, val & 0xFFFF);
    SetFlags(z, 0, h, c);
  }

  uint16_t SharpSM83::reverse(uint16_t n) {
    return ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
  }

  void SharpSM83::Disassemble(uint16_t start_addr, uint16_t end_addr) {
    m_Disassembly.clear();
    m_Disassembly.reserve(0xFFFF);

    auto &bus = Gameboy::Get();

    uint16_t index = 0;
    while (start_addr < end_addr) {
      auto byte = bus.cpuRead(start_addr);
      auto &op = instrs.OpcodeByByte(byte);

      std::string bytes = fmt::format("{:02X}", op.code);
      std::string code = op.name;
      std::string mode = instrs.AddressModeLabel(op.mode);

      std::regex u8{"u8"};
      std::regex i8{"i8"};
      std::regex u16{"u16"};

      uint8_t low = 0;
      uint8_t high = 0;

      switch (op.length) {
        case 2:
          low = bus.cpuRead(start_addr + 1);
          bytes += fmt::format(" {:02X}", low);
          break;

        case 3:
          low = bus.cpuRead(start_addr + 1);
          high = bus.cpuRead(start_addr + 2);
          bytes += fmt::format(" {:02X} {:02X}", low, high);
          break;

        default:
          break;
      }

      std::smatch m;

      if (std::regex_search(code, m, u8)) {
        std::string lowFormatted = fmt::format("#{:02X}", low);
        code.replace(m[0].first, m[0].second, lowFormatted);
      } else if (std::regex_search(code, m, i8)) {
        uint32_t addr = (start_addr + op.length) + (int8_t) low;
        std::string addrString = fmt::format("${:04X} ; [{}]", addr, (int8_t) low);
        code.replace(m[0].first, m[0].second, addrString);
      } else if (std::regex_search(code, m, u16)) {
        std::string addr = fmt::format("${:02X}{:02X}", high, low);
        code.replace(m[0].first, m[0].second, addr);
      }

      m_Disassembly.push_back({
                                  start_addr,
                                  index++,
                                  code,
                                  bytes,
                                  mode
                              });

      start_addr += op.length;
    }
  }

  void SharpSM83::Cycle(uint8_t cycles) {
    auto &bus = Gameboy::Get();
    m_CurrentCycles += cycles;
    bus.Cycles(cycles);
  }

} // hijo