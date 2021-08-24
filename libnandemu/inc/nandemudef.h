#ifndef FCB_EMULATOR_NANDEMUDEF_H
#define FCB_EMULATOR_NANDEMUDEF_H

#include <stdint.h>

#define LOG2_ZONE_SIZE 9u
#define LOG2_ZONES_PER_PAGE 2u
#define LOG2_PAGE_SIZE (LOG2_ZONE_SIZE + LOG2_ZONES_PER_PAGE)
#define LOG2_PAGES_PER_BLOCK 6u
#define LOG2_ZONES_PER_BLOCK (LOG2_ZONES_PER_PAGE + LOG2_PAGES_PER_BLOCK)
#define LOG2_BLOCK_SIZE (LOG2_PAGE_SIZE + LOG2_PAGES_PER_BLOCK)
#define NUM_BLOCKS 4096U
#define MAX_BAD_BLOCKS 410 // 2% of bad blocks are allowed by manufaturer

_Static_assert(LOG2_PAGE_SIZE == 11U, "ok");
_Static_assert(LOG2_BLOCK_SIZE == 17U, "ok");

#define ZONE_SIZE (1u << LOG2_ZONE_SIZE)
#define ZONES_PER_PAGE (1u << LOG2_ZONES_PER_PAGE)
#define PAGE_SIZE (1u << LOG2_PAGE_SIZE)
#define ZONES_PER_BLOCK (1u << LOG2_ZONES_PER_BLOCK)
#define PAGES_PER_BLOCK (1u << LOG2_PAGES_PER_BLOCK)
#define BLOCK_SIZE (1u << LOG2_BLOCK_SIZE)
#define NUM_SECTORS (NUM_BLOCKS * PAGES_PER_BLOCK * ZONES_PER_PAGE)
#define MEM_SIZE (NUM_BLOCKS * BLOCK_SIZE)

_Static_assert(ZONE_SIZE == 512U, "ok");
_Static_assert(PAGE_SIZE == 2048U, "ok");
_Static_assert(BLOCK_SIZE == 131072U, "ok");

typedef enum nandemu_error_e
{
  NANDEMU_E_NONE = 0,
  NANDEMU_E_BAD_BLOCK,
  NANDEMU_E_ECC,
  NANDEMU_E_TOO_BAD,
  NANDEMU_E_RECOVER,
  NANDEMU_E_JOURNAL_FULL,
  NANDEMU_E_NOT_FOUND,
  NANDEMU_E_MAP_FULL,
  NANDEMU_E_CORRUPT_MAP,
  NANDEMU_E_MAX
} nandemu_error_t;

typedef uint32_t block_id_t;

typedef uint32_t page_id_t;

#define BLOCK_BAD_MARK (1U << 0)

#define BLOCK_FAILED (1U << 1)

#endif //FCB_EMULATOR_NANDEMUDEF_H
