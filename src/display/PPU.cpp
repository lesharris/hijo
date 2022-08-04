
#include "PPU.h"
#include "LCD.h"
#include "system/Gameboy.h"
#include "cpu/Interrupts.h"
#include <spdlog/spdlog.h>

namespace hijo {
  PPU::PPU() {
    Init();
  }

  void PPU::Init() {
    auto &lcd = LCD::Get();

    currentFrame = 0;
    lineTicks = 0;
    videoBuffer.clear();
    videoBuffer.reserve(m_YRes * m_XRes);

    fifo.lineX = 0;
    fifo.pushedX = 0;
    fifo.fetchX = 0;
    fifo.pixelFifo.size = 0;
    fifo.pixelFifo.head = nullptr;
    fifo.pixelFifo.tail = nullptr;
    fifo.state = FetchState::Tile;

    lineSprites = nullptr;
    fetchedEntryCount = 0;
    windowLine = 0;

    lcd.Reset();
    lcd.LCDS_SetMode(LCD::Mode::OAM);

    memset(m_VideoRam, 0, 1024 * 8);
    memset(m_OAMRam, 0, sizeof(m_OAMRam));

    for (auto n = 0; n < m_YRes * m_XRes; n++) {
      videoBuffer.push_back({0, 0, 0, 0xFF});
    }
  }

  void PPU::Tick() {
    auto &lcd = LCD::Get();

    lineTicks++;

    switch (lcd.LCDS_Mode()) {
      case LCD::Mode::OAM:
        OAMMode();
        break;
      case LCD::Mode::XFER:
        XFERMode();
        break;
      case LCD::Mode::VBlank:
        VBlankMode();
        break;
      case LCD::Mode::HBlank:
        HBlankMode();
        break;
    }
  }

  void PPU::OAMWrite(uint16_t addr, uint8_t data) {
    if (addr >= 0xFE00)
      addr -= 0xFE00;

    /*spdlog::get("console")->info("A: {:08X} AC: {} | E {} - F {}",
                                 addr,
                                 a,
                                 a / 4,
                                 a % 4);*/


    size_t index = addr / 4;
    uint8_t field = addr % 4;

    switch (field) {
      case 0:
        m_OAMRam[index].y = data;
        break;

      case 1:
        m_OAMRam[index].x = data;
        break;

      case 2:
        m_OAMRam[index].tile = data;
        break;

      case 3:
        m_OAMRam[index].flags = data;
        break;
    }
  }

  uint8_t PPU::OAMRead(uint16_t addr) {
    if (addr >= 0xFE00)
      addr -= 0xFE00;

    size_t index = addr / 4;
    uint8_t field = addr % 4;

    switch (field) {
      case 0:
        return m_OAMRam[index].y;

      case 1:
        return m_OAMRam[index].x;

      case 2:
        return m_OAMRam[index].tile;

      case 3:
        return m_OAMRam[index].flags;
    }

    return 0;
  }

  void PPU::VRAMWrite(uint16_t addr, uint8_t data) {
    m_VideoRam[addr - 0x8000] = data;
  }

  uint8_t PPU::VRAMRead(uint16_t addr) {
    return m_VideoRam[addr - 0x8000];
  }

  bool PPU::WindowVisible() {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();

    return lcd.LCDC_WinEnable() &&
           lcdRegs.WINX >= 0 && lcdRegs.WINX <= 166 &&
           lcdRegs.WINY >= 0 && lcdRegs.WINY < m_YRes;
  }

  void PPU::PixelFifoPush(Color value) {
    FifoEntry *next = new FifoEntry();
    next->next = nullptr;
    next->value = value;

    if (!fifo.pixelFifo.head) {
      fifo.pixelFifo.head = next;
      fifo.pixelFifo.tail = next;
    } else {
      fifo.pixelFifo.tail->next = next;
      fifo.pixelFifo.tail = next;
    }

    fifo.pixelFifo.size++;
  }

