#include "ROM.h"

namespace hijo {
  uint8_t ROM::Read(uint16_t addr) {
    return m_Data[addr];
  }

  void ROM::Write(uint16_t, uint8_t) {
    // Nothing to do with no Mapper.
  }

  std::vector<Mapper::StatLine> ROM::GetStats() {
    return std::vector<Mapper::StatLine>();
  }

} // hijo