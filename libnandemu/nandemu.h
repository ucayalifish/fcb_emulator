#ifndef FCB_EMULATOR_NANDEMU_H
#define FCB_EMULATOR_NANDEMU_H

#include <stdint.h>

typedef enum {
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

void nandemu_reset(void);

int nandemu_block_erase(block_id_t blk, nandemu_error_t * error);

#endif //FCB_EMULATOR_NANDEMU_H
