#pragma once

#include <cstdint>
#include <vector>

#include "Mapper.h"

namespace hijo {

  class ROM : public Mapper {
  public:
    ROM(const std::string &path) : Mapper(path) {}

    uint8_t Read(uint16_t addr) override;

    void Write(uint16_t addr, uint8_t data) override;

    std::vector<StatLine> GetStats() override;

  private:
    std::vector<uint8_t> m_Data;
  };

} // hijo

