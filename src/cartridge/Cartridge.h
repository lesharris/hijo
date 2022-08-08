#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "mappers/Mapper.h"

namespace hijo {

  class Cartridge {
  public:
    struct RomSizeInfo {
      std::string label;
      uint16_t romBankCount = 0; // # of 16k banks
    };

    struct RamSizeInfo {
      std::string label;
      uint8_t ramBankCount = 0; // # of 8k banks
    };

    struct MapperInfo {
      std::string label;
      Mapper::Type type = Mapper::Type::NONE;
      bool hasRam = false;
      bool hasBattery = false;
      bool hasTimer = false;
      bool hasRumble = false;
    };

    // $0100 - $014F
    struct HeaderData {
      std::string title; // 0134 - 0142
      // 13F-0142: Manufactor Code
      // 143 - CGB Flag
      uint8_t CGBFlag;
      std::string newLicenseeCode;  // 0144-0145
      uint8_t sgbFlag; // 0146
      uint8_t cartridgeType; // 0147
      uint8_t romSize; // 0148
      uint8_t ramSize; // 0149
      uint8_t destinationCode; // 014A
      uint8_t licenseeCode; // 014B
      uint8_t maskRomVersion; // 014C
      uint8_t headerChecksum; // 014D
      uint16_t globalChecksum; // 014E - 014F

      bool headerChecksumPassed;

      std::string licensee;
      MapperInfo mapperInfo;
      RomSizeInfo romInfo;
      RamSizeInfo ramInfo;
    };
  public:
    std::vector<uint8_t> &Data() {
      return m_Data;
    }

  public:
    static std::unique_ptr<Cartridge> Load(const std::string &path);

  public:
    uint8_t Read(uint16_t addr);

    void Write(uint16_t addr, uint8_t data);

    void Tick(double timestep);

    const HeaderData &Header() const {
      return m_Header;
    }

  private:
    explicit Cartridge(const std::string &path);

    void LoadHeader();

    void LoadMapper();

  private:
    friend class UI;

  private:
    HeaderData m_Header;
    std::vector<uint8_t> m_Data;
    std::unique_ptr<Mapper> m_Mapper;

    std::map<std::string, std::string> m_NewLicenseeMap = {
        {"00", "None"},
        {"13", "Electronic Arts"},
        {"20", "KSS"},
        {"25", "San-X"},
        {"30", "Viacom"},
        {"33", "Ocean/Acclaim"},
        {"37", "Taito"},
        {"41", "Ubisoft"},
        {"46", "Angel"},
        {"50", "Absolute"},
        {"53", "American Sammy"},
        {"56", "LJN"},
        {"59", "Milton Bradley"},
        {"64", "Lucasarts"},
        {"70", "Infogrames"},
        {"73", "Sculptured"},
        {"79", "Accolade"},
        {"86", "Tokuma Shoten i*"},
        {"92", "Video System"},
        {"96", "Yonezawa/Spal"},
        {"01", "Nintendo"},
        {"18", "Hudsonsoft"},
        {"22", "POW"},
        {"28", "Kemco Japan"},
        {"31", "Nintendo"},
        {"34", "Konami"},
        {"38", "Hudson"},
        {"42", "Atlus"},
        {"47", "Pullet-Proof"},
        {"51", "Acclaim"},
        {"54", "Konami"},
        {"57", "Matchbox"},
        {"60", "Titus"},
        {"67", "Ocean"},
        {"71", "Interplay"},
        {"75", "SCI"},
        {"80", "Misawa"},
        {"87", "Tsukuda Ori*"},
        {"93", "Ocean/Acclaim"},
        {"97", "Kaneko"},
        {"08", "Capcom"},
        {"19", "B-AI"},
        {"24", "PCM Complete"},
        {"29", "Seta"},
        {"32", "Bandai Namco"},
        {"35", "Hector"},
        {"39", "Ban Presto"},
        {"44", "Malibu"},
        {"49", "Irem"},
        {"52", "Activision"},
        {"55", "Hi Tech Entertainment"},
        {"58", "Mattel"},
        {"61", "Virgin"},
        {"69", "Electronic Arts"},
        {"72", "Broderbund"},
        {"78", "T*HQ"},
        {"83", "Lozc"},
        {"91", "Chun Soft"},
        {"95", "Varie"},
        {"99", "Pack in Soft"}
    };

