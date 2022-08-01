#include "Cartridge.h"

#include <fstream>
#include <fmt/format.h>

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
  }

  void Cartridge::LoadHeader() {
    if (m_Data.size() >= 0x0145) {
      m_Header.entryPoint.clear();

      for (auto i = 0x100; i < 0x104; i++)
        m_Header.entryPoint += (fmt::format("{:02X} ", m_Data[i]).c_str());

      for (auto i = 0x105, j = 0; i < 0x133; i++, j++)
        m_Header.logo[j] = m_Data[i];

      m_Header.title.clear();

      for (auto i = 0x134; i < 0x143; i++)
        m_Header.title.push_back(m_Data[i]);
    }
  }

} // hijo