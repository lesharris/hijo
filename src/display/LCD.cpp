#include "LCD.h"

#include "common/common.h"
#include "system/Gameboy.h"

namespace hijo {
  LCD::LCD() {
    Reset();
  }

  bool LCD::LCDC_BGWEnabled() {
    return Bit(regs.LCDC, 0);
  }

  bool LCD::LCDC_OBJEnabled() {
    return Bit(regs.LCDC, 1);
  }

  uint8_t LCD::LCDC_ObjHeight() {
    return Bit(regs.LCDC, 2) ? 16 : 8;
  }

  uint16_t LCD::LCDC_BGTilemapArea() {
    return Bit(regs.LCDC, 3) ? 0x9C00 : 0x9800;
  }

  uint16_t LCD::LCDC_BGWTileDataArea() {
    return Bit(regs.LCDC, 4) ? 0x8000 : 0x8800;
  }

  bool LCD::LCDC_WinEnable() {
    return Bit(regs.LCDC, 5);
  }

  uint16_t LCD::LCDC_WindowTilemapArea() {
    return Bit(regs.LCDC, 6) ? 0x9C00 : 0x9800;
  }

  bool LCD::LCDC_Enabled() {
    return Bit(regs.LCDC, 7);
  }

  LCD::Mode LCD::LCDS_Mode() {
    return static_cast<Mode>(regs.LCDS & 0x3);
  }

  void LCD::LCDS_SetMode(const LCD::Mode &mode) {
    regs.LCDS &= ~0x3;
    regs.LCDS |= static_cast<uint8_t>(mode);
  }

  bool LCD::LCDS_LYC() {
    return Bit(regs.LCDS, 2);
  }

  void LCD::LCDS_LYCSet(bool isSet) {
    SetBit(regs.LCDS, 2, isSet);
  }

  uint8_t LCD::LCDS_StatInt(StatSrc src) {
    return regs.LCDS & static_cast<uint8_t>(src);
  }

  uint8_t LCD::Read(uint16_t addr) {
    uint8_t offset = (addr - 0xFF40);
    uint8_t *p = (uint8_t *) &regs;

    return p[offset];
  }

  void LCD::Write(uint16_t addr, uint8_t data) {
    uint8_t offset = (addr - 0xFF40);
    uint8_t *p = (uint8_t *) &regs;
    p[offset] = data;

    if (offset == 6) {
      auto &bus = Gameboy::Get();
      bus.m_DMA.Start(data);
    }

    if (addr == 0xFF47) {
      PaletteUpdate(data, 0);
    } else if (addr == 0xFF48) {
      PaletteUpdate(data & 0b11111100, 1);
    } else if (addr == 0xFF49) {
      PaletteUpdate(data & 0b11111100, 2);
    }
  }

  void LCD::PaletteUpdate(uint8_t data, uint8_t palette) {
    switch (palette) {
      case 0:
        regs.bgColors[0] = m_DefaultColors[data & 0x3];
        regs.bgColors[1] = m_DefaultColors[(data >> 2) & 0x3];
        regs.bgColors[2] = m_DefaultColors[(data >> 4) & 0x3];
        regs.bgColors[3] = m_DefaultColors[(data >> 6) & 0x3];
        break;

      case 1:
        regs.sp1Colors[0] = m_DefaultColors[data & 0x3];
        regs.sp1Colors[1] = m_DefaultColors[(data >> 2) & 0x3];
        regs.sp1Colors[2] = m_DefaultColors[(data >> 4) & 0x3];
        regs.sp1Colors[3] = m_DefaultColors[(data >> 6) & 0x3];
        break;

      case 2:
        regs.sp2Colors[0] = m_DefaultColors[data & 0x3];
        regs.sp2Colors[1] = m_DefaultColors[(data >> 2) & 0x3];
        regs.sp2Colors[2] = m_DefaultColors[(data >> 4) & 0x3];
        regs.sp2Colors[3] = m_DefaultColors[(data >> 6) & 0x3];
        break;

      default:
        break; // notify
    }
  }

  LCD::Registers &LCD::Regs() {
    return regs;
  }

  void LCD::Reset() {
    regs.LCDC = 0x91;
    regs.SCRX = 0;
    regs.SCRY = 0;
    regs.LY = 0;
    regs.LYCP = 0;
    regs.BG_PALETTE = 0xFC;
    regs.OBJ_PALETTE[0] = 0xFF;
    regs.OBJ_PALETTE[1] = 0xFF;
    regs.WINX = 0;
    regs.WINY = 0;

    regs.bgColors = m_DefaultColors;
    regs.sp1Colors = m_DefaultColors;
    regs.sp2Colors = m_DefaultColors;
  }
} // hijo