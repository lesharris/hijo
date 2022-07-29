#include "SharpSM83.h"

#include <fmt/format.h>

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
  void SharpSM83::Cycle(uint32_t cycles) {
    if (m_Halted || m_Stopped)
      return;

    m_Cycles = cycles + m_CycleDelta;
    m_CurrentCycleCount = 0;

    while (m_CurrentCycleCount < m_Cycles) {
      m_CurrentCycleCount += Step();
    }

    m_CycleDelta = m_Cycles - m_CurrentCycleCount;
  }

  // Does one Fetch/Decode/Execute and returns the tcycles taken.
  uint8_t SharpSM83::Step() {
    // Fetch
    uint8_t nextByte = bus->cpuRead(regs.pc);

    // Decode
    m_CurrentOpcode = &m_Opcodes[nextByte];

    auto cycles = m_CurrentOpcode->tcycles;

    // Potentially populate the latch
    AddressingModeExec(m_CurrentOpcode->mode);

    // Execute
    auto offset = m_CurrentOpcode->exec();
    cycles += offset;

    return static_cast<uint8_t>(cycles);
  }

  /*****************************************
   * Misc Utility
   *****************************************/
  void SharpSM83::ConnectBus(System *system) {
    bus = system;
  }

  void SharpSM83::Reset() {
    regs.sp = 0xfffe;
    regs.af = 0x01b0;
    regs.bc = 0x0013;
    regs.de = 0x00d8;
    regs.hl = 0x014d;
    regs.pc = 0;
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
        regs.f = (data & 0xFF00) >> 8;
        regs.a = data & 0xFF;
        break;
      case Register::BC:
        regs.c = (data & 0xFF00) >> 8;
        regs.b = data & 0xFF;
        break;
      case Register::DE:
        regs.e = (data & 0xFF00) >> 8;
        regs.d = data & 0xFF;
        break;
      case Register::HL:
        regs.l = (data & 0xFF00) >> 8;
        regs.h = data & 0xFF;
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
    uint8_t low = (data & 0xFF00) >> 8;
    uint8_t high = data & 0xFF;

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

      SetZero(res);
      ClearNegative();
      SetHalfCarry(res);

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
    auto value = bus->cpuRead(address);
    bus->cpuWrite(address, value + 1);
  }

  void SharpSM83::DEC(SharpSM83::Register r) {
    auto decReg = [this](uint8_t *r) {
      uint16_t res = *r - 1;

      SetZero(res);
      SetNegative();
      SetHalfCarry(res);

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
    auto value = bus->cpuRead(address);
    bus->cpuWrite(address, value - 1);
  }

  void SharpSM83::ADD(SharpSM83::Register target, SharpSM83::Register operand, bool isWide) {
    if (isWide) {
      auto targetPtr = WideRegToPointer(target);
      auto opPtr = WideRegToPointer(operand);

      if (!targetPtr || !opPtr) {
        // Todo Handle this
        return;
      }

      uint32_t res = *targetPtr + *opPtr;

      ClearNegative();
      SetHalfCarry(res);
      SetCarry(res);

      *targetPtr = static_cast<uint16_t>(res & 0xFFFF);
    } else {
      auto targetPtr = RegToPointer(target);
      auto opPtr = RegToPointer(operand);

      if (targetPtr != nullptr || opPtr != nullptr) {
        // Todo Handle this
        return;
      }

      uint16_t res = *targetPtr + *opPtr;

      ClearNegative();
      SetZero(res);
      SetHalfCarry(res);
      SetCarry(res);

      *targetPtr = static_cast<uint8_t>(res & 0xFF);
    }
  }

  void SharpSM83::ADD(SharpSM83::Register target, uint8_t data) {
    auto opPtr = RegToPointer(target);

    if (!opPtr) {
      // todo handle this
      return;
    }

    uint32_t res = *opPtr + data;

    ClearZero();
    ClearNegative();
    SetHalfCarry(res);
    SetCarry(res);

    *opPtr = (uint16_t) res;
  }

  void SharpSM83::ADD(uint8_t data) {
    uint16_t res = regs.a + data;

    SetZero(res);
    ClearNegative();
    SetHalfCarry(res);
    SetCarry(res);

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
    int32_t diff = regs.a + data + Carry();

    ClearNegative();
    SetHalfCarry(diff);
    SetZero(diff);
    SetCarry(diff);

    regs.a = static_cast<uint8_t>(diff);
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

    SetNegative();
    SetHalfCarry(diff);
    SetZero(diff);
    SetCarry(diff);

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
    int32_t diff = regs.a - data - Carry();

    SetNegative();
    SetHalfCarry(diff);
    SetZero(diff);
    SetCarry(diff);

    regs.a = static_cast<uint8_t>(diff);
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
    int32_t diff = regs.a - data;
    SetNegative();

    if (!(diff & 0xFF)) {
      SetZero();
    } else {
      ClearZero();
    }

    if ((regs.a & 0xF) - (data & 0xF) < 0) {
      SetHalfCarry();
    } else {
      ClearHalfCarry();
    }

    if (diff < 0)
      SetCarry();
    else
      ClearCarry();
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

    auto low = bus->cpuRead(regs.sp++);
    auto high = bus->cpuRead(regs.sp++);

    *targetPtr = (low << 8) | high;
  }

  void SharpSM83::PUSH(SharpSM83::Register operand) {
    auto targetPtr = WideRegToPointer(operand);

    if (!targetPtr) {
      // todo handle
      return;
    }

    uint8_t low = (*targetPtr & 0xFF00) >> 8;
    uint8_t high = *targetPtr & 0xFF;

    bus->cpuWrite(--regs.sp, high);
    bus->cpuWrite(--regs.sp, low);
  }

  // Flow control
  void SharpSM83::JR(int8_t offset) {
    int16_t res = (int16_t) regs.pc +
                  (int8_t) (offset & 0xFF);

    regs.pc = (uint16_t) res;
  }

  void SharpSM83::JP(uint16_t address) {
    regs.pc = address;
  }

  void SharpSM83::CALL(uint16_t address) {
    uint8_t low = (regs.pc & 0xFF00) >> 8;
    uint8_t high = regs.pc & 0xFF;

    bus->cpuWrite(--regs.sp, high);
    bus->cpuWrite(--regs.sp, low);

    regs.pc = address;
  }

  void SharpSM83::RET() {
    regs.pc = regs.sp;
    regs.sp += 2;
  }

  /* Rotates, Swaps, Shifts */
  void SharpSM83::RST(uint16_t operand) {
    uint8_t low = (regs.pc & 0xFF00) >> 8;
    uint8_t high = regs.pc & 0xFF;

    bus->cpuWrite(--regs.sp, high);
    bus->cpuWrite(--regs.sp, low);

    regs.pc = operand;
  }

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
    uint8_t res = (data << 1) | (data >> 7);

    ClearNegative();
    ClearHalfCarry();
    SetZero(res);
    if (res & 1) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return res;
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
    uint8_t low = data & 1;
    uint8_t res = (data >> 1) | (data << 7);

    ClearNegative();
    ClearHalfCarry();
    SetZero(res);

    if (low) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return res;
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
    uint32_t wide = (data << 1) | Carry();
    uint8_t carry = static_cast<uint8_t>(wide >> 8);

    ClearNegative();
    ClearHalfCarry();
    SetZero(static_cast<uint8_t>(wide));

    if (carry) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return static_cast<uint8_t>(wide);
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
    uint8_t low = data & 1;
    uint8_t res = (data >> 1) | (Carry() << 7);

    ClearNegative();
    ClearHalfCarry();
    SetZero(res);

    if (low) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return res;
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
    ClearNegative();
    ClearHalfCarry();
    SetZero(data << 1);

    if (data >> 7) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return data << 1;
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
    ClearNegative();
    ClearHalfCarry();
    SetZero(((int8_t) data) >> 1);

    if (data & 1) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return ((int8_t) data) >> 1;
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
    SetZero((data << 4) | (data >> 4));
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
    ClearNegative();
    ClearHalfCarry();
    SetZero(data >> 1);
    if (data & 1) {
      SetCarry();
    } else {
      ClearCarry();
    }

    return data >> 1;
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
    auto mask = bitmasks[bit];

    ClearNegative();
    SetHalfCarry();

    if ((data & mask) == 0)
      SetZero();
    else
      ClearZero();

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
    auto mask = bitmasks[bit];

    return data & ~mask;
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
    auto mask = bitmasks[bit];
    return data | mask;
  }

  void SharpSM83::Disassemble(uint16_t start_addr, uint16_t end_addr) {
    m_Disassembly.clear();

    while (start_addr < end_addr) {
      auto byte = bus->cpuRead(start_addr);
      auto &op = m_Opcodes[byte];

      m_Disassembly.push_back({
                                  start_addr,
                                  op.label,
                                  AddressingModeLabel[op.mode]
                              });

      start_addr += op.length;
    }
  }

} // hijo