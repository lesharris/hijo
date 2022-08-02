#include "Instructions.h"

namespace hijo {
  Instructions::Instruction &Instructions::InstructionByOpcode(uint8_t opcode) {
    return m_Instructions[opcode];
  }

  const std::string &Instructions::InstructionName(const Instructions::Instruction &t) {
    return m_InstructionNames[t.type];
  }

  std::string Instructions::InstructionToString(const Instructions::Instruction &t) {
    return std::string();
  }

  /*void inst_to_str(cpu_context *ctx, char *str) {
    instruction *inst = ctx->cur_inst;
    sprintf(str, "%s ", inst_name(inst->type));

    switch (inst->mode) {
      case Instructions::AM_IMP:
        return;

      case Instructions::AM_R_D16:
      case Instructions::AM_R_A16:
        sprintf(str, "%s %s,$%04X", inst_name(inst->type),
                rt_lookup[inst->reg_1], ctx->fetched_data);
        return;

      case Instructions::AM_R:
        sprintf(str, "%s %s", inst_name(inst->type),
                rt_lookup[inst->reg_1]);
        return;

      case Instructions::AM_R_R:
        sprintf(str, "%s %s,%s", inst_name(inst->type),
                rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

      case Instructions::AM_MR_R:
        sprintf(str, "%s (%s),%s", inst_name(inst->type),
                rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

      case Instructions::AM_MR:
        sprintf(str, "%s (%s)", inst_name(inst->type),
                rt_lookup[inst->reg_1]);
        return;

      case Instructions::AM_R_MR:
        sprintf(str, "%s %s,(%s)", inst_name(inst->type),
                rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

      case Instructions::AM_R_D8:
      case Instructions::AM_R_A8:
        sprintf(str, "%s %s,$%02X", inst_name(inst->type),
                rt_lookup[inst->reg_1], ctx->fetched_data & 0xFF);
        return;

      case Instructions::AM_R_HLI:
        sprintf(str, "%s %s,(%s+)", inst_name(inst->type),
                rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

      case Instructions::AM_R_HLD:
        sprintf(str, "%s %s,(%s-)", inst_name(inst->type),
                rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

      case Instructions::AM_HLI_R:
        sprintf(str, "%s (%s+),%s", inst_name(inst->type),
                rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

      case Instructions::AM_HLD_R:
        sprintf(str, "%s (%s-),%s", inst_name(inst->type),
                rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

      case Instructions::AM_A8_R:
        sprintf(str, "%s $%02X,%s", inst_name(inst->type),
                bus_read(ctx->regs.pc - 1), rt_lookup[inst->reg_2]);

        return;

      case Instructions::AM_HL_SPR:
        sprintf(str, "%s (%s),SP+%d", inst_name(inst->type),
                rt_lookup[inst->reg_1], ctx->fetched_data & 0xFF);
        return;

      case Instructions::AM_D8:
        sprintf(str, "%s $%02X", inst_name(inst->type),
                ctx->fetched_data & 0xFF);
        return;

      case Instructions::AM_D16:
        sprintf(str, "%s $%04X", inst_name(inst->type),
                ctx->fetched_data);
        return;

      case Instructions::AM_MR_D8:
        sprintf(str, "%s (%s),$%02X", inst_name(inst->type),
                rt_lookup[inst->reg_1], ctx->fetched_data & 0xFF);
        return;

      case Instructions::AM_A16_R:
        sprintf(str, "%s ($%04X),%s", inst_name(inst->type),
                ctx->fetched_data, rt_lookup[inst->reg_2]);
        return;

      default:
        fprintf(stderr, "INVALID AM: %d\n", inst->mode);
        NO_IMPL
    }
  }*/

