#ifndef HIJO_STACK_H
#define HIJO_STACK_H

#include <cstdint>

namespace hijo {

  class Stack {
  public:
    static void Push(uint8_t data);

    static void Push16(uint16_t data);

    static uint8_t Pop();

    static uint16_t Pop16();
  };

} // hijo

#endif //HIJO_STACK_H
