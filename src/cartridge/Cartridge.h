#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace hijo {

  class Cartridge {
  public:
    // $0100 - $014F
    struct Header {
      std::string entryPoint; // 0100 - 0103
      uint8_t logo[48]; // 0104 - 0133
      std::string title; // 0134 - 0143
      // 13F-0142: Manufactor Code
      // 143 - CGB Flag
      std::string licensee;
      uint16_t newLicenseeCode;  // 0144-0145
      uint8_t sgbFlag; // 0146
      std::string mapper;
      uint8_t cartridgeType; // 0147
      uint8_t romSize; // 0148
      uint8_t ramSize; // 0149
      uint8_t destinationCode; // 014A
      uint8_t licenseeCode; // 014B
      uint8_t maskRomVersion; // 014C
      uint8_t headerChecksum; // 014D
      uint16_t globalChecksum; // 014E - 014F
    };

  public:
    std::vector<uint8_t> &Data() {
      return m_Data;
    }

  public:
    static std::unique_ptr<Cartridge> Load(const std::string &path);

  private:
    explicit Cartridge(const std::string &path);

    void LoadHeader();

  private:
    Header m_Header;
    std::vector<uint8_t> m_Data;

    std::map<uint8_t, std::string> m_NewLicenseeMap = {
        {0x00, "none"},
        {0x13, "electronic arts"},
        {0x20, "kss"},
        {0x25, "san-x"},
        {0x30, "viacom"},
        {0x33, "ocean/acclaim"},
        {0x37, "taito"},
        {0x41, "ubi soft"},
        {0x46, "angel"},
        {0x50, "absolute"},
        {0x53, "american sammy"},
        {0x56, "ljn"},
        {0x59, "milton bradley"},
        {0x64, "lucasarts"},
        {0x70, "infogrames"},
        {0x73, "sculptured"},
        {0x79, "accolade"},
        {0x86, "tokuma shoten i*"},
        {0x92, "video system"},
        {0x96, "yonezawa/spal"},
        {0x01, "nintendo"},
        {0x18, "hudsonsoft"},
        {0x22, "pow"},
        {0x28, "kemco japan"},
        {0x31, "nintendo"},
        {0x34, "konami"},
        {0x38, "hudson"},
        {0x42, "atlus"},
        {0x47, "pullet-proof"},
        {0x51, "acclaim"},
        {0x54, "konami"},
        {0x57, "matchbox"},
        {0x60, "titus"},
        {0x67, "ocean"},
        {0x71, "interplay"},
        {0x75, "sci"},
        {0x80, "misawa"},
        {0x87, "tsukuda ori*"},
        {0x93, "ocean/acclaim"},
        {0x97, "kaneko"},
        {0x08, "capcom"},
        {0x19, "b-ai"},
        {0x24, "pcm complete"},
        {0x29, "seta"},
        {0x32, "bandia"},
        {0x35, "hector"},
        {0x39, "banpresto"},
        {0x44, "malibu"},
        {0x49, "irem"},
        {0x52, "activision"},
        {0x55, "hi tech entertainment"},
        {0x58, "mattel"},
        {0x61, "virgin"},
        {0x69, "electronic arts"},
        {0x72, "broderbund"},
        {0x78, "t*hq"},
        {0x83, "lozc"},
        {0x91, "chun soft"},
        {0x95, "varie"},
        {0x99, "pack in soft"}
    };

    std::map<uint8_t, std::string> m_OldLicenseeMap = {
        {0x00, "none"},
        {0x09, "hot-b"},
        {0x0C, "elite systems"},
        {0x19, "itc entertainment"},
        {0x1F, "virgin"},
        {0x28, "kotobuki systems"},
        {0x31, "nintendo"},
        {0x34, "konami"},
        {0x39, "banpresto"},
        {0x41, "ubi soft"},
        {0x46, "angel"},
        {0x4A, "virgin"},
        {0x50, "absolute"},
        {0x53, "american sammy"},
        {0x56, "ljn"},
        {0x5A, "mindscape"},
        {0x5D, "tradewest"},
        {0x67, "ocean"},
        {0x6F, "electro brain"},
        {0x72, "broderbund"},
        {0x78, "t*hq"},
        {0x7C, "microprose"},
        {0x83, "lozc"},
        {0x8C, "vic tokai"},
        {0x91, "chun soft"},
        {0x95, "varie"},
        {0x99, "arc"},
        {0x9C, "imagineer"},
        {0xA1, "hori electric"},
        {0xA6, "kawada"},
        {0xAA, "broderbund"},
        {0xAF, "namco"},
        {0xB2, "bandai"},
        {0xB7, "snk"},
        {0xBB, "sunsoft"},
        {0xC0, "taito"},
        {0xC4, "*tokuma shoten i"},
        {0xC8, "koei"},
        {0xCB, "vap"},
        {0xCE, "*pony canyon or"},
        {0xD1, "sofel"},
        {0xD4, "ask kodansha"},
        {0xD9, "banpresto"},
        {0xDD, "ncs"},
        {0xE0, "jaleco"},
        {0xE3, "varie"},
        {0xE8, "asmik"},
        {0xEB, "atlus"},
        {0xF0, "a wave"},
        {0x01, "nintendo"},
        {0x0A, "jaleco"},
        {0x13, "electronic arts"},
        {0x1A, "yanoman"},
        {0x24, "pcm complete"},
        {0x29, "seta"},
        {0x32, "bandai"},
        {0x35, "hector"},
        {0x3C, "*entertainment i"},
        {0x42, "atlus"},
        {0x47, "spectrum holoby"},
        {0x4D, "malibu"},
        {0x51, "acclaim"},
        {0x54, "gametek"},
        {0x57, "matchbox"},
        {0x5B, "romstar"},
        {0x60, "titus"},
        {0x69, "electronic arts"},
        {0x70, "infogrames"},
        {0x73, "sculptered soft"},
        {0x79, "accolade"},
        {0x7F, "kemco"},
        {0x86, "*tokuma shoten i"},
        {0x8E, "ape"},
        {0x92, "video system"},
        {0x96, "yonezawa/s'pal"},
        {0x9A, "nihon bussan"},
        {0x9D, "banpresto"},
        {0xA2, "bandai"},
        {0xA7, "takara"},
        {0xAC, "toei animation"},
        {0xB0, "acclaim"},
        {0xB4, "enix"},
        {0xB9, "pony canyon"},
        {0xBD, "sony imagesoft"},
        {0xC2, "kemco"},
        {0xC5, "data east"},
        {0xC9, "ufl"},
        {0xCC, "use"},
        {0xCF, "angel"},
        {0xD2, "quest"},
        {0xD6, "naxat soft"},
        {0xDA, "tomy"},
        {0xDE, "human"},
        {0xE1, "towachiki"},
        {0xE5, "epoch"},
        {0xE9, "natsume"},
        {0xEC, "epic/sony records"},
        {0xF3, "extreme entertainment"},
        {0x08, "capcom"},
        {0x0B, "coconuts"},
        {0x18, "hudsonsoft"},
        {0x1D, "clary"},
        {0x25, "san-x"},
        {0x30, "infogrames"},
        {0x33, "see above"},
        {0x38, "capcom"},
        {0x3E, "gremlin"},
        {0x44, "malibu"},
        {0x49, "irem"},
        {0x4F, "u.s. gold"},
        {0x52, "activision"},
        {0x55, "park place"},
        {0x59, "milton bradley"},
        {0x5C, "naxat soft"},
        {0x61, "virgin"},
        {0x6E, "elite systems"},
        {0x71, "interplay"},
        {0x75, "the sales curve"},
        {0x7A, "triffix entertainment"},
        {0x80, "misawa entertainment"},
        {0x8B, "bullet-proof software"},
        {0x8F, "i'max"},
        {0x93, "tsuburava"},
        {0x97, "kaneko"},
        {0x9B, "tecmo"},
        {0x9F, "nova"},
        {0xA4, "konami"},
        {0xA9, "technos japan"},
        {0xAD, "toho"},
        {0xB1, "ascii or nexoft"},
        {0xB6, "hal"},
        {0xBA, "*culture brain o"},
        {0xBF, "sammy"},
        {0xC3, "squaresoft"},
        {0xC6, "tonkin house"},
        {0xCA, "ultra"},
        {0xCD, "meldac"},
        {0xD0, "taito"},
        {0xD3, "sigma enterprises"},
        {0xD7, "copya systems"},
        {0xDB, "ljn"},
        {0xDF, "altron"},
        {0xE2, "uutaka"},
        {0xE7, "athena"},
        {0xEA, "king records"},
        {0xEE, "igs"},
        {0xFF, "ljn"}
    };

    std::map<uint8_t, std::string> m_MapperMap = {
        {0x00, "ROM"},
        {0x03, "MBC1+RAM+BATTERY"},
        {0x08, "ROM+RAM"},
        {0x0C, "MMM01+RAM"},
        {0x10, "MBC3+TIMER+RAM+BATTERY"},
        {0x13, "MBC3+RAM+BATTERY"},
        {0x17, "MBC4+RAM+BATTERY"},
        {0x1B, "MBC5+RAM+BATTERY"},
        {0x1E, "MBC5+RUMBLE+RAM+BATTERY"},
        {0xFE, "HuC3"},
        {0x01, "MBC1"},
        {0x05, "MBC2"},
        {0x09, "ROM+RAM+BATTERY"},
        {0x0D, "MMM01+RAM+BATTERY"},
        {0x11, "MBC3"},
        {0x15, "MBC4"},
        {0x19, "MBC5"},
        {0x1C, "MBC5+RUMBLE"},
        {0xFC, "POCKET CAMERA"},
        {0xFF, "HuC1+RAM+BATTERY"},
        {0x02, "MBC1+RAM"},
        {0x06, "MBC2+BATTERY"},
        {0x0B, "MMM01"},
        {0x0F, "MBC3+TIMER+BATTERY"},
        {0x12, "MBC3+RAM"},
        {0x16, "MBC4+RAM"},
        {0x1A, "MBC5+RAM"},
        {0x1D, "MBC5+RUMBLE+RAM"},
        {0xFD, "Bandai TAMA5"}
    };
  };

} // hijo

