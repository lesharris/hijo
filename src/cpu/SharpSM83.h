#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <vector>

#include "common/common.h"
#include "system/System.h"

namespace hijo {

  class SharpSM83 {
  public:
    SharpSM83();

    void ConnectBus(System *system);

    void Reset();

    /////////////////
    //     Types ////
    /////////////////

  private:
    uint8_t bitmasks[8] = {
        0b00000001,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b10000000
    };

    enum class AddressingMode {
      Implied,
      Immediate,
      ExtendedImmediate,
      Register,
      RegisterIndirect,
      Extended,
      ModifiedPageZero,
      Relative,
      Bit
    };

    enum class Register {
      NONE,
      A,
      B,
      C,
      D,
      E,
      F,
      H,
      L,
      AF,
      BC,
      DE,
      HL,
      SP,
      PC
    };

    struct Registers {
      uint16_t pc;
      uint16_t sp;

      union {
        struct {
          uint8_t f;
          uint8_t a;
        };
        uint16_t af;
      };

      union {
        struct {
          uint8_t c;
          uint8_t b;
        };
        uint16_t bc;
      };

      union {
        struct {
          uint8_t e;
          uint8_t d;
        };
        uint16_t de;
      };

      union {
        struct {
          uint8_t l;
          uint8_t h;
        };
        uint16_t hl;
      };
    };

    struct Opcode {
      uint8_t value = 0;
      std::string label = "";
      AddressingMode mode;
      uint32_t length = 0;
      uint32_t tcycles = 0;
      uint32_t mcycles = 0;
      std::function<int8_t()> exec{};

      Opcode(uint8_t value,
             const std::string &label,
             const AddressingMode &mode,
             uint32_t length,
             uint32_t tcycles,
             const std::function<int8_t()> &exec)
          : value(value),
            label(label),
            mode(mode),
            length(length),
            tcycles(tcycles),
            mcycles(tcycles / 4),
            exec(exec) {}
    };

    /////////////////////////
    //    Flag Handling ////
    ///////////////////////

  private:
    void SetCarry(uint16_t sum) {
      if ((sum >> 8) != 0) {
        SetCarry();
      } else {
        ClearCarry();
      }
    }

    void SetCarry() {
      regs.f |= bitmasks[4];
    }

    void ClearCarry() {
      regs.f &= ~bitmasks[4];
    }

    uint8_t Carry() {
      return (regs.f & bitmasks[4]) >> 4;
    }

    void SetHalfCarry(uint16_t sum) {
      if ((sum & 0x10) == 0x10) {
        SetHalfCarry();
      } else {
        ClearHalfCarry();
      }
    }

    uint8_t HalfCarry() {
      return (regs.f & bitmasks[5]) >> 5;
    }

    void SetHalfCarry() {
      regs.f |= bitmasks[5];
    }

    void ClearHalfCarry() {
      regs.f &= ~bitmasks[5];
    }

    void SetNegative(uint16_t sum) {
      if (((sum & 0xFF) & 0x80) != 0) {
        SetNegative();
      } else {
        ClearNegative();
      }
    }

    uint8_t Negative() {
      return (regs.f & bitmasks[6]) >> 6;
    }

    void SetNegative() {
      regs.f |= bitmasks[6];
    }

    void ClearNegative() {
      regs.f &= ~bitmasks[6];
    }

    void SetZero(uint16_t value) {
      if (value == 0) {
        SetZero();
      } else {
        ClearZero();
      }
    }

    uint8_t Zero() {
      return (regs.f & bitmasks[7]) >> 7;
    }

    void SetZero() {
      regs.f |= bitmasks[7];
    }

    void ClearZero() {
      regs.f &= ~bitmasks[7];
    }

    ////////////////////////////
    //    Addressing Modes ////
    //////////////////////////
  private:
    void ImpliedMode(const Register & = Register::NONE) {}

    void RegisterMode(const Register & = Register::NONE) {}

    void ImmediateMode(const Register & = Register::NONE) {
      latch = bus->cpuRead(regs.pc + 1) & 0xFF;
    }

    void ExtendedImmediateMode(const Register & = Register::NONE) {
      uint8_t low = bus->cpuRead(regs.pc + 1);
      uint8_t high = bus->cpuRead(regs.pc + 2);

      latch = (uint16_t) ((high << 8) | low);
    }

