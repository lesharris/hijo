#pragma once

#include <raylib.h>
#include <cstdint>
#include <vector>

#include "common/common.h"

namespace hijo {

  class PPU {
  public:
    enum class FetchState {
      Tile = 0,
      Data0,
      Data1,
      Idle,
      Push
    };

    struct FifoEntry {
      FifoEntry *next;
      Color value;
    };

    struct Fifo {
      FifoEntry *head;
      FifoEntry *tail;
      uint32_t size;
    };

    struct PixelFifo {
      FetchState state;
      Fifo pixelFifo;
      uint8_t lineX;
      uint8_t pushedX;
      uint8_t fetchX;
      uint8_t bgwFetchData[3];
      uint8_t fetchEntryData[6]; // OAM
      uint8_t mapY;
      uint8_t mapX;
      uint8_t tileY;
      uint8_t fifoX;
    };

    struct OAMEntry {
      uint8_t y;
      uint8_t x;
      uint8_t tile;
      uint8_t flags;
    };

    static uint8_t OAMEntryCGBPalette(const OAMEntry &entry) {
      return entry.flags & 0x3;
    };

    static bool OAMEntryCGBVramBank(const OAMEntry &entry) {
      return BIT(entry.flags, 3);
    }

    static bool OAMEntryPaletteNumber(const OAMEntry &entry) {
      return BIT(entry.flags, 4);
    }

    static bool OAMEntryXFlip(const OAMEntry &entry) {
      return BIT(entry.flags, 5);
    }

    static bool OAMEntryYFlip(const OAMEntry &entry) {
      return BIT(entry.flags, 6);
    }

    static bool OAMEntryBackgroundPriority(const OAMEntry &entry) {
      return BIT(entry.flags, 7);
    }

    /*
 Bit7   BG and Window over OBJ (0=No, 1=BG and Window colors 1-3 over the OBJ)
 Bit6   Y flip          (0=Normal, 1=Vertically mirrored)
 Bit5   X flip          (0=Normal, 1=Horizontally mirrored)
 Bit4   Palette number  **Non CGB Mode Only** (0=OBP0, 1=OBP1)
 Bit3   Tile VRAM-Bank  **CGB Mode Only**     (0=Bank 0, 1=Bank 1)
 Bit2-0 Palette number  **CGB Mode Only**     (OBP0-7)
 */

    struct OAMLineEntry {
      OAMEntry entry;
      OAMLineEntry *next;
    };

  public:
    PPU();

  public:
    void Init();
    
    void Tick();

    void OAMWrite(uint16_t addr, uint8_t data);

    uint8_t OAMRead(uint16_t addr);

    void VRAMWrite(uint16_t addr, uint8_t data);

    uint8_t VRAMRead(uint16_t addr);

    std::vector<Color> &VideoBuffer() {
      return videoBuffer;
    }

  private:
    bool WindowVisible();

    void PixelFifoPush(Color value);

    Color PixelFifoPop();

    Color FetchSpritePixels(uint8_t bit, Color color, uint8_t bgColor);

    bool PipelineFifoAdd();

    void PipelineLoadSpriteTile();

    void PipelineLoadSpriteData(uint8_t offset);

    void PipelineLoadWindowTile();

    void PipelineFetch();

    void PipelinePushPixel();

    void PipelineProcess();

    void PipelineFifoReset();

  private:
    void OAMMode();

    void XFERMode();

    void HBlankMode();

    void VBlankMode();

    void IncrementLY();

    void LoadLineSprites();

  private:
    friend class UI;

  private:
    PixelFifo fifo;

    uint8_t lineSpriteCount;
    OAMLineEntry *lineSprites;
    OAMLineEntry lineEntries[10];

    uint8_t fetchedEntryCount;
    OAMEntry fetchedEntries[3];
    uint8_t windowLine;

    uint32_t currentFrame;
    uint32_t lineTicks;
    std::vector<Color> videoBuffer;

    OAMEntry m_OAMRam[40];
    uint8_t m_VideoRam[1024 * 8];

    const uint16_t m_LinesPerFrame = 154;
    const uint16_t m_TicksPerLine = 456;
    const uint8_t m_YRes = 144;
    const uint8_t m_XRes = 160;
  };

} // hijo

