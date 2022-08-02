#include "SharpSM83.h"
#include "Interrupts.h"
#include "system/Gameboy.h"

namespace hijo {
  SharpSM83::SharpSM83() {
    Reset();
  }

  void SharpSM83::Reset() {
    auto &bus = Gameboy::Get();

    regs.pc = 0x100;
    regs.sp = 0xFFFE;
    *((short *) &regs.a) = 0xB001;
    *((short *) &regs.b) = 0x1300;
    *((short *) &regs.d) = 0xD800;
    *((short *) &regs.h) = 0x4D01;
    ie_register = 0;
    int_flags = 0;
    int_master_enabled = false;
    enabling_ime = false;

    bus.m_Timer.div = 0xABCC;
  }

  uint16_t SharpSM83::Register(const Instructions::RegisterType &t) {
    switch (t) {
      case Instructions::RegisterType::RT_A:
        return regs.a;
      case Instructions::RegisterType::RT_F:
        return regs.f;
      case Instructions::RegisterType::RT_B:
        return regs.b;
      case Instructions::RegisterType::RT_C:
        return regs.c;
      case Instructions::RegisterType::RT_D:
        return regs.d;
      case Instructions::RegisterType::RT_E:
        return regs.e;
      case Instructions::RegisterType::RT_H:
        return regs.h;
      case Instructions::RegisterType::RT_L:
        return regs.l;

      case Instructions::RegisterType::RT_AF:
        return reverse(*((uint16_t *) &regs.a));
      case Instructions::RegisterType::RT_BC:
        return reverse(*((uint16_t *) &regs.b));
      case Instructions::RegisterType::RT_DE:
        return reverse(*((uint16_t *) &regs.d));
      case Instructions::RegisterType::RT_HL:
        return reverse(*((uint16_t *) &regs.h));

      case Instructions::RegisterType::RT_PC:
        return regs.pc;
      case Instructions::RegisterType::RT_SP:
        return regs.sp;
      default:
        return 0;
    }
  }

  void SharpSM83::Register(const Instructions::RegisterType &t, uint16_t val) {
    switch (t) {
      case Instructions::RegisterType::RT_A:
        regs.a = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_F:
        regs.f = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_B:
        regs.b = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_C: {
        regs.c = val & 0xFF;
      }
        break;
      case Instructions::RegisterType::RT_D:
        regs.d = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_E:
        regs.e = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_H:
        regs.h = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_L:
        regs.l = val & 0xFF;
        break;

      case Instructions::RegisterType::RT_AF:
        *((uint16_t *) &regs.a) = reverse(val);
        break;
      case Instructions::RegisterType::RT_BC:
        *((uint16_t *) &regs.b) = reverse(val);
        break;
      case Instructions::RegisterType::RT_DE:
        *((uint16_t *) &regs.d) = reverse(val);
        break;
      case Instructions::RegisterType::RT_HL: {
        *((uint16_t *) &regs.h) = reverse(val);
        break;
      }

      case Instructions::RegisterType::RT_PC:
        regs.pc = val;
        break;
      case Instructions::RegisterType::RT_SP:
        regs.sp = val;
        break;
      case Instructions::RegisterType::RT_NONE:
        break;
    }
  }

  uint8_t SharpSM83::Register8(const Instructions::RegisterType &t) {
    auto &bus = Gameboy::Get();
    switch (t) {
      case Instructions::RegisterType::RT_A:
        return regs.a;
      case Instructions::RegisterType::RT_F:
        return regs.f;
      case Instructions::RegisterType::RT_B:
        return regs.b;
      case Instructions::RegisterType::RT_C:
        return regs.c;
      case Instructions::RegisterType::RT_D:
        return regs.d;
      case Instructions::RegisterType::RT_E:
        return regs.e;
      case Instructions::RegisterType::RT_H:
        return regs.h;
      case Instructions::RegisterType::RT_L:
        return regs.l;
      case Instructions::RegisterType::RT_HL: {
        return bus.cpuRead(Register(Instructions::RegisterType::RT_HL));
      }
      default:
        break;
    }
  }

