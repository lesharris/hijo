#pragma once

#include <cstdint>
#include "common/common.h"
#include "Instructions.h"

namespace hijo {

  class SharpSM83 {
  public:
    struct Registers {
      uint8_t a;
      uint8_t f;
      uint8_t b;
      uint8_t c;
      uint8_t d;
      uint8_t e;
      uint8_t h;
      uint8_t l;
      uint16_t pc;
      uint16_t sp;
    };

  private:
    using IN_PROC = void (*)(SharpSM83 &);

    IN_PROC GetProcessor(Instructions::InstructionType type);

    Registers regs;

#define CPU_FLAG_Z BIT(ctx->regs.f, 7)
#define CPU_FLAG_N BIT(ctx->regs.f, 6)
#define CPU_FLAG_H BIT(ctx->regs.f, 5)
#define CPU_FLAG_C BIT(ctx->regs.f, 4)

    uint16_t fetched_data;
    uint16_t mem_dest;
    bool dest_is_mem;
    uint8_t cur_opcode;
    Instructions::Instruction *cur_inst;

    bool halted;
    bool stepping;

    bool int_master_enabled;
    bool enabling_ime;
    uint8_t ie_register;
    uint8_t int_flags;
  };

} // hijo