  Color PPU::PixelFifoPop() {
    if (fifo.pixelFifo.size <= 0) {
      spdlog::get("console")->warn("Empty Pixel Fifo Popped");
      return BLACK;
    }

    FifoEntry *popped = fifo.pixelFifo.head;

    fifo.pixelFifo.head = popped->next;
    fifo.pixelFifo.size--;

    Color value = popped->value;
    delete popped;

    return value;
  }

  Color PPU::FetchSpritePixels(uint8_t bit, Color color, uint8_t bgColor) {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();

    for (auto i = 0; i < fetchedEntryCount; i++) {
      int32_t spX = (fetchedEntries[i].x - 8) + (lcdRegs.SCRX % 8);

      if (spX + 8 < fifo.fifoX)
        continue;

      int32_t offset = fifo.fifoX - spX;

      if (offset < 0 || offset > 7)
        continue;

      bit = 7 - offset;

      if (OAMEntryXFlip(fetchedEntries[i]))
        bit = offset;

      uint8_t high = !!(fifo.fetchEntryData[i * 2] & (1 << bit));
      uint8_t low = !!(fifo.fetchEntryData[(i * 2) + 1] & (1 << bit)) << 1;

      bool bgPriority = OAMEntryBackgroundPriority(fetchedEntries[i]);

      if (!(high | low))
        continue;

      if (!bgPriority || bgColor == 0) {
        color = OAMEntryPaletteNumber(fetchedEntries[i])
                ? lcdRegs.sp2Colors[high | low]
                : lcdRegs.sp1Colors[high | low];

        if (high | low)
          break;
      }
    }

    return color;
  }

  bool PPU::PipelineFifoAdd() {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();

    if (fifo.pixelFifo.size > 8) {
      return false;
    }

    int32_t x = fifo.fetchX - (8 - (lcdRegs.SCRX % 8));

    for (auto i = 0; i < 8; i++) {
      int bit = 7 - i;

      uint8_t high = !!(fifo.bgwFetchData[1] & (1 << bit));
      uint8_t low = !!(fifo.bgwFetchData[2] & (1 << bit)) << 1;

      auto color = lcdRegs.bgColors[high | low];

      if (!lcd.LCDC_BGWEnabled()) {
        color = lcdRegs.bgColors[0];
      }

      if (lcd.LCDC_OBJEnabled()) {
        color = FetchSpritePixels(bit, color, high | low);
      }

      if (x >= 0) {
        PixelFifoPush(color);
        fifo.fifoX++;
      }
    }

    return true;
  }

  void PPU::PipelineLoadSpriteTile() {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();

    OAMLineEntry *le = lineSprites;

    while (le) {
      int32_t spX = (le->entry.x - 8) + (lcdRegs.SCRX % 8);

      if ((spX >= fifo.fetchX && spX < fifo.fetchX + 8) ||
          (spX + 8 >= fifo.fetchX && spX + 8 < fifo.fetchX + 8)) {
        fetchedEntries[fetchedEntryCount++] = le->entry;
      }

      le = le->next;

      if (!le || fetchedEntryCount >= 3)
        break;
    }
  }

  void PPU::PipelineLoadSpriteData(uint8_t offset) {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();
    auto &bus = Gameboy::Get();

    int32_t curY = lcdRegs.LY;
    uint8_t sprHeight = lcd.LCDC_ObjHeight();

    for (auto i = 0; i < fetchedEntryCount; i++) {
      uint8_t ty = ((curY + 16) - fetchedEntries[i].y) * 2;

      if (OAMEntryYFlip(fetchedEntries[i]))
        ty = ((sprHeight * 2) - 2) - ty;

      uint8_t tileIndex = fetchedEntries[i].tile;

      if (sprHeight == 16)
        tileIndex &= ~(1);

      fifo.fetchEntryData[(i * 2) + offset] =
          bus.cpuRead(0x8000 + (tileIndex * 16) + ty + offset);
    }
  }