  void SharpSM83::Register8(const Instructions::RegisterType &t, uint8_t val) {
    auto &bus = Gameboy::Get();

    switch (t) {
      case Instructions::RegisterType::RT_A:
        regs.a = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_F:
        regs.f = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_B:
        regs.b = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_C:
        regs.c = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_D:
        regs.d = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_E:
        regs.e = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_H:
        regs.h = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_L:
        regs.l = val & 0xFF;
        break;
      case Instructions::RegisterType::RT_HL:
        bus.cpuWrite(Register(Instructions::RegisterType::RT_HL), val);
        break;
      default:
        break;
    }
  }

  SharpSM83::InstructionProc SharpSM83::GetProcessor(Instructions::InstructionType type) {
    return m_Processors[type];
  }

  void SharpSM83::FetchData() {
    auto &bus = Gameboy::Get();

    mem_dest = 0;
    dest_is_mem = false;

    if (cur_inst == NULL) {
      return;
    }

    switch (cur_inst->mode) {
      case Instructions::AddressMode::AM_IMP:
        return;

      case Instructions::AddressMode::AM_R:
        fetched_data = Register(cur_inst->reg1);
        return;

      case Instructions::AddressMode::AM_R_R:
        fetched_data = Register(cur_inst->reg2);
        return;

      case Instructions::AddressMode::AM_R_D8:
        fetched_data = bus.cpuRead(regs.pc);
        bus.Cycles(1);
        regs.pc++;
        return;

      case Instructions::AddressMode::AM_R_D16:
      case Instructions::AddressMode::AM_D16: {
        uint16_t lo = bus.cpuRead(regs.pc);
        bus.Cycles(1);

        uint16_t hi = bus.cpuRead(regs.pc + 1);
        bus.Cycles(1);

        fetched_data = lo | (hi << 8);

        regs.pc += 2;

        return;
      }

      case Instructions::AddressMode::AM_MR_R:
        fetched_data = Register(cur_inst->reg2);
        mem_dest = Register(cur_inst->reg1);
        dest_is_mem = true;

        if (cur_inst->reg1 == Instructions::RegisterType::RT_C) {
          mem_dest |= 0xFF00;
        }

        return;

      case Instructions::AddressMode::AM_R_MR: {
        uint16_t addr = Register(cur_inst->reg2);

        if (cur_inst->reg2 == Instructions::RegisterType::RT_C) {
          addr |= 0xFF00;
        }

        fetched_data = bus.cpuRead(addr);
        bus.Cycles(1);

      }
        return;

      case Instructions::AddressMode::AM_R_HLI:
        fetched_data = bus.cpuRead(Register(cur_inst->reg2));
        bus.Cycles(1);
        Register(Instructions::RegisterType::RT_HL, Register(Instructions::RegisterType::RT_HL) + 1);
        return;

      case Instructions::AddressMode::AM_R_HLD:
        fetched_data = bus.cpuRead(Register(cur_inst->reg2));
        bus.Cycles(1);
        Register(Instructions::RegisterType::RT_HL, Register(Instructions::RegisterType::RT_HL) - 1);
        return;

      case Instructions::AddressMode::AM_HLI_R:
        fetched_data = Register(cur_inst->reg2);
        mem_dest = Register(cur_inst->reg1);
        dest_is_mem = true;
        Register(Instructions::RegisterType::RT_HL, Register(Instructions::RegisterType::RT_HL) + 1);
        return;

      case Instructions::AddressMode::AM_HLD_R:
        fetched_data = Register(cur_inst->reg2);
        mem_dest = Register(cur_inst->reg1);
        dest_is_mem = true;
        Register(Instructions::RegisterType::RT_HL, Register(Instructions::RegisterType::RT_HL) - 1);
        return;

      case Instructions::AddressMode::AM_R_A8:
        fetched_data = bus.cpuRead(regs.pc);
        bus.Cycles(1);
        regs.pc++;
        return;

      case Instructions::AddressMode::AM_A8_R:
        mem_dest = bus.cpuRead(regs.pc) | 0xFF00;
        dest_is_mem = true;
        bus.Cycles(1);
        regs.pc++;
        return;

      case Instructions::AddressMode::AM_HL_SPR:
        fetched_data = bus.cpuRead(regs.pc);
        bus.Cycles(1);
        regs.pc++;
        return;

      case Instructions::AddressMode::AM_D8:
        fetched_data = bus.cpuRead(regs.pc);
        bus.Cycles(1);
        regs.pc++;
        return;

      case Instructions::AddressMode::AM_A16_R:
      case Instructions::AddressMode::AM_D16_R: {
        uint16_t lo = bus.cpuRead(regs.pc);
        bus.Cycles(1);

        uint16_t hi = bus.cpuRead(regs.pc + 1);
        bus.Cycles(1);

        mem_dest = lo | (hi << 8);
        dest_is_mem = true;

        regs.pc += 2;
        fetched_data = Register(cur_inst->reg2);

      }
        return;

      case Instructions::AddressMode::AM_MR_D8:
        fetched_data = bus.cpuRead(regs.pc);
        bus.Cycles(1);
        regs.pc++;
        mem_dest = Register(cur_inst->reg1);
        dest_is_mem = true;
        return;

      case Instructions::AddressMode::AM_MR:
        mem_dest = Register(cur_inst->reg1);
        dest_is_mem = true;
        fetched_data = bus.cpuRead(Register(cur_inst->reg1));
        bus.Cycles(1);
        return;

      case Instructions::AddressMode::AM_R_A16: {
        uint16_t lo = bus.cpuRead(regs.pc);
        bus.Cycles(1);

        uint16_t hi = bus.cpuRead(regs.pc + 1);
        bus.Cycles(1);

        uint16_t addr = lo | (hi << 8);

        regs.pc += 2;
        fetched_data = bus.cpuRead(addr);
        bus.Cycles(1);

        return;
      }

      default:
        printf("Unknown Addressing Mode! %d (%02X)\n", cur_inst->mode, cur_opcode);
        exit(-7);
    }
  }

