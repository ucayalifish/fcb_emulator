#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <nandemu.h>

#include "block_state.h"

/**
 * NAND emulator based on one from https://github.com/dlbeer/dhara.
 */

// todo: remove?
struct sim_stats
{
  int frozen;

  int is_bad;
  int mark_bad;

  int erase;
  int erase_fail;

  int is_erased;
  int prog;
  int prog_fail;

  int read;
  int read_bytes;
};

union page_u
{
  uint8_t zones[ZONES_PER_PAGE][ZONE_SIZE];
  uint8_t page[PAGE_SIZE];
};

_Static_assert(ZONES_PER_PAGE * ZONE_SIZE == PAGE_SIZE, "ok");

union block_u
{
  union page_u pages[PAGES_PER_BLOCK];
  uint8_t      bytes[BLOCK_SIZE];
};

_Static_assert(BLOCK_SIZE == PAGES_PER_BLOCK * PAGE_SIZE, "ok");

static union flash_u
{
  union block_u blocks[NUM_BLOCKS];
  uint8_t       sectors[NUM_SECTORS][ZONE_SIZE];
  uint8_t       bytes[MEM_SIZE];
} flash_;

_Static_assert(MEM_SIZE == ZONE_SIZE * ZONES_PER_PAGE * PAGES_PER_BLOCK * NUM_BLOCKS, "ok");

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc50-cpp"

static void seq_gen_(uint8_t * buf, size_t const length)
{
  for (size_t i = 0; i < length; i++)
    {
      buf[i] = (uint8_t) rand();
    }
}

#pragma clang diagnostic pop

void nandemu_reset(void)
{
  srand(time(NULL) * 57 + 29);

  memset(flash_.bytes, 0x55, sizeof flash_.bytes);

  block_state_reset();
}

int nandemu_block_erase(block_id_t const blk)
{
  if (blk > NUM_BLOCKS)
    {
      fprintf(stderr, "nandemu: nandemu_block_erase called on invalid block: %d\n", blk);
      abort();
    }

  if (block_state_is_marked_bad(blk))
    {
      fprintf(stderr, "nandemu: nandemu_block_erase called on block which is marked bad: %d\n", blk);
      abort();
    }

  block_state_timebomb_tick(blk);

  uint8_t * const block = flash_.blocks[blk].bytes;

  if (block_state_failed(blk))
    {
      block_state_inc_erase_failed(blk);
      block_state_clear_erased(blk);
      seq_gen_(block, BLOCK_SIZE);
      return block_state_fail_count_exhausted(blk) ? NANDEMU_E_BAD_BLOCK : NANDEMU_E_ECC;
    }

  block_state_inc_erase_success(blk);
  block_state_set_erased(blk);
  memset(block, 0xff, BLOCK_SIZE);
  return NANDEMU_E_NONE;
}

int nandemu_block_prog(block_id_t const blk, uint8_t const * data)
{
  if (blk >= NUM_BLOCKS)
    {
      fprintf(stderr, "\tnandemu: nandemu_block_prog called on invalid block: %d\n", blk);
      abort();
    }

  if (block_state_is_marked_bad(blk))
    {
      fprintf(stderr, "\tnandemu: nandemu_block_prog called on block which is marked bad: %d\n", blk);
      return NANDEMU_E_BAD_BLOCK;
    }

  block_state_timebomb_tick(blk);

  uint8_t * dest = flash_.blocks[blk].bytes;

  if (block_state_failed(blk))
    {
      block_state_inc_prog_failed(blk);
      seq_gen_(dest, PAGE_SIZE);
      return NANDEMU_E_ECC;
    }

  block_state_inc_prog_success(blk);
  memcpy(dest, data, BLOCK_SIZE);

  return NANDEMU_E_NONE;
}

int nandemu_block_read(block_id_t blk, uint8_t * dest)
{
  if (blk >= NUM_BLOCKS)
    {
      fprintf(stderr, "nandemu: nandemu_block_read called on invalid block: %d\n", blk);
      abort();
    }

  if (block_state_is_marked_bad(blk))
    {
      fprintf(stderr, "nandemu: nandemu_block_read called on damaged block: %d\n", blk);
      return NANDEMU_E_BAD_BLOCK;
    }

  block_state_timebomb_tick(blk);

  uint8_t * src = flash_.blocks[blk].bytes;

  if (block_state_failed(blk))
    {
      block_state_inc_read_failed(blk);
      seq_gen_(dest, PAGE_SIZE);
      return NANDEMU_E_ECC;
    }

  block_state_inc_read_success(blk);
  memcpy(dest, src, BLOCK_SIZE);

  return NANDEMU_E_NONE;
}

bool nandemu_is_marked_bad(block_id_t const blk)
{
  if (blk >= NUM_BLOCKS)
  {
    fprintf(stderr, "nandemu: nandemu_is_marked_bad called on invalid block: %d\n", blk);
    abort();
  }

  return block_state_is_marked_bad(blk);
}

void nandemu_mark_bad(block_id_t blk)
{
  if (blk >= NUM_BLOCKS)
  {
    fprintf(stderr, "nandemu: nandemu_mark_bad called on invalid block: %d\n", blk);
    abort();
  }

  block_state_mark_bad(blk);
}
