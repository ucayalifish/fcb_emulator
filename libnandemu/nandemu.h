#ifndef FCB_EMULATOR_NANDEMU_H
#define FCB_EMULATOR_NANDEMU_H

#include "nandemudef.h"

void nandemu_reset(void);

int nandemu_block_erase(block_id_t blk, nandemu_error_t * error);

int nandemu_nand_sector_prog(page_id_t sector_id, uint8_t const * data, nandemu_error_t * err);

int nandemu_page_prog(block_id_t blk, page_id_t pg, uint8_t const * data);

int nandemu_page_read(block_id_t blk, page_id_t pg, uint8_t * dest);

void nandemu_inject_bads(void);

void nandemu_inject_timebombs(void);

#endif //FCB_EMULATOR_NANDEMU_H
