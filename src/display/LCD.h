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
    bool LCDC_BGWEnabled();

    bool LCDC_OBJEnabled();

    uint8_t LCDC_ObjHeight();

    uint16_t LCDC_BGMapArea();

    uint16_t LCDC_BGWDataArea();

    bool LCDC_WinEnable();

    uint16_t LCDC_WinMapArea();

    bool LCDC_Enabled();

  public:
    Mode LCDS_Mode();

    void LCDS_SetMode(const Mode &mode);

    bool LCDS_LYC();

    void LCDS_LYCSet(bool isSet);

  public:
    uint8_t LCDS_StatInt(StatSrc src);

  public:
    void Reset();
    
  private:
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

