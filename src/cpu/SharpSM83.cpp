#include "SharpSM83.h"

#include <regex>
#include <fmt/format.h>

#include "Interrupts.h"
#include "Stack.h"

namespace hijo {
  SharpSM83::SharpSM83() {
    Reset();
    InitCBOps();
    InitOpcodes();
  }

  /*****************************************
   * Make our CPU work!
   *****************************************/

  // Execute a fixed number of tcycles
  void SharpSM83::Cycle(uint32_t tcycles) {
    if (!m_Halted) {

      uint32_t cycles = 0;

      do {
        uint8_t nextByte = bus->cpuRead(regs.pc);
        auto opcode = &m_Opcodes[nextByte];
        auto opCycles = opcode->tcycles;

        do {
          if (++m_CycleCount >= opCycles) {
            AddressingModeExec(opcode->mode);
            auto offset = opcode->exec();

            if (offset != 0)
              m_CycleCount += offset;
            else
              m_CycleCount = 0;
          }
        } while (m_CycleCount != opCycles && m_CycleCount != 0 && ++cycles < tcycles);

      } while (cycles < tcycles);
    } else {
      if (m_IntFlags) {
        m_Halted = false;
      }
    }

    if (m_InterruptsEnabled) {
      Interrupts::HandleInterrupts(*this);
      m_EnablingInterrupts = false;
    }

    if (m_EnablingInterrupts) {
      m_InterruptsEnabled = true;
    }
  }

  // Does one Fetch/Decode/Execute and returns the tcycles taken.
  uint8_t SharpSM83::Step() {
    // Fetch
    uint8_t nextByte = bus->cpuRead(regs.pc);

    m_CycleCount = 0;

    // Decode
    m_CurrentOpcode = &m_Opcodes[nextByte];

    auto cycles = m_CurrentOpcode->tcycles;

    // Potentially populate the latch
    AddressingModeExec(m_CurrentOpcode->mode);

    // Execute
    auto offset = m_CurrentOpcode->exec();
    cycles += offset;

    if (m_InterruptsEnabled) {
      Interrupts::HandleInterrupts(*this);
      m_EnablingInterrupts = false;
    }

    if (m_EnablingInterrupts) {
      m_InterruptsEnabled = true;
    }

    return static_cast<uint8_t>(cycles);
  }

  /*****************************************
   * Misc Utility
   *****************************************/
  void SharpSM83::ConnectBus(System *system) {
    bus = system;
  }

  void SharpSM83::Reset() {
    regs.af = 0xB001;
    regs.bc = 0x1300;
    regs.de = 0xD800;
    regs.hl = 0x4D01;
    regs.pc = 0x100;
    regs.sp = 0xFFFE;

    m_IE = 0;
    m_IntFlags = 0;
    m_InterruptsEnabled = false;
    m_EnablingInterrupts = false;
  }

  // Load the things
  void SharpSM83::LD(SharpSM83::Register r, uint16_t data) {
    switch (r) {
      case Register::A:
        regs.a = data & 0xFF;
        break;
      case Register::B:
        regs.b = data & 0xFF;
        break;
      case Register::C:
        regs.c = data & 0xFF;
        break;
      case Register::D:
        regs.d = data & 0xFF;
        break;
      case Register::E:
        regs.e = data & 0xFF;
        break;
      case Register::H:
        regs.h = data & 0xFF;
        break;
      case Register::L:
        regs.l = data & 0xFF;
        break;
      case Register::AF:
        regs.af = data;
        break;
      case Register::BC:
        regs.bc = data;
        break;
      case Register::DE:
        regs.de = data;
        break;
      case Register::HL:
        regs.hl = data;
        break;
      case Register::SP:
        regs.sp = data;
        break;
      case Register::PC:
        regs.pc = data;
        break;

      default:
        break;
    }
  }

