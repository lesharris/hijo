#include "Instructions.h"

#include <fmt/format.h>

namespace hijo {
  Instructions::Opcode &Instructions::OpcodeByByte(uint8_t opcode) {
    return m_Instructions[opcode];
  }

  std::string Instructions::InstructionToString(const Instructions::Opcode &t) {
  }

} // hijo