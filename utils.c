#include "utils.h"
#include <stdlib.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc50-cpp"
void fill_random_buffer(uint8_t * dest, size_t const dest_size)
{
  for (size_t i = 0; i < dest_size; i++)
  {
    dest[i] = (uint8_t) rand();
  }
}
#pragma clang diagnostic pop