    void RegisterIndirectMode(const Register &r) {
      uint16_t rvalue = 0;

      switch (r) {
        case Register::AF:
          rvalue = regs.af;
          break;

        case Register::BC:
          rvalue = regs.bc;
          break;

        case Register::DE:
          rvalue = regs.de;
          break;

        case Register::HL:
          rvalue = regs.hl;
          break;

        default:
          // Todo: Handle invalid
          break;
      }

      uint8_t low = rvalue & 0xFF;
      uint8_t high = (rvalue & 0xFF00) >> 8;

      latch = (uint16_t) ((high << 8) | low);
    }

    void ExtendedMode(const Register & = Register::NONE) {
      ExtendedImmediateMode();
    }

    void ModifiedPageZeroMode(const Register & = Register::NONE) {
      uint8_t p = bus->cpuRead(regs.pc + 1);

      switch (p) {
        case 0:
        case 0x8:
        case 0x10:
        case 0x18:
        case 0x20:
        case 0x28:
        case 0x30:
        case 0x38:
          latch = p & 0xFF;
          break;

        default:
          // Todo Handle invalid P here.
          latch = 0xFF;
          break;
      }
    }

    void RelativeMode(const Register & = Register::NONE) {
      ImmediateMode();
    }

    void BitMode(const Register & = Register::NONE) {}

    ////////////////////////////////////
    //              Opcode Handlers ////
    ////////////////////////////////////
  private:
    void LD(Register r, uint8_t data);

    void LD(Register r, uint16_t data);

    void LD(uint16_t address, uint8_t data);

    void LD(uint16_t address, uint16_t data);

    void LD(Register target, Register operand);

    void INC(Register r);

    void INC(uint16_t address);

    void DEC(Register r);

    void DEC(uint16_t address);

    void ADD(Register target, Register operand, bool isWide = false);

    void ADD(uint8_t data);

    void ADC(Register operand);

    void ADC(uint8_t data);

    void SUB(Register operand);

    void SUB(uint8_t data);

    void SBC(Register operand);

    void SBC(uint8_t data);

    void AND(Register operand);

    void AND(uint8_t data);

    void XOR(Register operand);

    void XOR(uint8_t data);

    void OR(Register operand);

    void OR(uint8_t data);

    void CP(Register operand);

    void CP(uint8_t data);

    void JR(int8_t offset);

    /////////////////////////////////////
    //    Addressing Mode Utilities ////
    ///////////////////////////////////

  private:
    typedef void (SharpSM83::*AddressingModeFunc)(const Register &);

    AddressingModeFunc modes[9] = {
        &SharpSM83::ImpliedMode,
        &SharpSM83::ImmediateMode,
        &SharpSM83::ExtendedImmediateMode,
        &SharpSM83::RegisterMode,
        &SharpSM83::RegisterIndirectMode,
        &SharpSM83::ExtendedMode,
        &SharpSM83::ModifiedPageZeroMode,
        &SharpSM83::RelativeMode,
        &SharpSM83::BitMode
    };

    void AddressingModeExec(AddressingMode modeType,
                            const Register &r = Register::NONE) {
      // lol
      (this->*modes[static_cast<size_t>(modeType)])(r);
    }

    /////////////////////////
    //    Misc Internal ////
    ///////////////////////

  private:
    void InitOpcodes();

    uint16_t *WideRegToPointer(Register r) {
      switch (r) {
        case Register::AF:
          return &regs.af;
        case Register::BC:
          return &regs.bc;
        case Register::DE:
          return &regs.de;
        case Register::HL:
          return &regs.hl;
        case Register::SP:
          return &regs.sp;
        case Register::PC:
          return &regs.pc;
        default:
          return nullptr;
      }
    }

    uint8_t *RegToPointer(Register r) {
      switch (r) {
        case Register::A:
          return &regs.a;
        case Register::B:
          return &regs.b;
        case Register::C:
          return &regs.c;
        case Register::D:
          return &regs.d;
        case Register::E:
          return &regs.e;
        case Register::F:
          return &regs.f;
        case Register::H:
          return &regs.h;
        case Register::L:
          return &regs.l;

        default:
          return nullptr;
      }
    }

    ////////////////
    //    State ////
    ////////////////

  private:
    Registers regs;
    uint8_t m_Cycle = 1;

    uint8_t m_CurrentOpcode;
    uint16_t latch;

    std::vector<Opcode> m_Opcodes{};

    System *bus = nullptr;

    bool m_Stopped = false;
    bool m_InterruptsEnabled = true;
    bool m_Halted = false;
  };

} // hijo