  void SharpSM83::LD(SharpSM83::Register r, uint8_t data) {
    switch (r) {
      case Register::A:
        regs.a = data;
        break;
      case Register::B:
        regs.b = data;
        break;
      case Register::C:
        regs.c = data;
        break;
      case Register::D:
        regs.d = data;
        break;
      case Register::E:
        regs.e = data;
        break;
      case Register::H:
        regs.h = data;
        break;
      case Register::L:
        regs.l = data;
        break;
      case Register::AF:
        regs.af = (regs.f << 8) | data;
        break;
      case Register::BC:
        regs.bc = (regs.c << 8) | data;
        break;
      case Register::DE:
        regs.de = (regs.e << 8) | data;
        break;
      case Register::HL:
        regs.hl = (regs.l << 8) | data;
        break;
      case Register::SP:
        regs.sp = (regs.sp << 8) | data;
        break;
      case Register::PC:
        regs.pc = (regs.pc << 8) | data;
        break;

      default:
        break;
    }
  }

  void SharpSM83::LD(uint16_t address, uint8_t data) {
    bus->cpuWrite(address, data);
  }

  void SharpSM83::LD(uint16_t address, uint16_t data) {
    uint8_t high = (data >> 8) & 0xFF;
    uint8_t low = data & 0xFF;

    LD(address, low);
    LD(address + 1, high);
  }

  void SharpSM83::LD(SharpSM83::Register target, SharpSM83::Register operand) {
    auto targetPtr = RegToPointer(target);
    auto opPtr = RegToPointer(operand);

    if (!targetPtr || !opPtr) {
      // Todo Handle this
      return;
    }

    *targetPtr = *opPtr;
  }

  // ALU
  void SharpSM83::INC(SharpSM83::Register r) {
    auto incReg = [this](uint8_t *r) {
      uint16_t res = *r + 1;

      if ((res & 0xFF) == 0) {
        SetZero();
      } else {
        ClearZero();
      }

      ClearNegative();

      if ((res & 0x0F) == 0) {
        SetHalfCarry();
      } else {
        ClearHalfCarry();
      }

      *r = res & 0xFF;
    };

    switch (r) {
      case Register::A:
        incReg(&regs.a);
        break;
      case Register::B:
        incReg(&regs.b);
        break;
      case Register::C:
        incReg(&regs.c);
        break;
      case Register::D:
        incReg(&regs.d);
        break;
      case Register::E:
        incReg(&regs.e);
        break;
      case Register::H:
        incReg(&regs.h);
        break;
      case Register::L:
        incReg(&regs.l);
        break;
      case Register::AF:
        regs.af++;
        break;
      case Register::BC:
        regs.bc++;
        break;
      case Register::DE:
        regs.de++;
        break;
      case Register::HL:
        regs.hl++;
        break;
      case Register::SP:
        regs.sp++;
        break;
      case Register::PC:
        regs.pc++;
        break;

      default:
        break;
    }
  }

  void SharpSM83::INC(uint16_t address) {
    uint16_t res = bus->cpuRead(address) + 1;
    bus->cpuWrite(address, res & 0xFF);

    ClearNegative();

    if ((res & 0xFF) == 0) {
      SetZero();
    } else {
      ClearZero();
    }

    if ((res & 0x0F) == 0) {
      SetHalfCarry();
    } else {
      ClearHalfCarry();
    }

  }

  void SharpSM83::DEC(SharpSM83::Register r) {
    auto decReg = [this](uint8_t *r) {
      uint16_t res = *r - 1;

      if ((res & 0xFF) == 0) {
        SetZero();
      } else {
        ClearZero();
      }

      SetNegative();

      if ((res & 0x0F) == 0x0F) {
        SetHalfCarry();
      } else {
        ClearHalfCarry();
      }

      *r = res & 0xFF;
    };

    switch (r) {
      case Register::A:
        decReg(&regs.a);
        break;
      case Register::B:
        decReg(&regs.b);
        break;
      case Register::C:
        decReg(&regs.c);
        break;
      case Register::D:
        decReg(&regs.d);
        break;
      case Register::E:
        decReg(&regs.e);
        break;
      case Register::H:
        decReg(&regs.h);
        break;
      case Register::L:
        decReg(&regs.l);
        break;
      case Register::AF:
        regs.af--;
        break;
      case Register::BC:
        regs.bc--;
        break;
      case Register::DE:
        regs.de--;
        break;
      case Register::HL:
        regs.hl--;
        break;
      case Register::SP:
        regs.sp--;
        break;
      case Register::PC:
        regs.pc--;
        break;

      default:
        break;
    }
  }

