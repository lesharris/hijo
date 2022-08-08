
#include "MBC2.h"

#include <fmt/format.h>

namespace hijo {
  uint8_t MBC2::Read(uint16_t addr) {
    uint16_t addrEnd = (m_RomBankCount == 2) ? 0x8000 : 0x4000;

    if (addr < addrEnd) {
      return m_Data[addr];
    }

    if ((addr & 0xE000) == 0xA000) {
      if (!m_RamEnabled) {
        return 0x0F;
      }

      return m_Ram[addr & 0x1FF] & 0xF;
    }

    return m_Data[m_RomBankBase + (addr - 0x4000)];
  }

  void MBC2::Write(uint16_t addr, uint8_t data) {
    if (addr < 0x4000) {
      bool bit8 = (((addr & 0xFF00) >> 8) & 1) == 1;

      if (bit8) {
        // Set ROM Bank
        uint8_t bank = data & 0xF;
        if (bank == 0)
          bank = 1;

        SetRomBank(bank);
      } else {
        // Enable/Disable RAM
        m_RamEnabled = data == 0x0A;
      }

      return;
    }

    if (addr < 0xC000) {
      if (!m_RamEnabled)
        return;

      m_Ram[addr & 0x1FF] = data & 0xF;
    }
  }

  void MBC2::SetRomData(std::vector<uint8_t> &data) {
    m_Data = data;
  }

  void MBC2::SetRomBanks(uint16_t bankCount) {
    m_RomBankCount = bankCount;
    SetRomBank(1);
  }
  
  std::vector<Mapper::StatLine> MBC2::GetStats() {
    std::vector<StatLine> lines;

    lines.push_back({"Rom Bank Count", fmt::format("{}", m_RomBankCount)});
    lines.push_back({"Banking Mode", fmt::format("{}", m_BankingMode ? "RAM" : "ROM")});
    lines.push_back({"Ram Enabled?", fmt::format("{}", m_RamEnabled ? "Yes" : "No")});
    lines.push_back({"Selected Rom Bank", fmt::format("{}", m_RomBankValue)});

    lines.push_back({"Rom Bank Base", fmt::format("0x{:04X}", m_RomBankBase)});
    lines.push_back({"Needs Save?", m_NeedsSave ? "Yes" : "No"});

    return lines;
  }

  void MBC2::SetRomBank(uint8_t value) {
    m_RomBankValue = value % m_RomBankCount;
    m_RomBankBase = 0x4000 * value;
  }
} // hijo