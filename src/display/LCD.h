#pragma once

#include <raylib.h>
#include <cstdint>
#include <vector>

namespace hijo {

  class LCD {
  public:
    struct Registers {
      uint8_t LCDC;
      uint8_t LCDS;
      uint8_t SCRY;
      uint8_t SCRX;
      uint8_t LY;
      uint8_t LYCP;
      uint8_t DMA;
      uint8_t BG_PALETTE;
      uint8_t OBJ_PALETTE[2];
      uint8_t WINY;
      uint8_t WINX;

      // Colors
      std::vector<Color> bgColors;
      std::vector<Color> sp1Colors;
      std::vector<Color> sp2Colors;
    };

    enum class Mode {
      HBlank = 0,
      VBlank,
      OAM,
      XFER
    };

    enum class StatSrc {
      HBlank = 1 << 3,
      VBlank = 1 << 4,
      OAM = 1 << 5,
      LYC = 1 << 6
    };

  public:
    static LCD &Get() {
      static LCD instance;

      return instance;
    }

  private:
    LCD();

  public:
    uint8_t Read(uint16_t addr);

    void Write(uint8_t addr, uint8_t data);

    Registers &Regs();

  public:
    bool LCDCEnableBGW();

    bool LCDCEnableOBJ();

    uint8_t LCDCObjHeight();

    uint16_t LCDCBGMapArea();

    uint16_t LCDBGWDataArea();

    bool LCDCWinEnable();

    uint16_t LCDCWinMapArea();

    bool LCDCEnable();

  public:
    Mode LCDSMode();

    void LCDSSetMode(const Mode &mode);

    bool LCDSLYC();

    void LCDSLYCSet(bool isSet);

  public:
    uint8_t LCDSStatInt(StatSrc src);

  public:
    void Reset();

  private:
    uint8_t bitmasks[8] = {
        0b00000001,
        0b00000010,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b10000000
    };

  private:
    bool GetLCDCBit(uint8_t bit);

    bool GetLCDSBit(uint8_t bit);

    void SetLCDSBit(uint8_t bit, bool isSet);

    void PaletteUpdate(uint8_t data, uint8_t palette);

  private:
    Registers regs;

    std::vector<Color> m_DefaultColors{
        WHITE,
        {0xAA, 0xAA, 0xAA, 0xFF},
        {0x55, 0x55, 0x55, 0xFF},
        BLACK
    };
  };
} // hijo

