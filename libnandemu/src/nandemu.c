#include <stddef.h>
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

static int block_erase_impl_(block_id_t const blk)
{
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

int nandemu_block_erase(block_id_t const blk)
{
  if (blk > NUM_BLOCKS)
    {
      fprintf(stderr, "nandemu: nandemu_block_erase called on invalid block: %d\n", blk);
      abort();
    }

  return block_erase_impl_(blk);
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

  ptrdiff_t offset = 0;

  for (page_id_t pg = 0; pg < PAGES_PER_BLOCK; ++pg)
    {
      block_state_timebomb_tick(blk);
      if (block_state_failed(blk))
        {
          block_state_inc_prog_failed(blk);
          return block_state_fail_count_exhausted(blk) ? NANDEMU_E_BAD_BLOCK : NANDEMU_E_ECC;
        }
      uint8_t * dest = flash_.blocks[blk].pages[pg].page;
      memcpy(dest + offset, data + offset, PAGE_SIZE);
      offset += PAGE_SIZE;
    }

  block_state_inc_prog_success(blk);
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

  ptrdiff_t offset = 0;

  for (page_id_t pg = 0; pg < PAGES_PER_BLOCK; ++pg)
    {
      block_state_timebomb_tick(blk);
      if (block_state_failed(blk))
        {
          block_state_inc_read_failed(blk);
          return block_state_fail_count_exhausted(blk) ? NANDEMU_E_BAD_BLOCK : NANDEMU_E_ECC;
        }

      dest += offset;
      uint8_t const * src = flash_.blocks[blk].pages[pg].page + offset;
      memcpy(dest, src, PAGE_SIZE);
      offset += PAGE_SIZE;
    }

  block_state_inc_read_success(blk);

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

bool nandemu_is_not_bad_and_erased(block_id_t const blk)
{
  if (blk >= NUM_BLOCKS)
    {
      fprintf(stderr, "nandemu: nandemu_is_not_bad_and_erased called on invalid block: %d\n", blk);
      abort();
    }

  if (!block_state_is_marked_bad(blk))
    {
      uint8_t * page = flash_.blocks->pages[0].page;
      for (unsigned i = 0; i < PAGE_SIZE; ++i)
        {
          if (page[i] != 0xffU)
            {
              return false;
            }
        }

      return true;
    }

  return false;
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

block_id_t nandemu_find_and_erase_next_block(block_id_t to_check, block_id_t const limit)
{
  while (to_check != limit)
    {
      if (block_state_is_marked_bad(to_check))
        {
          to_check = (to_check + 1) % NUM_BLOCKS;
          continue;
        }

      if (block_state_is_erased(to_check))
        {
          return to_check;
        }

      int r = block_erase_impl_(to_check);

      if (r == NANDEMU_E_NONE)
        {
          return to_check;
        }

      while (r != NANDEMU_E_NONE)
        {
          if (r == NANDEMU_E_BAD_BLOCK)
            {
              block_state_mark_bad(to_check);
              continue;
            }

          r = block_erase_impl_(to_check);
        }

      to_check = (to_check + 1) % NUM_BLOCKS;
    }

  return -1;
}
