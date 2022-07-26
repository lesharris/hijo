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
      uint32_t length = 0;
      uint32_t tcycles = 0;
      uint32_t mcycles = 0;
      std::vector<std::function<int8_t(SharpSM83 &)>> steps{};

      Opcode(uint8_t value,
             const std::string &label,
             uint32_t length,
             uint32_t tcycles,
             const std::vector<std::function<int8_t(SharpSM83 &)>> &steps)
          : value(value),
            label(label),
            length(length),
            tcycles(tcycles),
            mcycles(tcycles / 4),
            steps(steps) {}
    };

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
      return regs.f & bitmasks[4];
    }

    void SetHalfCarry(uint16_t sum) {
      if ((sum & 0x10) == 0x10) {
        SetHalfCarry();
      } else {
        ClearHalfCarry();
      }
    }

    uint8_t HalfCarry() {
      return regs.f & bitmasks[5];
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
      return regs.f & bitmasks[6];
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
      return regs.f & bitmasks[7];
    }

    void SetZero() {
      regs.f |= bitmasks[7];
    }

    void ClearZero() {
      regs.f &= ~bitmasks[7];
    }

    void InitOpcodes();

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

