#include "common.h"

namespace hijo {
  bool BIT(uint8_t data, uint8_t number) {
    return (data & (1 << number)) != 0;
  }

  void BIT_SET(uint8_t &data, uint8_t number, bool isSet) {
    if (isSet)
      data |= (1 << number);
    else
      data &= ~(1 << number);
  }
}