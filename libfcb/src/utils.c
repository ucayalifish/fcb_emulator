#include <time.h>
#include <stdlib.h>
#include "utils.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "cert-msc50-cpp"
uint64_t timestamp_marker(void)
{
  uint64_t ret = time(NULL);
  ret <<= 7U;
  int const msecs = rand() & ((1U << 7U) - 1U);
  return ret + msecs;
}
#pragma clang diagnostic pop