  void PPU::PipelineLoadWindowTile() {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();
    auto &bus = Gameboy::Get();

    if (!WindowVisible())
      return;

    uint8_t winY = lcdRegs.WINY;

    if (fifo.fetchX + 7 >= lcdRegs.WINX &&
        fifo.fetchX + 7 < lcdRegs.WINX + m_YRes + 14) {
      if (lcdRegs.LY >= winY && lcdRegs.LY < winY + m_XRes) {
        uint8_t w_tile_y = windowLine / 8;

        fifo.bgwFetchData[0] = bus.cpuRead(
            (lcd.LCDC_WinMapArea() + (fifo.fetchX + 7 - lcdRegs.WINX) / 8) + (w_tile_y * 32));

        if (lcd.LCDC_BGWDataArea() == 0x8800)
          fifo.bgwFetchData[0] += 128;
      }
    }
  }

  void PPU::PipelineFetch() {
    auto &lcd = LCD::Get();
    auto &bus = Gameboy::Get();

    switch (fifo.state) {
      case FetchState::Tile: {
        fetchedEntryCount = 0;

        if (lcd.LCDC_BGWEnabled()) {
          fifo.bgwFetchData[0] = bus.cpuRead(lcd.LCDC_BGMapArea() + (fifo.mapX / 8) + ((fifo.mapY / 8) * 32));

          if (lcd.LCDC_BGWDataArea() == 0x8800) {
            fifo.bgwFetchData[0] += 128;
          }

          PipelineLoadWindowTile();
        }

        if (lcd.LCDC_OBJEnabled() && lineSprites) {
          PipelineLoadSpriteTile();
        }

        fifo.state = FetchState::Data0;
        fifo.fetchX += 8;
      }
        break;
      case FetchState::Data0: {
        fifo.bgwFetchData[1] = bus.cpuRead(lcd.LCDC_BGWDataArea() + (fifo.bgwFetchData[0] * 16) + fifo.tileY);

        PipelineLoadSpriteData(0);

        fifo.state = FetchState::Data1;
      }
        break;
      case FetchState::Data1: {
        fifo.bgwFetchData[2] = bus.cpuRead(lcd.LCDC_BGWDataArea() + (fifo.bgwFetchData[0] * 16) + fifo.tileY + 1);

        PipelineLoadSpriteData(1);

        fifo.state = FetchState::Idle;
      }
        break;
      case FetchState::Idle:
        fifo.state = FetchState::Push;
        break;
      case FetchState::Push:
        if (PipelineFifoAdd())
          fifo.state = FetchState::Tile;
        break;
    }
  }

  void PPU::PipelinePushPixel() {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();

    if (fifo.pixelFifo.size > 8) {
      Color pixData = PixelFifoPop();

      if (fifo.lineX >= (lcdRegs.SCRX % 8)) {
        videoBuffer[fifo.pushedX + (lcdRegs.LY * m_XRes)] = pixData;

        fifo.pushedX++;
      }

      fifo.lineX++;
    }
  }

  void PPU::PipelineProcess() {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();

    fifo.mapY = lcdRegs.LY + lcdRegs.SCRY;
    fifo.mapX = fifo.fetchX + lcdRegs.SCRX;
    fifo.tileY = ((lcdRegs.LY + lcdRegs.SCRY) % 8) * 2;

    if (!(lineTicks & 1))
      PipelineFetch();

    PipelinePushPixel();
  }

  void PPU::PipelineFifoReset() {
    while (fifo.pixelFifo.size)
      PixelFifoPop();

    fifo.pixelFifo.head = nullptr;
  }

