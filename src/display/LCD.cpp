#include "LCD.h"

#include "common/common.h"
#include "system/Gameboy.h"

namespace hijo {
  LCD::LCD() {
    Reset();
  }

  bool LCD::LCDC_BGWEnabled() {
    return BIT(regs.LCDC, 0);
  }

  bool LCD::LCDC_OBJEnabled() {
    return BIT(regs.LCDC, 1);
  }

  uint8_t LCD::LCDC_ObjHeight() {
    return BIT(regs.LCDC, 2) ? 16 : 8;
  }

  uint16_t LCD::LCDC_BGMapArea() {
    return BIT(regs.LCDC, 3) ? 0x9C00 : 0x9800;
  }

  uint16_t LCD::LCDC_BGWDataArea() {
    return BIT(regs.LCDC, 4) ? 0x8000 : 0x8800;
  }

  bool LCD::LCDC_WinEnable() {
    return BIT(regs.LCDC, 5);
  }

  uint16_t LCD::LCDC_WinMapArea() {
    return BIT(regs.LCDC, 6) ? 0x9C00 : 0x9800;
  }

  bool LCD::LCDC_Enabled() {
    return BIT(regs.LCDC, 7);
  }

  LCD::Mode LCD::LCDS_Mode() {
    return static_cast<Mode>(regs.LCDS & 0x3);
  }

  void LCD::LCDS_SetMode(const LCD::Mode &mode) {
    regs.LCDS &= ~0x3;
    regs.LCDS |= static_cast<uint8_t>(mode);
  }

  bool LCD::LCDS_LYC() {
    return BIT(regs.LCDS, 2);
  }

  void LCD::LCDS_LYCSet(bool isSet) {
    BIT_SET(regs.LCDS, 2, isSet);
  }

  uint8_t LCD::LCDS_StatInt(StatSrc src) {
    return regs.LCDS & static_cast<uint8_t>(src);
  }

  uint8_t LCD::Read(uint16_t addr) {
    switch (addr) {
      case 0xFF40:
        return regs.LCDC;

      case 0xFF41:
        return regs.LCDS;

      case 0xFF42:
        return regs.SCRY;

      case 0xFF43:
        return regs.SCRX;

      case 0xFF44:
        return regs.LY;

      case 0xFF45:
        return regs.LYCP;

      case 0xFF46:
        return regs.DMA;

      case 0xFF47:
        return regs.BG_PALETTE;

      case 0xFF48:
        return regs.OBJ_PALETTE[0];

      case 0xFF49:
        return regs.OBJ_PALETTE[1];

      case 0xFF4A:
        return regs.WINY;

      case 0xFF4B:
        return regs.WINX;

      default:
        return 0; // Notify on this
    }
  }

  void LCD::Write(uint8_t addr, uint8_t data) {
    switch (addr) {
      case 0xFF40:
        regs.LCDC = data;
        break;

      case 0xFF41:
        regs.LCDS = data;
        break;

      case 0xFF42:
        regs.SCRY = data;
        break;

      case 0xFF43:
        regs.SCRX = data;
        break;

      case 0xFF44:
        regs.LY = data;
        break;

      case 0xFF45:
        regs.LYCP = data;
        break;

      case 0xFF46: {
        auto &bus = Gameboy::Get();
        auto &dma = bus.m_DMA;
        dma.Start(data);
      }
        break;

      case 0xFF47:
        PaletteUpdate(data, 0);
        break;

      case 0xFF48:
        PaletteUpdate(data & 0xFC, 1);
        break;

      case 0xFF49:
        PaletteUpdate(data & 0xFC, 1);
        break;

      case 0xFF4A:
        regs.WINY = data;
        break;

      case 0xFF4B:
        regs.WINX = data;
        break;

      default:
        break; // Notify on this
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