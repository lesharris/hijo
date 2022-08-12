
#include "Display.h"

#include "system/Gameboy.h"
#include "cpu/Interrupts.h"

namespace hijo {
  hijo::Display::Display() {
    Reset();
  }

  void Display::Reset() {
    m_VRam.clear();
    m_VRam.push_back(std::vector<uint8_t>(0x2000));
    m_VRam.push_back(std::vector<uint8_t>(0x2000));

    m_OAM = std::vector<OAMEntry>(40);

    m_BackgroundFifo.clear();
    m_OBJFifo.clear();
    m_BackgroundFifoState = FifoState::Tile;
    m_OBJFifoState = FifoState::Tile;

    m_LCDControl = 0x91;
    m_ScrollX = 0;
    m_ScrollY = 0;

    m_LY = 0;
    m_LYC = 0;

    m_BGPalette = 0xFC;
    m_OBJPalette[0] = 0xFF;
    m_OBJPalette[1] = 0xFF;

    m_WindowX = 0;
    m_WindowY = 0;

    m_WindowCounter = 0;
    m_CurrentDot = 0;
    m_CurrentLineTick = 0;
    m_CurrentScanline = 0;

    m_WindowXTriggered = false;
    m_WindowYTriggered = false;

    m_VramBank = 0;
    m_VramBlocked = false;

    bgColors = m_DefaultColors;
    sp1Colors = m_DefaultColors;
    sp2Colors = m_DefaultColors;

    m_Mode = Mode::OAM;

    m_Buffer.clear();
    m_Buffer.reserve(m_Width * m_Height);
    for (auto n = 0; n < m_Width * m_Height; n++) {
      m_Buffer.push_back({0, 0, 0, 0xFF});
    }
  }

  void Display::Tick(uint8_t cycles) {
    m_CurrentLineTick += cycles;

    if (Bit(m_LCDControl, (uint8_t) LCDControlFlag::LCDPPUEnable)) {
      switch (m_Mode) {
        case Mode::HBlank:
          HBlankMode();
          break;
        case Mode::VBlank:
          VBlankMode();
          break;
        case Mode::OAM:
          OAMMode();
          break;
        case Mode::Transfer:
          TransferMode();
          break;
      }
    } else {
      m_LY = 0;
      m_LCDStatus = 0;
      m_Mode = Mode::OAM;
    }
  }

  void Display::HBlankMode() {

  }

  void Display::VBlankMode() {

  }

  void Display::OAMMode() {
    if (m_CurrentLineTick > 80) {
      m_Mode = Mode::Transfer;

      /*fifo.state = FetchState::Tile;
      fifo.lineX = 0;
      fifo.fetchX = 0;
      fifo.pushedX = 0;
      fifo.fifoX = 0;*/
    }

    if (m_CurrentLineTick == 1) {
      //  lineSprites = nullptr;
      //lineSpriteCount = 0;

      //LoadLineSprites();
    }
  }

  void Display::TransferMode() {

  }

  uint8_t Display::vramRead(uint16_t addr) {
    if (m_Mode == Mode::Transfer) {
      return 0xFF;
    }

    return m_VRam[m_VramBank][addr - 0x8000];
  }

  void Display::vramWrite(uint16_t addr, uint8_t data) {
    if (m_Mode == Mode::Transfer)
      return;

    m_VRam[m_VramBank][addr - 0x8000] = data;
  }

  uint8_t Display::oamRead(uint16_t addr) {
    if (m_Mode == Mode::OAM || m_Mode == Mode::Transfer)
      return 0xFF;

    if (addr >= 0xFE00)
      addr -= 0xFE00;

    size_t index = addr / 4;
    uint8_t field = addr % 4;

    switch (field) {
      case 0:
        return m_OAM[index].y;

      case 1:
        return m_OAM[index].x;

      case 2:
        return m_OAM[index].tile;

      case 3:
        return m_OAM[index].flags;

      default:
        return 0xFF;
    }
  }

  void Display::oamWrite(uint16_t addr, uint8_t data) {
    if (m_Mode == Mode::OAM || m_Mode == Mode::Transfer)
      return;

    if (addr >= 0xFE00)
      addr -= 0xFE00;

    size_t index = addr / 4;
    uint8_t field = addr % 4;

    switch (field) {
      case 0:
        m_OAM[index].y = data;
        break;

      case 1:
        m_OAM[index].x = data;
        break;

      case 2:
        m_OAM[index].tile = data;
        break;

      case 3:
        m_OAM[index].flags = data;
        break;
    }
  }