  void PPU::OAMMode() {
    auto &lcd = LCD::Get();

    if (lineTicks >= 80) {
      lcd.LCDS_SetMode(LCD::Mode::XFER);

      fifo.state = FetchState::Tile;
      fifo.lineX = 0;
      fifo.fetchX = 0;
      fifo.pushedX = 0;
      fifo.fifoX = 0;
    }

    if (lineTicks == 1) {
      lineSprites = nullptr;
      lineSpriteCount = 0;

      LoadLineSprites();
    }
  }

  void PPU::XFERMode() {
    auto &lcd = LCD::Get();
    auto &bus = Gameboy::Get();

    PipelineProcess();

    if (fifo.pushedX >= m_XRes) {
      PipelineFifoReset();

      lcd.LCDS_SetMode(LCD::Mode::HBlank);

      if (lcd.LCDS_StatInt(LCD::StatSrc::HBlank)) {
        Interrupts::RequestInterrupt(bus.m_Cpu, Interrupts::Interrupt::LCDStat);
      }
    }
  }

  void PPU::HBlankMode() {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();
    auto &bus = Gameboy::Get();

    if (lineTicks >= m_TicksPerLine) {
      IncrementLY();

      if (lcdRegs.LY >= m_YRes) {
        lcd.LCDS_SetMode(LCD::Mode::VBlank);
        Interrupts::RequestInterrupt(bus.m_Cpu, Interrupts::Interrupt::VBlank);

        if (lcd.LCDS_StatInt(LCD::StatSrc::VBlank)) {
          Interrupts::RequestInterrupt(bus.m_Cpu, Interrupts::Interrupt::LCDStat);
        }

        currentFrame++;

        // Cart save here
      } else {
        lcd.LCDS_SetMode(LCD::Mode::OAM);
      }

      lineTicks = 0;
    }
  }

  void PPU::VBlankMode() {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();
    if (lineTicks >= m_TicksPerLine) {
      IncrementLY();

      if (lcdRegs.LY >= m_LinesPerFrame) {
        lcd.LCDS_SetMode(LCD::Mode::OAM);
        lcdRegs.LY = 0;
        windowLine = 0;
      }

      lineTicks = 0;
    }
  }

  void PPU::IncrementLY() {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();
    auto &bus = Gameboy::Get();

    if (WindowVisible() && lcdRegs.LY >= lcdRegs.WINY &&
        lcdRegs.LY < lcdRegs.WINY + m_YRes) {
      windowLine++;
    }

    lcdRegs.LY++;

    if (lcdRegs.LY == lcdRegs.LYCP) {
      lcd.LCDS_LYCSet(true);

      if (lcd.LCDS_StatInt(LCD::StatSrc::LYC)) {
        Interrupts::RequestInterrupt(bus.m_Cpu, Interrupts::Interrupt::LCDStat);
      }

    } else {
      lcd.LCDS_LYCSet(false);
    }
  }

  void PPU::LoadLineSprites() {
    auto &lcd = LCD::Get();
    auto &lcdRegs = lcd.Regs();

    int32_t curY = lcdRegs.LY;
    uint8_t sprHeight = lcd.LCDC_ObjHeight();
    memset(lineEntries, 0, sizeof(lineEntries));

    for (auto i = 0; i < 40; i++) {
      OAMEntry e = m_OAMRam[i];

      if (!e.x)
        continue;

      if (lineSpriteCount >= 10)
        break;

      if (e.y <= curY + 16 && e.y + sprHeight > curY + 16) {
        OAMLineEntry *entry = &lineEntries[lineSpriteCount++];

        entry->entry = e;
        entry->next = nullptr;

        if (!lineSprites || lineSprites->entry.x > e.x) {
          entry->next = lineSprites;
          lineSprites = entry;
          continue;
        }

        OAMLineEntry *le = lineSprites;
        OAMLineEntry *prev = le;

        while (le) {
          if (le->entry.x > e.x) {
            prev->next = entry;
            entry->next = le;
            break;
          }

          if (!le->next) {
            le->next = entry;
            break;
          }

          prev = le;
          le = le->next;
        }
      }
    }
  }
} // hijo