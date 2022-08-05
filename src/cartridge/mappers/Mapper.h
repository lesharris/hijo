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

    virtual void SetRomData(std::vector<uint8_t> &data) = 0;

    virtual void SetRomBanks(uint16_t bankCount) = 0;

    virtual void SetRamBanks(uint8_t bankCount) = 0;

    virtual std::vector<StatLine> GetStats() = 0;
    
    void SetFeatures(bool ram, bool battery, bool timer, bool rumble) {
      m_HasRam = ram;
      m_HasBattery = battery;
      m_HasTimer = timer;
      m_HasRumble = rumble;
    }

  protected:
    bool m_HasRam = false;
    bool m_HasBattery = false;
    bool m_HasTimer = false;
    bool m_HasRumble = false;
  };

} // hijo

