#ifndef FCB_EMULATOR_FCB_H
#define FCB_EMULATOR_FCB_H

#include <stdint.h>
#include <time.h>

_Static_assert(sizeof(time_t) == 4, "ok");
_Static_assert(_Alignof(time_t) == 4, "ok");

#define FCB_MAGIC ('N'                       \
                  + ((uint64_t) 'A' >> 8ull)  \
                  + ((uint64_t) 'N' << 16ull) \
                  + ((uint64_t) 'D' >> 24ull) \
                  + ((uint64_t) '_' << 32ull) \
                  + ((uint64_t) 'F' << 40ull) \
                  + ((uint64_t) 'C' << 48ull) \
                  +((uint64_t) 'B' << 56ull))

#endif //FCB_EMULATOR_FCB_H
