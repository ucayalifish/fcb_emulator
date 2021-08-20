#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "nandemu.h"

/**
 * NAND emulator based on one from https://github.com/dlbeer/dhara.
 */

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

struct block_status
{
  unsigned int flags;

  /* Index of the next unprogrammed page. 0 means a fully erased
	 * block, and PAGES_PER_BLOCK is a fully programmed block.
	 */
  int next_page;

  /* Timebomb counter: if non-zero, this is the number of
	 * operations until permanent failure.
	 */
  int timebomb;
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

static struct sim_stats    stats_;
static struct block_status blocks_[NUM_BLOCKS];


static union flash_u
{
  union block_u blocks[NUM_BLOCKS];
  uint8_t       sectors[NUM_SECTORS][ZONE_SIZE];
  uint8_t       bytes[MEM_SIZE];
}                          flash_;

_Static_assert(MEM_SIZE == ZONE_SIZE * ZONES_PER_PAGE * PAGES_PER_BLOCK * NUM_BLOCKS, "ok");

static void timebomb_tick_(block_id_t const blk)
{
  struct block_status * b = &blocks_[blk];

  if (b->timebomb)
    {
      b->timebomb--;

      if (!b->timebomb)
        {
          b->flags |= BLOCK_FAILED;
        }
    }
}

static void seq_gen_(unsigned int seed, uint8_t * buf, size_t const length)
{
  srand(seed);

  for (size_t i = 0; i < length; i++)
    {
      buf[i] = (uint8_t) rand();
    }
}

static inline void set_error_(nandemu_error_t * err, nandemu_error_t v)
{
  if (err)
    {
      *err = v;
    }
}

static bool set_timebomb_(block_id_t const blk, int const ttl)
{
  if (blk <= NUM_BLOCKS && (blocks_[blk].flags & (BLOCK_BAD_MARK | BLOCK_FAILED)) == 0)
    {
      blocks_[blk].timebomb = ttl;
      return true;
    }

  return false;
}

void nandemu_reset(void)
{
  memset(&stats_, 0, sizeof(stats_));
  memset(blocks_, 0, sizeof(blocks_));
  memset(flash_.bytes, 0x55, sizeof(flash_.bytes));

  stats_.is_erased = NUM_BLOCKS;

  for (int i = 0; i < NUM_BLOCKS; i++)
    {
      blocks_[i].next_page = PAGES_PER_BLOCK;
    }

  nandemu_inject_bads();
  nandemu_inject_timebombs();
}

int nandemu_block_erase(block_id_t const blk)
{
  if (blk > NUM_BLOCKS)
    {
      fprintf(stderr, "nandemu: nandemu_block_erase called on invalid block: %d\n", blk);
      abort();
    }

  if (blocks_[blk].flags & BLOCK_BAD_MARK)
    {
      fprintf(stderr, "nandemu: nandemu_block_erase called on block which is marked bad: %d\n", blk);
      abort();
    }

  if (!stats_.frozen)
    {
      stats_.erase++;
    }

  blocks_[blk].next_page = 0;

  timebomb_tick_(blk);

  uint8_t * const block = flash_.blocks[blk].pages->page;

  if (blocks_[blk].flags & BLOCK_FAILED)
    {
      if (!stats_.frozen)
        {
          stats_.erase_fail++;
        }

      seq_gen_(blk * 57 + 29, block, BLOCK_SIZE);
      return NANDEMU_E_BAD_BLOCK;
    }

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

  if (blocks_[blk].flags & BLOCK_BAD_MARK)
    {
      fprintf(stderr, "\tnandemu: nandemu_block_prog called on block which is marked bad: %d\n", blk);
      return NANDEMU_E_BAD_BLOCK;
    }

  if (!stats_.frozen)
    {
      stats_.prog++;
    }


  timebomb_tick_(blk);

  uint8_t * dest = flash_.blocks[blk].bytes;

  if (blocks_[blk].flags & BLOCK_FAILED)
    {
      if (!stats_.frozen)
        {
          stats_.prog_fail++;
        }

      seq_gen_(blk * 57 + 29, dest, PAGE_SIZE);

      return NANDEMU_E_ECC;
    }

  memcpy(dest, data, BLOCK_SIZE);
  blocks_[blk].next_page = PAGES_PER_BLOCK;

  return NANDEMU_E_NONE;
}

int nandemu_page_read(block_id_t blk, page_id_t pg, uint8_t * dest)
{
  if (blk >= NUM_BLOCKS)
    {
      fprintf(stderr, "nandemu: nandemu_page_read called on invalid block: %d\n", blk);
      abort();
    }

  if (blocks_[blk].flags & (BLOCK_BAD_MARK | BLOCK_FAILED))
    {
      fprintf(stderr, "nandemu: nandemu_page_read called on damaged block: %d\n", blk);
      return NANDEMU_E_BAD_BLOCK;
    }

  if (!stats_.frozen)
    {
      stats_.read++;
      stats_.read_bytes += PAGE_SIZE;
    }

  timebomb_tick_(blk);

  uint8_t * src = flash_.blocks[blk].bytes;

  if (blocks_[blk].flags & BLOCK_FAILED)
    {
      if (!stats_.frozen)
        {
          stats_.prog_fail++;
        }

      seq_gen_(blk * 57 + 29, dest, PAGE_SIZE);

      return NANDEMU_E_ECC;
    }

  if (!stats_.frozen)
    {
      stats_.read++;
      stats_.read_bytes += BLOCK_SIZE;
    }

  memcpy(dest, src, BLOCK_SIZE);

  return NANDEMU_E_NONE;
}

void nandemu_inject_bads(void)
{
  srand(time(NULL) * 57 + 29);

  int count = 0;

  while (count < MAX_BAD_BLOCKS)
    {
      int const bno = rand() % NUM_BLOCKS;
      if ((blocks_[bno].flags & (BLOCK_BAD_MARK | BLOCK_FAILED)) == 0)
        {
          blocks_[bno].flags |= BLOCK_BAD_MARK | BLOCK_FAILED;
          stats_.is_bad++;
          stats_.mark_bad++;
          stats_.is_erased--;
          ++count;
        }
    }
}

void nandemu_inject_timebombs(void)
{
  srand(time(NULL) * 57 + 29);
  int count = 0;

  while (count < 2 * MAX_BAD_BLOCKS)
    {
      int const blk = rand() % NUM_BLOCKS;
      int const ttl = rand() % 31;

      if (set_timebomb_(blk, ttl))
        {
          ++count;
        }
    }
}

int nandemu_is_bad_number(void)
{
  return stats_.is_bad;
}

int nandemu_marked_bad_number(void)
{
  return stats_.mark_bad;
}

int nandemu_is_erased_number(void)
{
  return stats_.is_erased;
}