    std::map<uint8_t, std::string> m_OldLicenseeMap = {
        {0x00, "none"},
        {0x09, "Hot-B"},
        {0x0C, "Elite Systems"},
        {0x19, "ITC Entertainment"},
        {0x1F, "Virgin"},
        {0x28, "Kotobuki Systems"},
        {0x31, "Nintendo"},
        {0x34, "Konami"},
        {0x39, "Ban Presto"},
        {0x41, "Ubisoft"},
        {0x46, "Angel"},
        {0x4A, "Virgin"},
        {0x50, "Absolute"},
        {0x53, "American Sammy"},
        {0x56, "LJN"},
        {0x5A, "Mindscape"},
        {0x5D, "Tradewest"},
        {0x67, "Ocean"},
        {0x6F, "Electro Brain"},
        {0x72, "Broderbund"},
        {0x78, "T*HQ"},
        {0x7C, "Microprose"},
        {0x83, "Lozc"},
        {0x8C, "Vic Tokai"},
        {0x91, "Chun Soft"},
        {0x95, "Varie"},
        {0x99, "Arc"},
        {0x9C, "Imagineer"},
        {0xA1, "Hori Electric"},
        {0xA6, "Kawada"},
        {0xAA, "Broderbund"},
        {0xAF, "Namco"},
        {0xB2, "Bandai"},
        {0xB7, "SNK"},
        {0xBB, "Sunsoft"},
        {0xC0, "Taito"},
        {0xC4, "*Tokuma Shoten i"},
        {0xC8, "Koei"},
        {0xCB, "Vap"},
        {0xCE, "*Pony Canyon or"},
        {0xD1, "Sofel"},
        {0xD4, "Ask Kodansha"},
        {0xD9, "Banpresto"},
        {0xDD, "NCS"},
        {0xE0, "Jaleco"},
        {0xE3, "Varie"},
        {0xE8, "Asmik"},
        {0xEB, "Atlus"},
        {0xF0, "A Wave"},
        {0x01, "Nintendo"},
        {0x0A, "Jaleco"},
        {0x13, "Electronic Arts"},
        {0x1A, "Yanoman"},
        {0x24, "PCM Complete"},
        {0x29, "Seta"},
        {0x32, "Bandai"},
        {0x35, "Hector"},
        {0x3C, "*Entertainment i"},
        {0x42, "Atlus"},
        {0x47, "Spectrum Holobyte"},
        {0x4D, "Malibu"},
        {0x51, "Acclaim"},
        {0x54, "Gametek"},
        {0x57, "Matchbox"},
        {0x5B, "Romstar"},
        {0x60, "Titus"},
        {0x69, "Electronic Arts"},
        {0x70, "Infogrames"},
        {0x73, "Sculptered Software"},
        {0x79, "Accolade"},
        {0x7F, "Kemco"},
        {0x86, "*Tokuma Shoten i"},
        {0x8E, "Ape"},
        {0x92, "Video System"},
        {0x96, "Yonezawa/S'pal"},
        {0x9A, "Nihon Bussan"},
        {0x9D, "Banpresto"},
        {0xA2, "Bandai"},
        {0xA7, "Takara"},
        {0xAC, "Toei Animation"},
        {0xB0, "Acclaim"},
        {0xB4, "Enix"},
        {0xB9, "Pony Canyon"},
        {0xBD, "Sony Imagesoft"},
        {0xC2, "Kemco"},
        {0xC5, "Data East"},
        {0xC9, "UFL"},
        {0xCC, "USE"},
        {0xCF, "Angel"},
        {0xD2, "Quest"},
        {0xD6, "Naxat Soft"},
        {0xDA, "Tomy"},
        {0xDE, "Human"},
        {0xE1, "Towachiki"},
        {0xE5, "Epoch"},
        {0xE9, "Natsume"},
        {0xEC, "Epic/Sony Records"},
        {0xF3, "Extreme Entertainment"},
        {0x08, "Capcom"},
        {0x0B, "Coconuts"},
        {0x18, "Hudsonsoft"},
        {0x1D, "Clary"},
        {0x25, "San-X"},
        {0x30, "Infogrames"},
        {0x33, "See Above"},
        {0x38, "Capcom"},
        {0x3E, "Gremlin"},
        {0x44, "Malibu"},
        {0x49, "Irem"},
        {0x4F, "U.S. Gold"},
        {0x52, "Activision"},
        {0x55, "Park Place"},
        {0x59, "Milton Bradley"},
        {0x5C, "Naxat Soft"},
        {0x61, "Virgin"},
        {0x6E, "Elite Systems"},
        {0x71, "Interplay"},
        {0x75, "The Sales Curve"},
        {0x7A, "Triffix Entertainment"},
        {0x80, "Misawa Entertainment"},
        {0x8B, "Bullet-Proof Software"},
        {0x8F, "I'Max"},
        {0x93, "Tsuburava"},
        {0x97, "Kaneko"},
        {0x9B, "Tecmo"},
        {0x9F, "Nova"},
        {0xA4, "Konami"},
        {0xA9, "Technos Japan"},
        {0xAD, "Toho"},
        {0xB1, "ASCII or Nexoft"},
        {0xB6, "HAL"},
        {0xBA, "*Culture Brain o"},
        {0xBF, "Sammy"},
        {0xC3, "Squaresoft"},
        {0xC6, "Tonkin House"},
        {0xCA, "Ultra"},
        {0xCD, "Meldac"},
        {0xD0, "Taito"},
        {0xD3, "Sigma Enterprises"},
        {0xD7, "Copya Systems"},
        {0xDB, "LJN"},
        {0xDF, "Altron"},
        {0xE2, "Uutaka"},
        {0xE7, "Athena"},
        {0xEA, "King Records"},
        {0xEE, "IGS"},
        {0xFF, "LJN"}
    };

