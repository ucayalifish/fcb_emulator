#ifndef FCB_EMULATOR_NANDEMU_H
#define FCB_EMULATOR_NANDEMU_H

#include <stdbool.h>

#include "nandemudef.h"

void nandemu_reset(void);

int nandemu_block_erase(block_id_t blk);

int nandemu_block_prog(block_id_t blk, uint8_t const * data);

int nandemu_block_read(block_id_t blk, uint8_t * dest);

int nandemu_number_of_failed(void);

int nandemu_number_of_marked_bad(void);

int nandemu_is_erased_number(void);

bool nandemu_is_marked_bad(block_id_t blk);

void nandemu_mark_bad(block_id_t blk);

int nandemu_timebombed_number(void);

bool nandemu_is_not_bad_and_erased(block_id_t blk);

#endif //FCB_EMULATOR_NANDEMU_H
