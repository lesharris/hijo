#pragma once

#include <cstdint>
#include <vector>
#include <deque>

#include <raylib.h>

#include "common/common.h"

namespace hijo {

  class Display {
  public:
    enum class LCDControlFlag {
      BackgroundWindowEnablePrio = 0,
      OBJEnable,
      OBJSize,
      BackgroundTileMapArea,
      BackgroundWindowTileDataArea,
      WindowEnable,
      WindowTileMapArea,
      LCDPPUEnable
    };

    enum class LCDStatusFlag {
      LYCisLY = 2,
      Mode0HBlankIntr,
      Mode1VBlankIntr,
      Mode2OAMIntr,
      LYCisLYInter
    };

    enum class Mode {
      HBlank = 0,
      VBlank,
      OAM,
      Transfer
    };

    enum class OAMAttribute {
      PaletteNumber = 4,
      XFlip,
      YFlip,
      BackgroundPriority
    };

    enum class FifoState {
      Tile = 0,
      Data0,
      Data1,
      Idle,
      Push
    };

  public:
    struct OAMEntry {
      uint8_t y;
      uint8_t x;
      uint8_t tile;
      uint8_t flags;
      bool fetched;
    };

  public:
    Display();

    void Reset();

    void Tick(uint8_t cycles);

    uint8_t vramRead(uint16_t addr);

    void vramWrite(uint16_t addr, uint8_t data);

    uint8_t oamRead(uint16_t addr);

    void oamWrite(uint16_t addr, uint8_t data);

    uint8_t lcdRead(uint16_t addr);

    void lcdWrite(uint16_t addr, uint8_t data);

  private:
    void HBlankMode();

    void VBlankMode();

    void OAMMode();

    void TransferMode();

  private:
    void UpdatePalette(uint8_t data, uint8_t palette);

    uint16_t TileStartAddrByIndex(uint8_t tileIndex);

    void SetLY(uint8_t value);

    void RequestInterrupt(uint8_t interrupt);

  private:
    /* RAM */
    uint8_t m_VramBank = 0;
    bool m_VramBlocked = false;
    std::vector<std::vector<uint8_t>> m_VRam;
    std::vector<OAMEntry> m_OAM = std::vector<OAMEntry>(40);

    /* LCD */
    uint8_t m_LCDControl = 0;
    uint8_t m_LCDStatus = 0;

    /* PPU State */
    Mode m_Mode;
    uint8_t m_CurrentScanline = 0;
    uint16_t m_CurrentLineTick = 0;
    uint8_t m_CurrentDot = 0;

    uint8_t m_ScrollY = 0;
    uint8_t m_ScrollX = 0;

    uint8_t m_LY = 0;
    uint8_t m_LYC = 0;

    uint8_t m_WindowY = 0;
    uint8_t m_WindowX = 7;

    uint8_t m_WindowCounter = 0;
    bool m_WindowYTriggered = false;
    bool m_WindowXTriggered = false;

    uint8_t m_BGPalette = 0;
    uint8_t m_OBJPalette[2] = {0, 0};

    /* Pixel FIFO */
    FifoState m_BackgroundFifoState;
    FifoState m_OBJFifoState;

    std::deque<Color> m_BackgroundFifo;
    std::deque<Color> m_OBJFifo;

    /* Colors */
    std::vector<Color> bgColors;
    std::vector<Color> sp1Colors;
    std::vector<Color> sp2Colors;

    std::vector<Color> m_DefaultColors{
        WHITE,
        {0xAA, 0xAA, 0xAA, 0xFF},
        {0x55, 0x55, 0x55, 0xFF},
        BLACK
    };

    /* Output */
    std::vector<Color> m_Buffer;

    /* General */
    static constexpr uint8_t m_Height = 144;
    static constexpr uint8_t m_Width = 160;
    static constexpr uint16_t m_DotsPerLine = 456;
    static constexpr uint8_t m_VBlankStart = 144;
    static constexpr uint8_t m_MaxScanlines = 153;

    uint8_t m_DMA = 0;
  };

} // hijo
