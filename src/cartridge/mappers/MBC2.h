#pragma once

#include <vector>

#include "Mapper.h"

namespace hijo {

  class MBC2 : public Mapper {
  public:
    MBC2(const std::string &path) : Mapper(path) {}

    uint8_t Read(uint16_t addr) override;

    void Write(uint16_t addr, uint8_t data) override;

    void SetRomData(std::vector<uint8_t> &data) override;

    void SetRomBanks(uint16_t bankCount) override;

    void SetRamBanks(uint8_t uint8) override;

    std::vector<StatLine> GetStats() override;

  private:
    void SetRomBank(uint8_t value);

    void SaveRam();

    void LoadRam();

  private:
    std::vector<uint8_t> m_Ram = std::vector<uint8_t>(512);

    uint16_t m_RomBankCount;

    bool m_RamEnabled = false;

    uint8_t m_BankingMode = 0;

    uint8_t m_RomBankValue = 0;
    uint32_t m_RomBankBase = 0;

    bool m_NeedsSave = false;
  };

} // hijo

