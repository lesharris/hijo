#include "ROM.h"

namespace hijo {
  uint8_t ROM::Read(uint16_t addr) {
    return m_Data[addr];
  }

  void ROM::Write(uint16_t, uint8_t) {
    // Nothing to do with no Mapper.
  }

  void ROM::SetRomData(std::vector<uint8_t> &data) {
    m_Data = data;
  }

  void ROM::SetRomBanks(uint16_t) {

  }

  void ROM::SetRamBanks(uint8_t) {

  }

  std::vector<Mapper::StatLine> ROM::GetStats() {
    return std::vector<Mapper::StatLine>();
  }

} // hijo