  void SharpSM83::DEC(uint16_t address) {
    uint16_t res = bus->cpuRead(address) - 1;
    bus->cpuWrite(address, res);

    SetNegative();

    if (res == 0) {
      SetZero();
    } else {
      ClearZero();
    }

    if ((res & 0x0F) == 0x0F) {
      SetHalfCarry();
    } else {
      ClearHalfCarry();
    }
  }

  void SharpSM83::ADD(SharpSM83::Register target, SharpSM83::Register operand, bool isWide) {
    switch (target) {
      case Register::BC:
      case Register::DE:
      case Register::HL:
        isWide = true;
        break;

      default:
        break;
    }

    switch (operand) {
      case Register::BC:
      case Register::DE:
      case Register::HL:
        isWide = true;
        break;

      default:
        break;
    }

    if (isWide) {
      auto targetPtr = WideRegToPointer(target);
      auto opPtr = WideRegToPointer(operand);

      if (!targetPtr || !opPtr) {
        // Todo Handle this
        return;
      }

      uint32_t res = *targetPtr + *opPtr;

      int h = (*targetPtr & 0xFFF) + (*opPtr & 0xFFF) >= 0x1000;
      int n = ((int) *targetPtr) + ((int) *opPtr);
      int c = n >= 0x10000;

      ClearNegative();

      if (h) {
        SetHalfCarry();
      } else {
        ClearHalfCarry();
      }

      if (c) {
        SetCarry();
      } else {
        ClearCarry();
      }

      *targetPtr = static_cast<uint16_t>(res & 0xFFFF);

    } else {
      auto targetPtr = RegToPointer(target);
      auto opPtr = RegToPointer(operand);

      if (targetPtr != nullptr || opPtr != nullptr) {
        // Todo Handle this
        return;
      }

      uint32_t res = *targetPtr + *opPtr;

      int z = (res & 0xFF) == 0;
      int h = (*targetPtr & 0xF) + (*opPtr & 0xF) >= 0x10;
      int c = (int) (*targetPtr & 0xFF) + (int) (*opPtr & 0xFF) >= 0x100;

      if (z) {
        SetZero();
      } else {
        ClearZero();
      }

      ClearNegative();

      if (h) {
        SetHalfCarry();
      } else {
        ClearHalfCarry();
      }

      if (c) {
        SetCarry();
      } else {
        ClearCarry();
      }

      *targetPtr = static_cast<uint8_t>(res & 0xFF);
    }
  }

  void SharpSM83::ADD(SharpSM83::Register target, uint8_t data) {
    auto targetPtr = RegToPointer(target);

    if (targetPtr != nullptr) {
      // Todo Handle this
      return;
    }

    if (target == Register::SP) {
      uint32_t res = regs.sp + (int8_t) data;;

      int h = (regs.sp & 0xF) + (data & 0xF) >= 0x10;
      int c = (int) (regs.sp & 0xFF) + (int) (data & 0xFF) >= 0x100;

      ClearZero();
      ClearNegative();

      if (h) {
        SetHalfCarry();
      } else {
        ClearHalfCarry();
      }

      if (c) {
        SetCarry();
      } else {
        ClearCarry();
      }

      regs.sp = res & 0xFFFF;

      return;
    }

    uint16_t res = *targetPtr + data;

    int z = (res & 0xFF) == 0;
    int h = (*targetPtr & 0xF) + (data & 0xF) >= 0x10;
    int c = (int) (*targetPtr & 0xFF) + (int) (data & 0xFF) >= 0x100;

    if (z) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearNegative();

    if (h) {
      SetHalfCarry();
    } else {
      ClearHalfCarry();
    }

    if (c) {
      SetCarry();
    } else {
      ClearCarry();
    }

    *targetPtr = static_cast<uint8_t>(res & 0xFF);
  }

