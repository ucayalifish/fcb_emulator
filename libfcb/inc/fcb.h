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
                  + ((uint64_t) 'B' << 56ull))

struct fcb_block_header_s
{
  uint64_t magic;
  uint64_t ts_marker; // timestamp of new consecutive sequenceof blocks creation
  uint32_t crc32;
  uint32_t data_offset; // offset of first table in bytes from start of block
};

_Static_assert(sizeof(struct fcb_block_header_s) == 24, "ok");
_Static_assert(sizeof(struct fcb_block_header_s) - 2 * sizeof(uint64_t) - 2 * sizeof(uint32_t) == 0, "ok");

#define TBL_MAGIC ('T'                      \
                  + ((uint32_t) 'A' << 8U)  \
                  + ((uint32_t) 'B' << 16U) \
                  + ((uint32_t) 'L' << 24U))

struct fcb_table_header_s
{
  uint32_t magic;
  uint32_t record_size;
  uint32_t num_records;
  uint32_t first_record_id;
};

#endif //FCB_EMULATOR_FCB_H
