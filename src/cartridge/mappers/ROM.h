#pragma once

#include <cstdint>
#include <vector>

#include "Mapper.h"

namespace hijo {

  class ROM : public Mapper {
  public:
    void SetRomData(std::vector<uint8_t> &data) override;

    uint8_t Read(uint16_t addr) override;

    void Write(uint16_t addr, uint8_t data) override;

    std::vector<StatLine> GetStats() override;

    void SetRomBanks(uint16_t bankCount) override;

    void SetRamBanks(uint8_t bankCount) override;

  private:
    std::vector<uint8_t> m_Data;
  };

} // hijo