  void SharpSM83::ADD(uint8_t data) {
    uint16_t res = regs.a + data;

    int z = (res & 0xFF) == 0;
    int h = (regs.a & 0xF) + (data & 0xF) >= 0x10;
    int c = (int) (regs.a & 0xFF) + (int) (data & 0xFF) >= 0x100;

    if (z) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearNegative();

    if (h) {
      SetHalfCarry();
    } else {
      ClearHalfCarry();
    }

    if (c) {
      SetCarry();
    } else {
      ClearCarry();
    }

    regs.a = static_cast<uint8_t>(res & 0xFF);
  }

  void SharpSM83::ADC(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    ADC(*opPtr);
  }

  void SharpSM83::ADC(uint8_t data) {

    uint16_t u = data;
    uint16_t a = regs.a;
    uint16_t c = Carry();

    regs.a = (a + u + c) & 0xFF;

    if (regs.a == 0) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearNegative();

    if ((a & 0xF) + (u & 0xF) + c > 0xF) {
      SetHalfCarry();
    } else {
      ClearHalfCarry();
    }

    if (a + u + c > 0xFF) {
      SetCarry();
    } else {
      ClearCarry();
    }
  }

  void SharpSM83::SUB(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    SUB(*opPtr);
  }

  void SharpSM83::SUB(uint8_t data) {
    int32_t diff = regs.a - data;

    if (diff == 0) {
      SetZero();
    } else {
      ClearZero();
    }

    SetNegative();

    int h = ((int) (regs.a) & 0xF) - ((int) data & 0xF) < 0;
    int c = ((int) (regs.a)) - ((int) data) < 0;

    if (h) {
      SetHalfCarry();
    } else {
      ClearHalfCarry();
    }

    if (c) {
      SetCarry();
    } else {
      ClearCarry();
    }

    regs.a = static_cast<uint8_t>(diff);
  }

  void SharpSM83::SBC(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    SBC(*opPtr);
  }

  void SharpSM83::SBC(uint8_t data) {
    uint8_t val = data + Carry();

    int z = regs.a - val == 0;

    int h = ((int) regs.a & 0xF)
            - ((int) data & 0xF) - ((int) Carry()) < 0;
    int c = ((int) regs.a)
            - ((int) data) - ((int) Carry()) < 0;

    regs.a = static_cast<uint8_t>(regs.a - val);

    if (z) {
      SetZero();
    } else {
      ClearZero();
    }

    SetNegative();

    if (h) {
      SetHalfCarry();
    } else {
      ClearHalfCarry();
    }

    if (c) {
      SetCarry();
    } else {
      ClearCarry();
    }
  }

  // Boolean Ops
  void SharpSM83::AND(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    AND(*opPtr);
  }

  void SharpSM83::AND(uint8_t data) {
    regs.a &= data;

    ClearNegative();
    ClearCarry();
    SetHalfCarry();
    SetZero(regs.a);
  }

  void SharpSM83::XOR(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    XOR(*opPtr);
  }

  void SharpSM83::XOR(uint8_t data) {
    regs.a ^= data;

    ClearNegative();
    ClearHalfCarry();
    ClearCarry();
    SetZero(regs.a);
  }

  void SharpSM83::OR(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    OR(*opPtr);
  }

  void SharpSM83::OR(uint8_t data) {
    regs.a |= data;

    ClearNegative();
    ClearHalfCarry();
    ClearCarry();
    SetZero(regs.a);
  }

  // Comparisons
  void SharpSM83::CP(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    CP(*opPtr);
  }

