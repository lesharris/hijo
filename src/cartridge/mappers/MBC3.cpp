#include "MBC3.h"

#include "common/common.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "system/Gameboy.h"

namespace hijo {
  uint8_t MBC3::Read(uint16_t addr) {
    uint16_t addrEnd = (m_RomBankCount == 2) ? 0x8000 : 0x4000;

    if (addr < addrEnd) {
      return m_Data[addr];
    }

    if ((addr & 0xE000) == 0xA000) {
      if (!m_RamEnabled) {
        return 0xFF;
      }

      if (m_RTCBanked) {
        switch (m_SelectedField) {
          case RTCField::Seconds:
            return m_RTC.seconds;
          case RTCField::Minutes:
            return m_RTC.minutes;
          case RTCField::Hours:
            return m_RTC.hours;
          case RTCField::DayLow:
            return (m_RTC.day & 0xFF00) >> 8;
          case RTCField::DayHigh:
            return m_RTC.day & 0xFF;
        }
      }

      if (m_RamBanks.size() == 0 || m_RamBankValue >= m_RamBanks.size()) {
        return 0xFF;
      }

      return m_RamBanks[m_RamBankValue][addr - 0xA000];
    }

    return m_Data[m_RomBankBase + (addr - 0x4000)];
  }

  void MBC3::Write(uint16_t addr, uint8_t data) {
    if (addr < 0x2000) {
      m_RamEnabled = ((data & 0xF) == 0xA);
      if (!m_RamEnabled) {
        m_RTCBanked = false;
        // EventManager::Dispatcher().trigger(Events::SaveRam{});
      }
    }

    switch (addr & 0xE000) {
      // Rom Bank Number
      case 0x2000:
        if (data == 0) {
          data = 1;
        }

        data &= 0x7F;

        SetRomBank(data);
        break;

        // Ram Bank Number/ RTC Register Select
      case 0x4000:
        if (data >= 0x8 && data <= 0xC) {
          auto &bus = Gameboy::Get();

          bus.Cycles(1);

          m_SelectedField = static_cast<RTCField>(data);
          m_RTCBanked = true;
        } else {
          SetRamBank(data & 0x3);
          m_RTCBanked = false;

          if (m_RamBanking) {
            // cart save here
            /*  if (cart_need_save()) {
                  cart_battery_save();
              }*/
          }
        }
        break;

        // Latch Clock Data
      case 0x6000:
        if (data == 0) {
          m_PrevWriteZero = true;
          return;
        }

        if (data == 1 && m_PrevWriteZero) {
          m_RTC = m_ShadowRTC;
        }

        m_PrevWriteZero = false;
        break;

        // RAM Write
      case 0xA000:
        if (!m_RamEnabled)
          return;

        if (m_RamBanks.size() == 0 || m_RamBankValue >= m_RamBanks.size()) {
          return;
        }

        if (m_RTCBanked) {
          switch (m_SelectedField) {
            case RTCField::Seconds:
              m_ShadowRTC.seconds = data;
              return;
            case RTCField::Minutes:
              m_ShadowRTC.minutes = data;
              return;
            case RTCField::Hours:
              m_ShadowRTC.hours = data;
              return;
            case RTCField::DayLow: {
              uint16_t day = m_ShadowRTC.day & 0xFF00;
              m_ShadowRTC.day = day | data;
            }
              return;
            case RTCField::DayHigh: {
              uint16_t day = m_ShadowRTC.day & 0xFF;
              m_ShadowRTC.day = (data << 8) | day;

              if (Bit(data, 6)) {
                m_TimerHalted = true;
              } else {
                m_TimerHalted = false;
              }
            }
              return;
          }
        }

        m_RamBanks[m_RamBankValue][addr - 0xA000] = data;

        if (m_HasBattery) {
          SaveRam();
        }
        break;
    }
  }

  void MBC3::SetRomBanks(uint16_t bankCount) {
    m_RomBankCount = bankCount;
    SetRomBank(0);
  }

  void MBC3::SetRamBanks(uint8_t bankCount) {
    m_RamBanks.clear();
    m_RamBankCount = bankCount;

    if (m_RamBankCount > 0) {
      for (uint8_t i = 0; i < m_RamBankCount; i++) {
        std::vector<uint8_t> bank(0x2000);
        m_RamBanks.push_back(bank);
      }
    }

    LoadRam();
  }

