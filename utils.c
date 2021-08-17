#include "utils.h"
#include <stdlib.h>

void fill_random_buffer(unsigned const seed, uint8_t * dest, size_t const dest_size)
{
  srand(seed);

  for (size_t i = 0; i < dest_size; i++)
  {
    dest[i] = (uint8_t) rand();
  }
}