    std::map<uint8_t, MapperInfo> m_MapperMap = {
        {0x00, {"ROM",                     Mapper::Type::ROM,   false, false, false, false}},
        {0x03, {"MBC1+RAM+BATTERY",        Mapper::Type::MBC1,  true,  true,  false, false}},
        {0x08, {"ROM+RAM",                 Mapper::Type::ROM,   true,  false, false, false}},
        {0x0C, {"MMM01+RAM",               Mapper::Type::MMM01, true,  false, false, false}},
        {0x10, {"MBC3+TIMER+RAM+BATTERY",  Mapper::Type::MBC3,  true,  true,  true,  false}},
        {0x13, {"MBC3+RAM+BATTERY",        Mapper::Type::MBC3,  true,  true,  false, false}},
        {0x1B, {"MBC5+RAM+BATTERY",        Mapper::Type::MBC5,  true,  true,  false, false}},
        {0x1E, {"MBC5+RUMBLE+RAM+BATTERY", Mapper::Type::MBC5,  true,  true,  false, true}},
        {0xFE, {"HuC3",                    Mapper::Type::HuC3,  false, false, false, false}},
        {0x01, {"MBC1",                    Mapper::Type::MBC1,  false, false, false, false}},
        {0x05, {"MBC2",                    Mapper::Type::MBC2,  false, false, false, false}},
        {0x09, {"ROM+RAM+BATTERY",         Mapper::Type::ROM,   true,  true,  false, false}},
        {0x0D, {"MMM01+RAM+BATTERY",       Mapper::Type::MMM01, true,  true,  false, false}},
        {0x11, {"MBC3",                    Mapper::Type::MBC3,  false, false, false, false}},
        {0x19, {"MBC5",                    Mapper::Type::MBC5,  false, false, false, false}},
        {0x1C, {"MBC5+RUMBLE",             Mapper::Type::MBC5,  false, false, false, true}},
        {0xFC, {"POCKET CAMERA",           Mapper::Type::OTHER, false, false, false, false}},
        {0xFF, {"HuC1+RAM+BATTERY",        Mapper::Type::HuC1,  true,  true,  false, false}},
        {0x02, {"MBC1+RAM",                Mapper::Type::MBC1,  true,  false, false, false}},
        {0x06, {"MBC2+BATTERY",            Mapper::Type::MBC2,  false, true,  false, false}},
        {0x0B, {"MMM01",                   Mapper::Type::MMM01, false, false, false, false}},
        {0x0F, {"MBC3+TIMER+BATTERY",      Mapper::Type::MBC3,  false, true,  true,  false}},
        {0x12, {"MBC3+RAM",                Mapper::Type::MBC3,  true,  false, false, false}},
        {0x1A, {"MBC5+RAM",                Mapper::Type::MBC5,  true,  false, false, false}},
        {0x1D, {"MBC5+RUMBLE+RAM",         Mapper::Type::MBC5,  true,  false, false, true}},
        {0xFD, {"Bandai TAMA5",            Mapper::Type::OTHER, false, false, false, false}},
        {0xFE, {"HuC3",                    Mapper::Type::HuC3,  false, false, false, false}},
        {0xFF, {"HuC1+RAM+BATTERY",        Mapper::Type::HuC1,  true,  true,  false, false}}
    };

    std::map<uint8_t, RomSizeInfo> m_RomSize{
        {0x00, {"32 KiB",  2}},
        {0x01, {"64 KiB",  4}},
        {0x02, {"128 KiB", 8}},
        {0x03, {"256 KiB", 16}},
        {0x04, {"512 KiB", 32}},
        {0x05, {"1 MiB",   64}},
        {0x06, {"2 MiB",   128}},
        {0x07, {"4 MiB",   256}},
        {0x08, {"8 MiB",   512}},
        {0x52, {"1.1 MiB", 72}},
        {0x53, {"1.2 MiB", 80}},
        {0x54, {"1.5 MiB", 96}}
    };

    std::map<uint8_t, RamSizeInfo> m_RamSize{
        {0, {"None",    0}},
        {2, {"8 KiB",   1}},
        {3, {"32 KiB",  4}},
        {4, {"128 KiB", 16}},
        {5, {"64 KiB",  8}}
    };
  };

} // hijo