  std::vector<Mapper::StatLine> MBC3::GetStats() {
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

    lines.push_back({"Shadow RTC - Seconds", fmt::format("{}", m_ShadowRTC.seconds)});
    lines.push_back({"Shadow RTC - Minutes", fmt::format("{}", m_ShadowRTC.minutes)});
    lines.push_back({"Shadow RTC - Hours", fmt::format("{}", m_ShadowRTC.hours)});
    lines.push_back({"Shadow RTC - Days", fmt::format("{}", m_ShadowRTC.day & 0x1FF)});
    lines.push_back({"Shadow RTC - Halted",
                     fmt::format("{}",
                                 Bit((m_ShadowRTC.day & 0xFF00) >> 8, 6) ? "Yes" : "No")});
    lines.push_back({"Shadow RTC - Day Carry Set",
                     fmt::format("{}",
                                 Bit((m_ShadowRTC.day & 0xFF00) >> 8, 7) ? "Yes" : "No")});

    lines.push_back({"RTC - Seconds", fmt::format("{}", m_RTC.seconds)});
    lines.push_back({"RTC - Minutes", fmt::format("{}", m_RTC.minutes)});
    lines.push_back({"RTC - Hours", fmt::format("{}", m_RTC.hours)});
    lines.push_back({"RTC - Days", fmt::format("{}", m_RTC.day & 0x1FF)});
    lines.push_back({"RTC - Halted",
                     fmt::format("{}",
                                 Bit((m_RTC.day & 0xFF00) >> 8, 6) ? "Yes" : "No")});
    lines.push_back({"RTC - Day Carry Set",
                     fmt::format("{}",
                                 Bit((m_RTC.day & 0xFF00) >> 8, 7) ? "Yes" : "No")});


    return lines;
  }

  void MBC3::Tick(double timestep) {
    if (m_TimerHalted)
      return;

    m_Delta += timestep;

    if (m_Delta > 1.0) {
      m_ShadowRTC.seconds++;

      if (m_ShadowRTC.seconds == 60) {
        m_ShadowRTC.seconds = 0;
        m_ShadowRTC.minutes++;

        if (m_ShadowRTC.minutes == 60) {
          m_ShadowRTC.minutes = 0;
          m_ShadowRTC.hours++;

          if (m_ShadowRTC.hours == 24) {
            m_ShadowRTC.hours = 0;

            uint16_t controlBits = m_ShadowRTC.day & 0xC000;
            uint16_t day = m_ShadowRTC.day & 0x1FF;

            day = day + 1;

            if (day == 512) {
              controlBits |= 0x8000; // Set Day Counter Carry
              day = 0;
            }

            m_ShadowRTC.day = controlBits | (day & 0x1FF);
          }
        }
      }

      m_Delta = 0;
    }
  }

  void MBC3::SetRomBank(uint8_t value) {
    m_RomBankValue = value % m_RomBankCount;
    m_RomBankBase = 0x4000 * value;
  }

  void MBC3::SetRamBank(uint8_t value) {
    m_RamBankValue = value;
  }

  void MBC3::SaveRam() {
    if (m_RamBankCount == 0)
      return;

    std::ofstream ramFile(fmt::format("{}.sav", path), std::ios::out | std::ios::binary);

    if (!ramFile) {
      spdlog::get("console")->warn("Couldn't open save file for saving!");
      return;
    }

    for (auto n = 0; n < m_RamBankCount; n++) {
      auto &bank = m_RamBanks[n];
      ramFile.write(reinterpret_cast<const char *>(&bank[0]), 0x2000);
    }

    ramFile.close();
  }

  void MBC3::LoadRam() {
    if (m_RamBankCount == 0)
      return;

    std::ifstream ramFile(fmt::format("{}.sav", path), std::ios::binary);

    if (!ramFile) {
      spdlog::get("console")->warn("Couldn't open save file for loading!");
      return;
    }

    for (auto n = 0; n < m_RamBankCount; n++) {
      std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(ramFile), {});
      m_RamBanks[n] = buffer;
    }

    ramFile.close();
  }
} // hijo