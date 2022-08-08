#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace hijo {

  class Mapper {
  public:
    struct StatLine {
      std::string label;
      std::string data;
    };

  public:
    enum class Type {
      ROM,
      MBC1,
      MBC2,
      MBC3,
      MBC5,
      MBC6,
      MBC7,
      MMM01,
      M161,
      HuC1,
      HuC3,
      OTHER,
      NONE
    };

  public:
    virtual ~Mapper() = default;

    virtual uint8_t Read(uint16_t addr) = 0;

    virtual void Write(uint16_t addr, uint8_t data) = 0;

    virtual void SetRomData(std::vector<uint8_t> &data) {
      m_Data = data;
    }

    virtual void SetRomBanks(uint16_t) {};

    virtual void SetRamBanks(uint8_t) {}

    virtual std::vector<StatLine> GetStats() = 0;

    virtual void Tick() {}

    void SetFeatures(bool ram, bool battery, bool timer, bool rumble) {
      m_HasRam = ram;
      m_HasBattery = battery;
      m_HasTimer = timer;
      m_HasRumble = rumble;
    }

  protected:
    std::vector<uint8_t> m_Data;

    bool m_HasRam = false;
    bool m_HasBattery = false;
    bool m_HasTimer = false;
    bool m_HasRumble = false;
  };

} // hijo