  uint8_t Display::lcdRead(uint16_t addr) {
    switch (addr) {
      case 0xFF40:
        return m_LCDControl;

      case 0xFF41:
        return m_LCDStatus;

      case 0xFF42:
        return m_ScrollY;

      case 0xFF43:
        return m_ScrollX;

      case 0xFF44:
        return m_LY;

      case 0xFF45:
        return m_LYC;

      case 0xFF46:
        return m_DMA;

      case 0xFF47:
        return m_BGPalette;

      case 0xFF48:
        return m_OBJPalette[0];

      case 0xFF49:
        return m_OBJPalette[1];

      case 0xFF4A:
        return m_WindowY;

      case 0xFF4B:
        return m_WindowX;

      default:
        return 0xFF;
    }
  }

  void Display::lcdWrite(uint16_t addr, uint8_t data) {
    auto &bus = Gameboy::Get();

    switch (addr) {
      case 0xFF40:
        m_LCDControl = data;
        return;

      case 0xFF41:
        m_LCDStatus = data;
        return;

      case 0xFF42:
        m_ScrollY = data;
        return;

      case 0xFF43:
        m_ScrollX = data;
        return;

      case 0xFF44:
        m_LY = data;
        return;

      case 0xFF45:
        m_LYC = data;
        return;

      case 0xFF46:
        m_DMA = data;
        bus.m_DMA.Start(data);
        return;

      case 0xFF47:
        m_BGPalette = data;
        UpdatePalette(data, 0);
        return;

      case 0xFF48:
        m_OBJPalette[0] = data;
        UpdatePalette(data, 1);
        return;

      case 0xFF49:
        m_OBJPalette[1] = data;
        UpdatePalette(data, 2);
        return;

      case 0xFF4A:
        m_WindowY = data;
        return;

      case 0xFF4B:
        m_WindowX = data;
        return;

      default:
        break;
    }
  }

  void Display::UpdatePalette(uint8_t data, uint8_t palette) {
    switch (palette) {
      case 0:
        bgColors[0] = m_DefaultColors[data & 0x3];
        bgColors[1] = m_DefaultColors[(data >> 2) & 0x3];
        bgColors[2] = m_DefaultColors[(data >> 4) & 0x3];
        bgColors[3] = m_DefaultColors[(data >> 6) & 0x3];
        break;

      case 1:
        sp1Colors[0] = m_DefaultColors[data & 0x3];
        sp1Colors[1] = m_DefaultColors[(data >> 2) & 0x3];
        sp1Colors[2] = m_DefaultColors[(data >> 4) & 0x3];
        sp1Colors[3] = m_DefaultColors[(data >> 6) & 0x3];
        break;

      case 2:
        sp2Colors[0] = m_DefaultColors[data & 0x3];
        sp2Colors[1] = m_DefaultColors[(data >> 2) & 0x3];
        sp2Colors[2] = m_DefaultColors[(data >> 4) & 0x3];
        sp2Colors[3] = m_DefaultColors[(data >> 6) & 0x3];
        break;

      default:
        break; // notify
    }
  }

  uint16_t Display::TileStartAddrByIndex(uint8_t tileIndex) {
    uint16_t addr = 0;

    if (Bit(m_LCDControl, (uint8_t) LCDControlFlag::BackgroundWindowTileDataArea)) {
      // $8000 Mode
      addr = 0x8000 + tileIndex;
    } else {
      // $8800 Mode
      addr = 0x9000 + (int8_t) tileIndex;
    }

    return addr;
  }

  void Display::SetLY(uint8_t value) {
    m_LY = value;

    if (m_LY == m_LYC) {
      RequestInterrupt((uint8_t) LCDStatusFlag::LYCisLYInter);
    }
  }

  void Display::RequestInterrupt(uint8_t interrupt) {
    auto &bus = Gameboy::Get();

    if (Bit(m_LCDStatus, interrupt)) {
      Interrupts::RequestInterrupt(bus.m_Cpu, Interrupts::Interrupt::LCDStat);
    }
  }

} // hijo