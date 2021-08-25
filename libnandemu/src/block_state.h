#ifndef FCB_EMULATOR_BLOCK_STATE_H
#define FCB_EMULATOR_BLOCK_STATE_H

#include <stdbool.h>
#include <nandemudef.h>

void block_state_reset(void);

bool block_state_is_marked_bad(block_id_t blk);

void block_state_mark_bad(block_id_t blk);

void block_state_timebomb_tick(block_id_t blk);

bool block_state_failed(block_id_t blk);

bool block_state_is_erased(block_id_t blk);

void block_state_set_erased(block_id_t blk);

void block_state_clear_erased(block_id_t);

void block_state_inc_erase_failed(block_id_t blk);

void block_state_inc_erase_success(block_id_t blk);

void block_state_inc_prog_failed(block_id_t blk);

void block_state_inc_prog_success(block_id_t blk);

void block_state_inc_read_failed(block_id_t blk);

void block_state_inc_read_success(block_id_t blk);

bool block_state_fail_count_exhausted(block_id_t blk);

#endif //FCB_EMULATOR_BLOCK_STATE_H
