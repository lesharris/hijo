#include "Mapper.h"

#include <vector>

namespace hijo {

  class MBC1 : public Mapper {
  public:
    uint8_t Read(uint16_t addr) override;

    void Write(uint16_t addr, uint8_t data) override;

    void SetRomData(std::vector<uint8_t> &data) override;

    void SetRomBanks(uint16_t bankCount) override;

    void SetRamBanks(uint8_t bankCount) override;

    std::vector<StatLine> GetStats() override;

  private:
    void SetRomBank(uint8_t value);

    void SetRamBank(uint8_t value);

  private:
    std::vector<std::vector<uint8_t>> m_RamBanks;

    std::vector<uint8_t> m_Data;
    uint16_t m_RomBankCount;
    uint8_t m_RamBankCount;

    bool m_RamEnabled = false;
    bool m_RamBanking = false;

    uint8_t m_BankingMode = 0;

    uint8_t m_RomBankValue = 0;
    uint8_t m_RamBankValue = 0;

    uint32_t m_RomBankBase = 0;

    bool m_NeedsSave = false;
  };

} // hijo
