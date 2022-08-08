
#include "MBC1.h"
#include <fmt/format.h>

namespace hijo {

  uint8_t MBC1::Read(uint16_t addr) {
    uint16_t addrEnd = (m_RomBankCount == 2) ? 0x8000 : 0x4000;

    if (addr < addrEnd) {
      return m_Data[addr];
    }

    if ((addr & 0xE000) == 0xA000) {
      if (!m_RamEnabled) {
        return 0xFF;
      }

      if (m_RamBanks.size() == 0 || m_RamBankValue >= m_RamBanks.size()) {
        return 0xFF;
      }

      return m_RamBanks[m_RamBankValue][addr - 0xA000];
    }

    return m_Data[m_RomBankBase + (addr - 0x4000)];
  }

  void MBC1::Write(uint16_t addr, uint8_t data) {
    if (addr < 0x2000) {
      m_RamEnabled = ((data & 0xF) == 0xA);
    }

    switch (addr & 0xE000) {
      // Rom Bank Number
      case 0x2000:
        if (data == 0) {
          data = 1;
        }

        data &= 0x1F;

        SetRomBank(data);
        break;

        // Ram Bank Number
      case 0x4000:
        SetRamBank(data & 0x3);

        if (m_RamBanking) {
          // cart save here
          /*  if (cart_need_save()) {
                cart_battery_save();
            }*/
        }
        break;

        // Banking Mode Select
      case 0x6000:
        m_BankingMode = data & 1;
        m_RamBanking = m_BankingMode;

        if (m_RamBanking) {
          // cart save here
          /*  if (cart_need_save()) {
                cart_battery_save();
            }*/
        }
        break;

        // RAM Write
      case 0xA000:
        if (!m_RamEnabled)
          return;

        if (m_RamBanks.size() == 0 || m_RamBankValue >= m_RamBanks.size()) {
          return;
        }

        m_RamBanks[m_RamBankValue][addr - 0xA000] = data;

        if (m_HasBattery) {
          m_NeedsSave = true;
        }
        break;
    }
  }

  void MBC1::SetRomData(std::vector<uint8_t> &data) {
    m_Data = data;
  }

  void MBC1::SetRomBanks(uint16_t bankCount) {
    m_RomBankCount = bankCount;
    SetRomBank(0);
  }

  void MBC1::SetRamBanks(uint8_t bankCount) {
    m_RamBanks.clear();
    m_RamBankCount = bankCount;

    if (m_RamBankCount > 0) {
      for (uint8_t i = 0; i < m_RamBankCount; i++) {
        std::vector<uint8_t> bank(0x2000);
        m_RamBanks.push_back(bank);
      }
    }
  }

  void MBC1::SetRomBank(uint8_t value) {
    m_RomBankValue = value % m_RomBankCount;
    m_RomBankBase = 0x4000 * value;
  }

  void MBC1::SetRamBank(uint8_t value) {
    m_RamBankValue = value;
  }


  std::vector<Mapper::StatLine> MBC1::GetStats() {
    std::vector<StatLine> lines;

    lines.push_back({"Rom Bank Count", fmt::format("{}", m_RomBankCount)});
    lines.push_back({"Ram Bank Count", fmt::format("{}", m_RamBankCount)});

    lines.push_back({"Banking Mode", fmt::format("{}", m_BankingMode ? "RAM" : "ROM")});

    lines.push_back({"Ram Enabled?", fmt::format("{}", m_RamEnabled ? "Yes" : "No")});
    lines.push_back({"Ram Banking?", fmt::format("{}", m_RamBanking ? "Yes" : "No")});

    lines.push_back({"Selected Rom Bank", fmt::format("{}", m_RomBankValue)});
    lines.push_back({"Selected Ram Bank", fmt::format("{}", m_RamBankValue)});

    lines.push_back({"Rom Bank Base", fmt::format("0x{:04X}", m_RomBankBase)});
    lines.push_back({"Needs Save?", m_NeedsSave ? "Yes" : "No"});

    return lines;
  }
} // hijo