  std::vector<Instructions::Instruction> m_Instructions = {
      {Instructions::IN_NOP,  Instructions::AM_IMP},
      {Instructions::IN_LD,   Instructions::AM_R_D16,  Instructions::RT_BC},
      {Instructions::IN_LD,   Instructions::AM_MR_R,   Instructions::RT_BC,   Instructions::RT_A},
      {Instructions::IN_INC,  Instructions::AM_R,      Instructions::RT_BC},
      {Instructions::IN_INC,  Instructions::AM_R,      Instructions::RT_B},
      {Instructions::IN_DEC,  Instructions::AM_R,      Instructions::RT_B},
      {Instructions::IN_LD,   Instructions::AM_R_D8,   Instructions::RT_B},
      {Instructions::IN_RLCA},
      {Instructions::IN_LD,   Instructions::AM_A16_R,  Instructions::RT_NONE, Instructions::RT_SP},
      {Instructions::IN_ADD,  Instructions::AM_R_R,    Instructions::RT_HL,   Instructions::RT_BC},
      {Instructions::IN_LD,   Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_BC},
      {Instructions::IN_DEC,  Instructions::AM_R,      Instructions::RT_BC},
      {Instructions::IN_INC,  Instructions::AM_R,      Instructions::RT_C},
      {Instructions::IN_DEC,  Instructions::AM_R,      Instructions::RT_C},
      {Instructions::IN_LD,   Instructions::AM_R_D8,   Instructions::RT_C},
      {Instructions::IN_RRCA},

      //0x1X
      {Instructions::IN_STOP},
      {Instructions::IN_LD,   Instructions::AM_R_D16,  Instructions::RT_DE},
      {Instructions::IN_LD,   Instructions::AM_MR_R,   Instructions::RT_DE,   Instructions::RT_A},
      {Instructions::IN_INC,  Instructions::AM_R,      Instructions::RT_DE},
      {Instructions::IN_INC,  Instructions::AM_R,      Instructions::RT_D},
      {Instructions::IN_DEC,  Instructions::AM_R,      Instructions::RT_D},
      {Instructions::IN_LD,   Instructions::AM_R_D8,   Instructions::RT_D},
      {Instructions::IN_RLA},
      {Instructions::IN_JR,   Instructions::AM_D8},
      {Instructions::IN_ADD,  Instructions::AM_R_R,    Instructions::RT_HL,   Instructions::RT_DE},
      {Instructions::IN_LD,   Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_DE},
      {Instructions::IN_DEC,  Instructions::AM_R,      Instructions::RT_DE},
      {Instructions::IN_INC,  Instructions::AM_R,      Instructions::RT_E},
      {Instructions::IN_DEC,  Instructions::AM_R,      Instructions::RT_E},
      {Instructions::IN_LD,   Instructions::AM_R_D8,   Instructions::RT_E},
      {Instructions::IN_RRA},

      //0x2X
      {Instructions::IN_JR,   Instructions::AM_D8,     Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NZ},
      {Instructions::IN_LD,   Instructions::AM_R_D16,  Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_HLI_R,  Instructions::RT_HL,   Instructions::RT_A},
      {Instructions::IN_INC,  Instructions::AM_R,      Instructions::RT_HL},
      {Instructions::IN_INC,  Instructions::AM_R,      Instructions::RT_H},
      {Instructions::IN_DEC,  Instructions::AM_R,      Instructions::RT_H},
      {Instructions::IN_LD,   Instructions::AM_R_D8,   Instructions::RT_H},
      {Instructions::IN_DAA},
      {Instructions::IN_JR,   Instructions::AM_D8,     Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_Z},
      {Instructions::IN_ADD,  Instructions::AM_R_R,    Instructions::RT_HL,   Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_R_HLI,  Instructions::RT_A,    Instructions::RT_HL},
      {Instructions::IN_DEC,  Instructions::AM_R,      Instructions::RT_HL},
      {Instructions::IN_INC,  Instructions::AM_R,      Instructions::RT_L},
      {Instructions::IN_DEC,  Instructions::AM_R,      Instructions::RT_L},
      {Instructions::IN_LD,   Instructions::AM_R_D8,   Instructions::RT_L},
      {Instructions::IN_CPL},

      //0x3X
      {Instructions::IN_JR,   Instructions::AM_D8,     Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NC},
      {Instructions::IN_LD,   Instructions::AM_R_D16,  Instructions::RT_SP},
      {Instructions::IN_LD,   Instructions::AM_HLD_R,  Instructions::RT_HL,   Instructions::RT_A},
      {Instructions::IN_INC,  Instructions::AM_R,      Instructions::RT_SP},
      {Instructions::IN_INC,  Instructions::AM_MR,     Instructions::RT_HL},
      {Instructions::IN_DEC,  Instructions::AM_MR,     Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_MR_D8,  Instructions::RT_HL},
      {Instructions::IN_SCF},
      {Instructions::IN_JR,   Instructions::AM_D8,     Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_C},
      {Instructions::IN_ADD,  Instructions::AM_R_R,    Instructions::RT_HL,   Instructions::RT_SP},
      {Instructions::IN_LD,   Instructions::AM_R_HLD,  Instructions::RT_A,    Instructions::RT_HL},
      {Instructions::IN_DEC,  Instructions::AM_R,      Instructions::RT_SP},
      {Instructions::IN_INC,  Instructions::AM_R,      Instructions::RT_A},
      {Instructions::IN_DEC,  Instructions::AM_R,      Instructions::RT_A},
      {Instructions::IN_LD,   Instructions::AM_R_D8,   Instructions::RT_A},
      {Instructions::IN_CCF},

      //0x4X
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_B,    Instructions::RT_B},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_B,    Instructions::RT_C},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_B,    Instructions::RT_D},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_B,    Instructions::RT_E},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_B,    Instructions::RT_H},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_B,    Instructions::RT_L},
      {Instructions::IN_LD,   Instructions::AM_R_MR,   Instructions::RT_B,    Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_B,    Instructions::RT_A},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_C,    Instructions::RT_B},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_C,    Instructions::RT_C},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_C,    Instructions::RT_D},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_C,    Instructions::RT_E},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_C,    Instructions::RT_H},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_C,    Instructions::RT_L},
      {Instructions::IN_LD,   Instructions::AM_R_MR,   Instructions::RT_C,    Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_C,    Instructions::RT_A},

      //0x5X
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_D,    Instructions::RT_B},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_D,    Instructions::RT_C},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_D,    Instructions::RT_D},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_D,    Instructions::RT_E},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_D,    Instructions::RT_H},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_D,    Instructions::RT_L},
      {Instructions::IN_LD,   Instructions::AM_R_MR,   Instructions::RT_D,    Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_D,    Instructions::RT_A},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_E,    Instructions::RT_B},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_E,    Instructions::RT_C},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_E,    Instructions::RT_D},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_E,    Instructions::RT_E},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_E,    Instructions::RT_H},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_E,    Instructions::RT_L},
      {Instructions::IN_LD,   Instructions::AM_R_MR,   Instructions::RT_E,    Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_E,    Instructions::RT_A},

      //0x6X
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_H,    Instructions::RT_B},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_H,    Instructions::RT_C},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_H,    Instructions::RT_D},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_H,    Instructions::RT_E},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_H,    Instructions::RT_H},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_H,    Instructions::RT_L},
      {Instructions::IN_LD,   Instructions::AM_R_MR,   Instructions::RT_H,    Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_H,    Instructions::RT_A},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_L,    Instructions::RT_B},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_L,    Instructions::RT_C},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_L,    Instructions::RT_D},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_L,    Instructions::RT_E},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_L,    Instructions::RT_H},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_L,    Instructions::RT_L},
      {Instructions::IN_LD,   Instructions::AM_R_MR,   Instructions::RT_L,    Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_L,    Instructions::RT_A},

      //0x7X
      {Instructions::IN_LD,   Instructions::AM_MR_R,   Instructions::RT_HL,   Instructions::RT_B},
      {Instructions::IN_LD,   Instructions::AM_MR_R,   Instructions::RT_HL,   Instructions::RT_C},
      {Instructions::IN_LD,   Instructions::AM_MR_R,   Instructions::RT_HL,   Instructions::RT_D},
      {Instructions::IN_LD,   Instructions::AM_MR_R,   Instructions::RT_HL,   Instructions::RT_E},
      {Instructions::IN_LD,   Instructions::AM_MR_R,   Instructions::RT_HL,   Instructions::RT_H},
      {Instructions::IN_LD,   Instructions::AM_MR_R,   Instructions::RT_HL,   Instructions::RT_L},
      {Instructions::IN_HALT},
      {Instructions::IN_LD,   Instructions::AM_MR_R,   Instructions::RT_HL,   Instructions::RT_A},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_B},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_C},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_D},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_E},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_H},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_L},
      {Instructions::IN_LD,   Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_A},

      //0x8X
      {Instructions::IN_ADD,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_B},
      {Instructions::IN_ADD,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_C},
      {Instructions::IN_ADD,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_D},
      {Instructions::IN_ADD,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_E},
      {Instructions::IN_ADD,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_H},
      {Instructions::IN_ADD,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_L},
      {Instructions::IN_ADD,  Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_HL},
      {Instructions::IN_ADD,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_A},
      {Instructions::IN_ADC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_B},
      {Instructions::IN_ADC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_C},
      {Instructions::IN_ADC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_D},
      {Instructions::IN_ADC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_E},
      {Instructions::IN_ADC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_H},
      {Instructions::IN_ADC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_L},
      {Instructions::IN_ADC,  Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_HL},
      {Instructions::IN_ADC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_A},

      //0x9X
      {Instructions::IN_SUB,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_B},
      {Instructions::IN_SUB,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_C},
      {Instructions::IN_SUB,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_D},
      {Instructions::IN_SUB,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_E},
      {Instructions::IN_SUB,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_H},
      {Instructions::IN_SUB,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_L},
      {Instructions::IN_SUB,  Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_HL},
      {Instructions::IN_SUB,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_A},
      {Instructions::IN_SBC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_B},
      {Instructions::IN_SBC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_C},
      {Instructions::IN_SBC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_D},
      {Instructions::IN_SBC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_E},
      {Instructions::IN_SBC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_H},
      {Instructions::IN_SBC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_L},
      {Instructions::IN_SBC,  Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_HL},
      {Instructions::IN_SBC,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_A},


      //0xAX
      {Instructions::IN_AND,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_B},
      {Instructions::IN_AND,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_C},
      {Instructions::IN_AND,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_D},
      {Instructions::IN_AND,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_E},
      {Instructions::IN_AND,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_H},
      {Instructions::IN_AND,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_L},
      {Instructions::IN_AND,  Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_HL},
      {Instructions::IN_AND,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_A},
      {Instructions::IN_XOR,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_B},
      {Instructions::IN_XOR,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_C},
      {Instructions::IN_XOR,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_D},
      {Instructions::IN_XOR,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_E},
      {Instructions::IN_XOR,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_H},
      {Instructions::IN_XOR,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_L},
      {Instructions::IN_XOR,  Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_HL},
      {Instructions::IN_XOR,  Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_A},

      //0xBX
      {Instructions::IN_OR,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_B},
      {Instructions::IN_OR,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_C},
      {Instructions::IN_OR,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_D},
      {Instructions::IN_OR,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_E},
      {Instructions::IN_OR,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_H},
      {Instructions::IN_OR,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_L},
      {Instructions::IN_OR,   Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_HL},
      {Instructions::IN_OR,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_A},
      {Instructions::IN_CP,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_B},
      {Instructions::IN_CP,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_C},
      {Instructions::IN_CP,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_D},
      {Instructions::IN_CP,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_E},
      {Instructions::IN_CP,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_H},
      {Instructions::IN_CP,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_L},
      {Instructions::IN_CP,   Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_HL},
      {Instructions::IN_CP,   Instructions::AM_R_R,    Instructions::RT_A,    Instructions::RT_A},

      {Instructions::IN_RET,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NZ},
      {Instructions::IN_POP,  Instructions::AM_R,      Instructions::RT_BC},
      {Instructions::IN_JP,   Instructions::AM_D16,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NZ},
      {Instructions::IN_JP,   Instructions::AM_D16},
      {Instructions::IN_CALL, Instructions::AM_D16,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NZ},
      {Instructions::IN_PUSH, Instructions::AM_R,      Instructions::RT_BC},
      {Instructions::IN_ADD,  Instructions::AM_R_D8,   Instructions::RT_A},
      {Instructions::IN_RST,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NONE, 0x00},
      {Instructions::IN_RET,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_Z},
      {Instructions::IN_RET},
      {Instructions::IN_JP,   Instructions::AM_D16,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_Z},
      {Instructions::IN_CB,   Instructions::AM_D8},
      {Instructions::IN_CALL, Instructions::AM_D16,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_Z},
      {Instructions::IN_CALL, Instructions::AM_D16},
      {Instructions::IN_ADC,  Instructions::AM_R_D8,   Instructions::RT_A},
      {Instructions::IN_RST,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NONE, 0x08},

      {Instructions::IN_RET,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NC},
      {Instructions::IN_POP,  Instructions::AM_R,      Instructions::RT_DE},
      {Instructions::IN_JP,   Instructions::AM_D16,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NC},
      {Instructions::IN_CALL, Instructions::AM_D16,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NC},
      {Instructions::IN_PUSH, Instructions::AM_R,      Instructions::RT_DE},
      {Instructions::IN_SUB,  Instructions::AM_R_D8,   Instructions::RT_A},
      {Instructions::IN_RST,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NONE, 0x10},
      {Instructions::IN_RET,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_C},
      {Instructions::IN_RETI},
      {Instructions::IN_JP,   Instructions::AM_D16,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_C},
      {Instructions::IN_CALL, Instructions::AM_D16,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_C},
      {Instructions::IN_SBC,  Instructions::AM_R_D8,   Instructions::RT_A},
      {Instructions::IN_RST,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NONE, 0x18},

      //0xEX
      {Instructions::IN_LDH,  Instructions::AM_A8_R,   Instructions::RT_NONE, Instructions::RT_A},
      {Instructions::IN_POP,  Instructions::AM_R,      Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_MR_R,   Instructions::RT_C,    Instructions::RT_A},
      {Instructions::IN_PUSH, Instructions::AM_R,      Instructions::RT_HL},
      {Instructions::IN_AND,  Instructions::AM_R_D8,   Instructions::RT_A},
      {Instructions::IN_RST,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NONE, 0x20},
      {Instructions::IN_ADD,  Instructions::AM_R_D8,   Instructions::RT_SP},
      {Instructions::IN_JP,   Instructions::AM_R,      Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_A16_R,  Instructions::RT_NONE, Instructions::RT_A},
      {Instructions::IN_XOR,  Instructions::AM_R_D8,   Instructions::RT_A},
      {Instructions::IN_RST,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NONE, 0x28},


      //0xFX
      {Instructions::IN_LDH,  Instructions::AM_R_A8,   Instructions::RT_A},
      {Instructions::IN_POP,  Instructions::AM_R,      Instructions::RT_AF},
      {Instructions::IN_LD,   Instructions::AM_R_MR,   Instructions::RT_A,    Instructions::RT_C},
      {Instructions::IN_DI},
      {Instructions::IN_PUSH, Instructions::AM_R,      Instructions::RT_AF},
      {Instructions::IN_OR,   Instructions::AM_R_D8,   Instructions::RT_A},
      {Instructions::IN_RST,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NONE, 0x30},
      {Instructions::IN_LD,   Instructions::AM_HL_SPR, Instructions::RT_HL,   Instructions::RT_SP},
      {Instructions::IN_LD,   Instructions::AM_R_R,    Instructions::RT_SP,   Instructions::RT_HL},
      {Instructions::IN_LD,   Instructions::AM_R_A16,  Instructions::RT_A},
      {Instructions::IN_EI},
      {Instructions::IN_CP,   Instructions::AM_R_D8,   Instructions::RT_A},
      {Instructions::IN_RST,  Instructions::AM_IMP,    Instructions::RT_NONE, Instructions::RT_NONE, Instructions::CT_NONE, 0x38},
  };


  std::vector<std::string> m_InstructionNames = {
      "<NONE>",
      "NOP",
      "LD",
      "INC",
      "DEC",
      "RLCA",
      "ADD",
      "RRCA",
      "STOP",
      "RLA",
      "JR",
      "RRA",
      "DAA",
      "CPL",
      "SCF",
      "CCF",
      "HALT",
      "ADC",
      "SUB",
      "SBC",
      "AND",
      "XOR",
      "OR",
      "CP",
      "POP",
      "JP",
      "PUSH",
      "RET",
      "CB",
      "CALL",
      "RETI",
      "LDH",
      "JPHL",
      "DI",
      "EI",
      "RST",
      "ERR",
      "IN_RLC",
      "IN_RRC",
      "IN_RL",
      "IN_RR",
      "N_SLA",
      "IN_SRA",
      "IN_SWAP",
      "IN_SRL",
      "IN_BIT",
      "IN_RES",
      "IN_SET"
  };

  std::vector<std::string> m_RegisterNames{
      "<NONE>",
      "A",
      "F",
      "B",
      "C",
      "D",
      "E",
      "H",
      "L",
      "AF",
      "BC",
      "DE",
      "HL",
      "SP",
      "PC"
  };
} // hijo