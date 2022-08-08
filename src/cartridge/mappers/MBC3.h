#pragma once

#include "Mapper.h"

namespace hijo {

  class MBC3 : public Mapper {
  public:
    enum class RTCField {
      Seconds = 0x8,
      Minutes,
      Hours,
      DayLow,
      DayHigh
    };

    struct RTC {
      uint8_t seconds;
      uint8_t minutes;
      uint8_t hours;
      uint16_t day;
    };

  public:
    MBC3(const std::string &path) : Mapper(path) {}

    uint8_t Read(uint16_t addr) override;

    void Write(uint16_t addr, uint8_t data) override;

    void SetRomBanks(uint16_t uint16) override;

    void SetRamBanks(uint8_t uint8) override;

    std::vector<StatLine> GetStats() override;

    void Tick(double timestep) override;

  private:
    void SetRomBank(uint8_t value);

    void SetRamBank(uint8_t value);

    void SaveRam();

    void LoadRam();

  private:
    std::vector<std::vector<uint8_t>> m_RamBanks;

    RTC m_RTC;
    RTC m_ShadowRTC;

    uint16_t m_RomBankCount;
    uint8_t m_RamBankCount;

    bool m_RamEnabled = false;
    bool m_RamBanking = false;

    uint8_t m_BankingMode = 0;

    uint8_t m_RomBankValue = 0;
    uint8_t m_RamBankValue = 0;

    uint32_t m_RomBankBase = 0;

    bool m_NeedsSave = false;
    bool m_PrevWriteZero = false;

    bool m_RTCBanked = false;

    RTCField m_SelectedField = RTCField::Seconds;

    double m_Delta = 0;

    bool m_TimerHalted = false;
  };

} // hijo
