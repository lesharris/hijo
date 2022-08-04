#include "common.h"

namespace hijo {
  uint8_t BIT(uint8_t data, uint8_t number) {
    return (data & (1 << number)) ? 1 : 0;
  }

  void BIT_SET(uint8_t &data, uint8_t number, bool isSet) {
    if (isSet)
      data |= (1 << number);
    else
      data &= ~(1 << number);
  }

  bool BETWEEN(uint16_t a, uint16_t b, uint16_t c) {
    return (a >= b) && (a <= c);
  }
}