  void SharpSM83::CP(uint8_t data) {
    int32_t n = regs.a - data;

    if (n == 0) {
      SetZero();
    } else {
      ClearZero();
    }

    SetNegative();

    int32_t lowNibReg = regs.a & 0x0F;
    int32_t lowNibData = data & 0x0F;

    int32_t diff = lowNibReg - lowNibData;

    if (diff < 0) {
      SetHalfCarry();
    } else {
      ClearHalfCarry();
    }

    if (n < 0) {
      SetCarry();
    } else {
      ClearCarry();
    }

  }

  // Stack
  void SharpSM83::POP(SharpSM83::Register operand) {
    auto targetPtr = WideRegToPointer(operand);

    if (!targetPtr) {
      // todo handle
      return;
    }

    // BC -> C, B
    // C = low, B = high

    auto low = Stack::Pop();
    auto high = Stack::Pop();

    uint16_t n = (high << 8) | low;

    if (operand == Register::AF) {
      *targetPtr = n & 0xFFF0;
      // SetZero(regs.a);
      //  SetHalfCarry(regs.a);
      //  SetNegative(regs.a);
      //  SetCarry(regs.a);
    } else {
      *targetPtr = n;
    }
  }

  void SharpSM83::PUSH(SharpSM83::Register operand) {
    auto targetPtr = WideRegToPointer(operand);

    if (!targetPtr) {
      // todo handle
      return;
    }

    uint8_t high = ((*targetPtr & 0xFF00) >> 8) & 0xFF;
    uint8_t low = *targetPtr & 0xFF;

    uint16_t n = (high << 8) | low;
    Stack::Push16(n);
  }

  // Flow control
  void SharpSM83::JR(int8_t offset) {
    int8_t rel = (int8_t) (offset & 0xFF);

    uint16_t addr = regs.pc + rel;

    regs.pc = addr;
  }

  void SharpSM83::JP(uint16_t address) {
    regs.pc = address;
  }

  void SharpSM83::CALL(uint16_t address) {
    Stack::Push16(regs.pc);

    regs.pc = address;
  }

  void SharpSM83::RET() {
    uint8_t low = Stack::Pop();
    uint8_t high = Stack::Pop();

    uint16_t n = (high << 8) | low;
    regs.pc = n;
  }

  void SharpSM83::RST(uint16_t operand) {
    CALL(operand);
  }

  /* Rotates, Swaps, Shifts */
  void SharpSM83::RLC(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = RLC(*opPtr);
  }

