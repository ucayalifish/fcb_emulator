#ifndef FCB_EMULATOR_NANDEMU_H
#define FCB_EMULATOR_NANDEMU_H

#include "nandemudef.h"

void nandemu_reset(void);

int nandemu_block_erase(block_id_t blk);

int nandemu_block_prog(block_id_t blk, uint8_t const * data);

int nandemu_page_read(block_id_t blk, page_id_t pg, uint8_t * dest);

int nandemu_number_of_bad(void);

int nandemu_number_of_marked_bad(void);

int nandemu_is_erased_number(void);

#endif //FCB_EMULATOR_NANDEMU_H
