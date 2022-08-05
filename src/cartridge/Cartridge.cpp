#include "Cartridge.h"

#include <fstream>
#include <spdlog/spdlog.h>

#include "mappers/ROM.h"
#include "mappers/MBC1.h"

namespace hijo {
  std::unique_ptr<Cartridge> Cartridge::Load(const std::string &path) {
    return std::unique_ptr<Cartridge>(new Cartridge(path));
  }

  Cartridge::Cartridge(const std::string &path) {
    std::ifstream stream(path.c_str(), std::ios::binary | std::ios::ate);

    if (!stream.good()) {
      // TODO: Do something here
      return;
    }

    auto position = stream.tellg();
    auto fileSize = static_cast<size_t>(position);

    std::vector<char> contents(fileSize);

    stream.seekg(0, std::ios::beg);
    stream.read(&contents[0], static_cast<std::streamsize>(position));
    stream.close();

    m_Data = std::vector<uint8_t>(contents.begin(), contents.end());

    LoadHeader();
    LoadMapper();
  }

  void Cartridge::LoadHeader() {
    if (m_Data.size() >= 0x0150) {
      m_Header.title.clear();

      for (auto i = 0x134; i < 0x143; i++)
        if (m_Data[i])
          m_Header.title.push_back(m_Data[i]);

      m_Header.CGBFlag = m_Data[0x143];

      m_Header.newLicenseeCode.clear();
      for (auto i = 0x144; i < 0x146; i++)
        m_Header.newLicenseeCode.push_back((char) m_Data[i]);

      m_Header.sgbFlag = m_Data[0x146];
      m_Header.cartridgeType = m_Data[0x147];
      m_Header.romSize = m_Data[0x148];
      m_Header.ramSize = m_Data[0x149];
      m_Header.destinationCode = m_Data[0x14A];
      m_Header.licenseeCode = m_Data[0x14B];
      m_Header.maskRomVersion = m_Data[0x14C];
      m_Header.headerChecksum = m_Data[0x14D];
      m_Header.globalChecksum = 0;

      uint16_t x = 0;
      for (uint16_t i = 0x0134; i <= 0x014C; i++) {
        x = x - m_Data[i] - 1;
      }

      m_Header.headerChecksumPassed = x & 0xFF;

      if (m_Header.licenseeCode == 0x33) {
        if (m_NewLicenseeMap.contains(m_Header.newLicenseeCode)) {
          m_Header.licensee = m_NewLicenseeMap[m_Header.newLicenseeCode];
        }
      } else {
        if (m_OldLicenseeMap.contains(m_Header.licenseeCode)) {
          m_Header.licensee = m_OldLicenseeMap[m_Header.licenseeCode];
        }
      }

      if (m_MapperMap.contains(m_Header.cartridgeType)) {
        m_Header.mapperInfo = m_MapperMap[m_Header.cartridgeType];
      }

      if (m_RomSize.contains(m_Header.romSize)) {
        m_Header.romInfo = m_RomSize[m_Header.romSize];
      }

      if (m_RamSize.contains(m_Header.ramSize)) {
        m_Header.ramInfo = m_RamSize[m_Header.ramSize];
      }
    }
  }

  uint8_t Cartridge::Read(uint16_t addr) {
    return m_Mapper->Read(addr);
  }

  void Cartridge::Write(uint16_t addr, uint8_t data) {
    m_Mapper->Write(addr, data);
  }

  void Cartridge::LoadMapper() {
    switch (m_Header.mapperInfo.type) {
      case Mapper::Type::ROM:
        m_Mapper = std::make_unique<ROM>();
        m_Mapper->SetRomData(m_Data);
        break;

      case Mapper::Type::MBC1:
        m_Mapper = std::make_unique<MBC1>();
        m_Mapper->SetRomData(m_Data);
        m_Mapper->SetRomBanks(m_Header.romInfo.romBankCount);
        m_Mapper->SetRamBanks(m_Header.ramInfo.ramBankCount);
        break;

      default:
        m_Mapper = std::make_unique<ROM>();
        m_Mapper->SetRomData(m_Data);
        spdlog::get("console")->warn("Unsupported Mapper type: {}, using ROM mapper.", m_Header.cartridgeType);
        break;
    }

    if (m_Mapper) {
      m_Mapper->SetFeatures(m_Header.mapperInfo.hasRam,
                            m_Header.mapperInfo.hasBattery,
                            m_Header.mapperInfo.hasTimer,
                            m_Header.mapperInfo.hasRumble);
    }
  }

} // hijo