  void SharpSM83::RLC(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, RLC(data));
  }

  uint8_t SharpSM83::RLC(uint8_t data) {
    bool setC = false;
    uint8_t result = (data << 1) & 0xFF;

    if ((data & (1 << 7)) != 0) {
      result |= 1;
      setC = true;
    }

    if (result == 0) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearNegative();
    ClearHalfCarry();

    if (setC) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return result;
  }

  void SharpSM83::RRC(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = RRC(*opPtr);
  }

  void SharpSM83::RRC(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, RRC(data));
  }

  uint8_t SharpSM83::RRC(uint8_t data) {
    uint8_t old = data;
    data >>= 1;
    data |= (old << 7);

    if (!data) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearNegative();
    ClearHalfCarry();

    if (old & 1) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return data;
  }

  void SharpSM83::RL(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = RL(*opPtr);
  }

  void SharpSM83::RL(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, RL(data));
  }

  uint8_t SharpSM83::RL(uint8_t data) {
    uint8_t old = data;
    data <<= 1;
    data |= Carry();

    if (!data) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearNegative();
    ClearHalfCarry();

    if (!!(old & 0x80)) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return data;
  }

  void SharpSM83::RR(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = RR(*opPtr);
  }

  void SharpSM83::RR(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, RR(data));
  }

  uint8_t SharpSM83::RR(uint8_t data) {
    uint8_t old = data;
    data >>= 1;

    data |= (Carry() << 7);

    if (!data) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearNegative();
    ClearHalfCarry();

    if (old & 1) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return data;
  }

  void SharpSM83::SLA(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = SLA(*opPtr);
  }

  void SharpSM83::SLA(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, SLA(data));
  }

  uint8_t SharpSM83::SLA(uint8_t data) {
    uint8_t old = data;
    data <<= 1;

    if (!data) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearNegative();
    ClearHalfCarry();

    if (!!(old & 0x80)) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return data;
  }

  void SharpSM83::SRA(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = SRA(*opPtr);
  }

  void SharpSM83::SRA(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, SRA(data));
  }

  uint8_t SharpSM83::SRA(uint8_t data) {
    uint8_t u = (int8_t) data >> 1;

    if (!u) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearNegative();
    ClearHalfCarry();

    if (data & 1) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return u;
  }

  void SharpSM83::SWAP(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = SWAP(*opPtr);
  }

  void SharpSM83::SWAP(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, SWAP(data));
  }

  uint8_t SharpSM83::SWAP(uint8_t data) {
    ClearNegative();
    ClearHalfCarry();
    SetZero(data);
    ClearCarry();

    return (data << 4) | (data >> 4);
  }

  void SharpSM83::SRL(SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    *opPtr = SRL(*opPtr);
  }

  void SharpSM83::SRL(uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    bus->cpuWrite(address, SRL(data));
  }

  uint8_t SharpSM83::SRL(uint8_t data) {
    uint8_t u = data >> 1;

    if (!u) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearNegative();
    ClearHalfCarry();

    if (data & 1) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return u;
  }

  /* Bit Ops */
  void SharpSM83::BIT(uint8_t bit, SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    BIT(bit, *opPtr);
  }

  void SharpSM83::BIT(uint8_t bit, uint8_t data) {
    if (!(data & (1 << bit))) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearNegative();
    SetHalfCarry();
  }

  void SharpSM83::RES(uint8_t bit, SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    auto res = RES(bit, *opPtr);

    *opPtr = res;
  }

  void SharpSM83::RES(uint8_t bit, uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    auto res = RES(bit, data);
    bus->cpuWrite(address, res);
  }

  uint8_t SharpSM83::RES(uint8_t bit, uint8_t data) {
    return data & ~(1 << bit);
  }

  void SharpSM83::SET(uint8_t bit, SharpSM83::Register operand) {
    auto opPtr = RegToPointer(operand);

    if (!opPtr) {
      // todo handle this
      return;
    }

    auto res = SET(bit, *opPtr);

    *opPtr = res;
  }

  void SharpSM83::SET(uint8_t bit, uint16_t address) {
    uint8_t data = bus->cpuRead(address);
    auto res = SET(bit, data);
    bus->cpuWrite(address, res);
  }

  uint8_t SharpSM83::SET(uint8_t bit, uint8_t data) {
    return data | (1 << bit);
  }

  void SharpSM83::Disassemble(uint16_t start_addr, uint16_t end_addr) {
    m_Disassembly.clear();

    uint16_t index = 0;
    while (start_addr < end_addr) {
      auto byte = bus->cpuRead(start_addr);
      auto &op = m_Opcodes[byte];
      std::string bytes = fmt::format("{:02X} ", op.value);
      std::string code = op.label;

      switch (op.mode) {
        case AddressingMode::Implied:
        case AddressingMode::Bit:
        case AddressingMode::Register:
        case AddressingMode::RegisterIndirectAF:
        case AddressingMode::RegisterIndirectBC:
        case AddressingMode::RegisterIndirectDE:
        case AddressingMode::RegisterIndirectHL:
          break;

        case AddressingMode::Immediate: {
          uint8_t data = IMM(start_addr);
          std::string operand = fmt::format("#${:02X}", data);

          std::smatch m;
          std::regex re{"u8"};

          if (std::regex_search(code, m, re)) {
            code.replace(m[0].first, m[0].second, operand);
          }

          bytes += fmt::format("{:02X}", data);
        }
          break;
        case AddressingMode::ExtendedImmediate: {
          uint16_t data = EXTI(start_addr);
          uint8_t low = data & 0xFF;
          uint8_t high = (data & 0xFF00) >> 8;
          std::string addr = fmt::format("${:02X}{:02X}", high, low);

          std::smatch m;
          std::regex re{"u16"};

          if (std::regex_search(code, m, re)) {
            code.replace(m[0].first, m[0].second, addr);
          }

          bytes += fmt::format("{:02X} {:02X}", low, high);
        }
          break;
        case AddressingMode::Extended: {
          uint16_t data = EXTI(start_addr);
          uint8_t low = data & 0xFF;
          uint8_t high = (data & 0xFF00) >> 8;
          std::string addr = fmt::format("${:02X}{:02X}", high, low);

          std::smatch m;
          std::regex re{"u16"};

          if (std::regex_search(code, m, re)) {
            code.replace(m[0].first, m[0].second, addr);
          }

          bytes += fmt::format("{:02X} {:02X}", low, high);
        }
          break;
        case AddressingMode::ModifiedPageZero: {
          uint8_t data = MPZ(start_addr);
          bytes += fmt::format("{:02X}", data);
        }
          break;

        case AddressingMode::Relative: {
          int8_t data = REL(start_addr);
          uint32_t addr = start_addr + data;
          std::string addrString = fmt::format("${:04X} ; [{}]", addr, data);

          std::smatch m;
          std::regex re{"i8"};

          if (std::regex_search(code, m, re)) {
            code.replace(m[0].first, m[0].second, addrString);
          }

          bytes += fmt::format("{:02X}", static_cast<uint8_t>(data));
        }
          break;
      }

      m_Disassembly.push_back({
                                  start_addr,
                                  index++,
                                  code,
                                  bytes,
                                  AddressingModeLabel[op.mode]
                              });

      start_addr += op.length;
    }
  }

  void SharpSM83::DAA() {
    int u = 0;
    int fc = 0;

    if (HalfCarry() || (!Negative() && (regs.a & 0xF) > 9)) {
      u = 6;
    }

    if (Carry() || (!Negative() && regs.a > 0x99)) {
      u |= 0x60;
      fc = 1;
    }

    if (Negative()) {
      regs.a -= u;
    } else {
      regs.a += u;
    }

    if ((regs.a & 0xFF) == 0) {
      SetZero();
    } else {
      ClearZero();
    }

    ClearHalfCarry();

    if (fc == 1) {
      SetCarry();
    } else {
      ClearCarry();
    }
  }

  void SharpSM83::RLCA() {
    uint8_t u = regs.a;
    bool c = (u >> 7) & 1;
    u = (u << 1) | c;
    regs.a = u;

    ClearZero();
    ClearNegative();
    ClearHalfCarry();
    if (c) {
      SetCarry();
    } else {
      ClearCarry();
    }
  }

  void SharpSM83::RRCA() {
    uint8_t b = regs.a & 1;
    regs.a >>= 1;
    regs.a |= (b << 7);

    ClearZero();
    ClearNegative();
    ClearHalfCarry();
    if (b) {
      SetCarry();
    } else {
      ClearCarry();
    }
  }

  void SharpSM83::RLA() {
    uint8_t u = regs.a;
    uint8_t cf = Carry();
    uint8_t c = (u >> 7) & 1;

    regs.a = (u << 1) | cf;

    ClearZero();
    ClearNegative();
    ClearHalfCarry();
    if (c) {
      SetCarry();
    } else {
      ClearCarry();
    }
  }

  void SharpSM83::RRA() {
    uint8_t carry = Carry();
    uint8_t new_c = regs.a & 1;

    regs.a >>= 1;
    regs.a |= (carry << 7);

    ClearZero();
    ClearNegative();
    ClearHalfCarry();
    if (new_c) {
      SetCarry();
    } else {
      ClearCarry();
    }
  }

} // hijo