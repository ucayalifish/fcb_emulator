#ifndef FCB_EMULATOR_NANDEMU_H
#define FCB_EMULATOR_NANDEMU_H

#include "nandemudef.h"

void nandemu_reset(void);

int nandemu_block_erase(block_id_t blk);

int nandemu_block_prog(block_id_t blk, uint8_t const * data);

int nandemu_page_read(block_id_t blk, page_id_t pg, uint8_t * dest);

void nandemu_inject_bads(void);

void nandemu_inject_timebombs(void);

int nandemu_is_bad_number(void);

int nandemu_marked_bad_number(void);

int nandemu_is_erased_number(void);

#endif //FCB_EMULATOR_NANDEMU_H