  void SharpSM83::FetchInstruction() {
    auto &bus = Gameboy::Get();

    cur_opcode = bus.cpuRead(regs.pc++);
    cur_inst = &instrs.InstructionByOpcode(cur_opcode);
  }

  void SharpSM83::Execute() {
    auto proc = GetProcessor(cur_inst->type);

    if (!proc) {
      return;
    }

    (this->*proc)();
  }

  bool SharpSM83::Step() {
    auto &bus = Gameboy::Get();

    if (!halted) {
      FetchInstruction();
      bus.Cycles(1);
      FetchData();

      if (cur_inst == nullptr) {
        exit(-1);
      }

      Execute();
    } else {
      bus.Cycles(1);

      if (int_flags) {
        halted = false;
      }
    }

    if (int_master_enabled) {
      Interrupts::HandleInterrupts(*this);
      enabling_ime = false;
    }

    if (enabling_ime) {
      int_master_enabled = true;
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

  Instructions::RegisterType SharpSM83::DecodeRegister(uint8_t reg) {
    if (reg > 0x7) {
      return Instructions::RegisterType::RT_NONE;
    }

    return rt_lookup[reg];
  }

  bool SharpSM83::Is16Bit(const Instructions::RegisterType &t) {
    return static_cast<uint8_t>(t) > static_cast<uint8_t>(Instructions::RegisterType::RT_AF);
  }

  bool SharpSM83::CheckCondition() {
    bool z = CPU_FLAG_Z;
    bool c = CPU_FLAG_C;

    switch (cur_inst->cond) {
      case Instructions::ConditionType::CT_NONE:
        return true;
      case Instructions::ConditionType::CT_C:
        return c;
      case Instructions::ConditionType::CT_NC:
        return !c;
      case Instructions::ConditionType::CT_Z:
        return z;
      case Instructions::ConditionType::CT_NZ:
        return !z;
    }

    return false;
  }

  void SharpSM83::GotoAddress(uint16_t addr, bool pushPC) {
    auto &bus = Gameboy::Get();

    if (CheckCondition()) {
      if (pushPC) {
        bus.Cycles(2);
        Stack::Push16(regs.pc);
      }

      regs.pc = addr;
      //bus.cycles(1);
    }
  }

  void SharpSM83::ProcNone() {

  }

  void SharpSM83::ProcNOP() {

  }

  void SharpSM83::ProcCB() {
    auto &bus = Gameboy::Get();

    uint8_t op = fetched_data;
    Instructions::RegisterType reg = DecodeRegister(op & 0b111);
    uint8_t bit = (op >> 3) & 0b111;
    uint8_t bit_op = (op >> 6) & 0b11;
    uint8_t reg_val = Register8(reg);

    bus.Cycles(1);

    if (reg == Instructions::RegisterType::RT_HL) {
      bus.Cycles(2);
    }

    switch (bit_op) {
      case 1:
        //BIT
        SetFlags(!(reg_val & (1 << bit)), 0, 1, -1);
        return;

      case 2:
        //RST
        reg_val &= ~(1 << bit);
        Register8(reg, reg_val);
        return;

      case 3:
        //SET
        reg_val |= (1 << bit);
        Register8(reg, reg_val);
        return;
    }

    bool flagC = CPU_FLAG_C;

    switch (bit) {
      case 0: {
        //RLC
        bool setC = false;
        uint8_t result = (reg_val << 1) & 0xFF;

        if ((reg_val & (1 << 7)) != 0) {
          result |= 1;
          setC = true;
        }

        Register8(reg, result);
        SetFlags(result == 0, false, false, setC);
      }
        return;

      case 1: {
        //RRC
        uint8_t old = reg_val;
        reg_val >>= 1;
        reg_val |= (old << 7);

        Register8(reg, reg_val);
        SetFlags(!reg_val, false, false, old & 1);
      }
        return;

      case 2: {
        //RL
        uint8_t old = reg_val;
        reg_val <<= 1;
        reg_val |= flagC;

        Register8(reg, reg_val);
        SetFlags(!reg_val, false, false, !!(old & 0x80));
      }
        return;

      case 3: {
        //RR
        uint8_t old = reg_val;
        reg_val >>= 1;

        reg_val |= (flagC << 7);

        Register8(reg, reg_val);
        SetFlags(!reg_val, false, false, old & 1);
      }
        return;

      case 4: {
        //SLA
        uint8_t old = reg_val;
        reg_val <<= 1;

        Register8(reg, reg_val);
        SetFlags(!reg_val, false, false, !!(old & 0x80));
      }
        return;

      case 5: {
        //SRA
        uint8_t u = (int8_t) reg_val >> 1;
        Register8(reg, u);
        SetFlags(!u, 0, 0, reg_val & 1);
      }
        return;

      case 6: {
        //SWAP
        reg_val = ((reg_val & 0xF0) >> 4) | ((reg_val & 0xF) << 4);
        Register8(reg, reg_val);
        SetFlags(reg_val == 0, false, false, false);
      }
        return;

      case 7: {
        //SRL
        uint8_t u = reg_val >> 1;
        Register8(reg, u);
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
    uint8_t cf = CPU_FLAG_C;
    uint8_t c = (u >> 7) & 1;

    regs.a = (u << 1) | cf;
    SetFlags(0, 0, 0, c);
  }

  void SharpSM83::ProcSTOP() {

  }

  void SharpSM83::ProcDAA() {
    uint8_t u = 0;
    int fc = 0;

    if (CPU_FLAG_H || (!CPU_FLAG_N && (regs.a & 0xF) > 9)) {
      u = 6;
    }

    if (CPU_FLAG_C || (!CPU_FLAG_N && regs.a > 0x99)) {
      u |= 0x60;
      fc = 1;
    }

    regs.a += CPU_FLAG_N ? -u : u;

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
    SetFlags(-1, 0, 0, CPU_FLAG_C ^ 1);
  }

  void SharpSM83::ProcHALT() {
    halted = true;
  }

  void SharpSM83::ProcRRA() {
    uint8_t carry = CPU_FLAG_C;
    uint8_t new_c = regs.a & 1;

    regs.a >>= 1;
    regs.a |= (carry << 7);

    SetFlags(0, 0, 0, new_c);
  }

  void SharpSM83::ProcAND() {
    regs.a &= fetched_data;
    SetFlags(regs.a == 0, 0, 1, 0);
  }

  void SharpSM83::ProcXOR() {
    regs.a ^= fetched_data & 0xFF;
    SetFlags(regs.a == 0, 0, 0, 0);
  }

  void SharpSM83::ProcOR() {
    regs.a |= fetched_data & 0xFF;
    SetFlags(regs.a == 0, 0, 0, 0);
  }

  void SharpSM83::ProcCP() {
    int n = (int) regs.a - (int) fetched_data;

    SetFlags(n == 0, 1,
             ((int) regs.a & 0x0F) - ((int) fetched_data & 0x0F) < 0, n < 0);
  }

  void SharpSM83::ProcDI() {
    int_master_enabled = false;
  }

  void SharpSM83::ProcEI() {
    enabling_ime = true;
  }

  void SharpSM83::ProcLD() {
    if (dest_is_mem) {
      //LD (BC), A for instance...

      if (Is16Bit(cur_inst->reg2)) {
        //if 16 bit register...
        Gameboy::Get().Cycles(1);
        Gameboy::Get().cpuWrite16(mem_dest, fetched_data);
      } else {
        Gameboy::Get().cpuWrite(mem_dest, fetched_data);
      }

      Gameboy::Get().Cycles(1);

      return;
    }

    if (cur_inst->mode == Instructions::AddressMode::AM_HL_SPR) {
      uint8_t hflag = (Register(cur_inst->reg2) & 0xF) +
                      (fetched_data & 0xF) >= 0x10;

      uint8_t cflag = (Register(cur_inst->reg2) & 0xFF) +
                      (fetched_data & 0xFF) >= 0x100;

      SetFlags(0, 0, hflag, cflag);
      Register(cur_inst->reg1,
               Register(cur_inst->reg2) + (int8_t) fetched_data);

      return;
    }

    Register(cur_inst->reg1, fetched_data);
  }

  void SharpSM83::ProcLDH() {
    if (cur_inst->reg1 == Instructions::RegisterType::RT_A) {
      Register(cur_inst->reg1, Gameboy::Get().cpuRead(0xFF00 | fetched_data));
    } else {
      Gameboy::Get().cpuWrite(mem_dest, regs.a);
    }

    Gameboy::Get().Cycles(1);
  }

  void SharpSM83::ProcJP() {
    GotoAddress(fetched_data, false);
  }

  void SharpSM83::ProcJR() {
    int8_t rel = (int8_t) (fetched_data & 0xFF);
    uint16_t addr = regs.pc + rel;
    GotoAddress(addr, false);
  }

  void SharpSM83::ProcCALL() {
    GotoAddress(fetched_data, true);
  }

  void SharpSM83::ProcRST() {
    GotoAddress(cur_inst->param, true);
  }

  void SharpSM83::ProcRET() {
    if (cur_inst->cond != Instructions::ConditionType::CT_NONE) {
      Gameboy::Get().Cycles(1);
    }

    if (CheckCondition()) {
      uint16_t lo = Stack::Pop();
      Gameboy::Get().Cycles(1);
      uint16_t hi = Stack::Pop();
      Gameboy::Get().Cycles(1);

      uint16_t n = (hi << 8) | lo;
      regs.pc = n;

      Gameboy::Get().Cycles(1);
    }
  }

  void SharpSM83::ProcRETI() {
    int_master_enabled = true;
    ProcRET();
  }

  void SharpSM83::ProcPOP() {
    uint16_t lo = Stack::Pop();
    Gameboy::Get().Cycles(1);
    uint16_t hi = Stack::Pop();
    Gameboy::Get().Cycles(1);

    uint16_t n = (hi << 8) | lo;

    Register(cur_inst->reg1, n);

    if (cur_inst->reg1 == Instructions::RegisterType::RT_AF) {
      Register(cur_inst->reg1, n & 0xFFF0);
    }
  }

  void SharpSM83::ProcPUSH() {
    uint16_t hi = (Register(cur_inst->reg1) >> 8) & 0xFF;
    Gameboy::Get().Cycles(1);
    Stack::Push(hi);

    uint16_t lo = Register(cur_inst->reg1) & 0xFF;
    Gameboy::Get().Cycles(1);
    Stack::Push(lo);

    Gameboy::Get().Cycles(1);
  }

  void SharpSM83::ProcINC() {
    uint16_t val = Register(cur_inst->reg1) + 1;

    if (Is16Bit(cur_inst->reg1)) {
      Gameboy::Get().Cycles(1);
    }

    if (cur_inst->reg1 == Instructions::RegisterType::RT_HL && cur_inst->mode == Instructions::AddressMode::AM_MR) {
      val = Gameboy::Get().cpuRead(Register(Instructions::RegisterType::RT_HL)) + 1;
      val &= 0xFF;
      Gameboy::Get().cpuWrite(Register(Instructions::RegisterType::RT_HL), val);
    } else {
      Register(cur_inst->reg1, val);
      val = Register(cur_inst->reg1);
    }

    if ((cur_opcode & 0x03) == 0x03) {
      return;
    }

    SetFlags(val == 0, 0, (val & 0x0F) == 0, -1);
  }

  void SharpSM83::ProcDEC() {
    uint16_t val = Register(cur_inst->reg1) - 1;

    if (Is16Bit(cur_inst->reg1)) {
      Gameboy::Get().Cycles(1);
    }

    if (cur_inst->reg1 == Instructions::RegisterType::RT_HL && cur_inst->mode == Instructions::AddressMode::AM_MR) {
      val = Gameboy::Get().cpuRead(Register(Instructions::RegisterType::RT_HL)) - 1;
      Gameboy::Get().cpuWrite(Register(Instructions::RegisterType::RT_HL), val);
    } else {
      Register(cur_inst->reg1, val);
      val = Register(cur_inst->reg1);
    }

    if ((cur_opcode & 0x0B) == 0x0B) {
      return;
    }

    SetFlags(val == 0, 1, (val & 0x0F) == 0x0F, -1);
  }

  void SharpSM83::ProcSUB() {
    uint16_t val = Register(cur_inst->reg1) - fetched_data;

    int z = val == 0;
    int h = ((int) Register(cur_inst->reg1) & 0xF) - ((int) fetched_data & 0xF) < 0;
    int c = ((int) Register(cur_inst->reg1)) - ((int) fetched_data) < 0;

    Register(cur_inst->reg1, val);
    SetFlags(z, 1, h, c);
  }

  void SharpSM83::ProcSBC() {
    uint8_t val = fetched_data + CPU_FLAG_C;

    int z = Register(cur_inst->reg1) - val == 0;

    int h = ((int) Register(cur_inst->reg1) & 0xF)
            - ((int) fetched_data & 0xF) - ((int) CPU_FLAG_C) < 0;
    int c = ((int) Register(cur_inst->reg1))
            - ((int) fetched_data) - ((int) CPU_FLAG_C) < 0;

    Register(cur_inst->reg1, Register(cur_inst->reg1) - val);
    SetFlags(z, 1, h, c);
  }

  void SharpSM83::ProcADC() {
    uint16_t u = fetched_data;
    uint16_t a = regs.a;
    uint16_t c = CPU_FLAG_C;

    regs.a = (a + u + c) & 0xFF;

    SetFlags(regs.a == 0, 0,
             (a & 0xF) + (u & 0xF) + c > 0xF,
             a + u + c > 0xFF);
  }

  void SharpSM83::ProcADD() {
    uint32_t val = Register(cur_inst->reg1) + fetched_data;

    bool is_16bit = Is16Bit(cur_inst->reg1);

    if (is_16bit) {
      Gameboy::Get().Cycles(1);
    }

    if (cur_inst->reg1 == Instructions::RegisterType::RT_SP) {
      val = Register(cur_inst->reg1) + (int8_t) fetched_data;
    }

    int z = (val & 0xFF) == 0;
    int h = (Register(cur_inst->reg1) & 0xF) + (fetched_data & 0xF) >= 0x10;
    int c = (int) (Register(cur_inst->reg1) & 0xFF) + (int) (fetched_data & 0xFF) >= 0x100;

    if (is_16bit) {
      z = -1;
      h = (Register(cur_inst->reg1) & 0xFFF) + (fetched_data & 0xFFF) >= 0x1000;
      uint32_t n = ((uint32_t) Register(cur_inst->reg1)) + ((uint32_t) fetched_data);
      c = n >= 0x10000;
    }

    if (cur_inst->reg1 == Instructions::RegisterType::RT_SP) {
      z = 0;
      h = (Register(cur_inst->reg1) & 0xF) + (fetched_data & 0xF) >= 0x10;
      c = (int) (Register(cur_inst->reg1) & 0xFF) + (int) (fetched_data & 0xFF) >= 0x100;
    }

    Register(cur_inst->reg1, val & 0xFFFF);
    SetFlags(z, 0, h, c);
  }

  uint16_t SharpSM83::reverse(uint16_t n) {
    return ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
  }

} // hijo