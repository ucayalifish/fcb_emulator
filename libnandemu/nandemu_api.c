#include <stdint.h>
#include <strings.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

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

_Static_assert(ZONES_PER_PAGE * ZONE_SIZE == PAGE_SIZE);

union block_u
{
  union page_u pages[PAGES_PER_BLOCK];
  uint8_t      bytes[BLOCK_SIZE];
};

_Static_assert(BLOCK_SIZE == PAGES_PER_BLOCK * PAGE_SIZE);

static struct sim_stats    stats_;
static struct block_status blocks_[NUM_BLOCKS];


static union flash_u
{
  union block_u blocks[NUM_BLOCKS];
  uint8_t       sectors[NUM_SECTORS][ZONE_SIZE];
  uint8_t       bytes[MEM_SIZE];
}                          flash_;

_Static_assert(MEM_SIZE == ZONE_SIZE * ZONES_PER_PAGE * PAGES_PER_BLOCK * NUM_BLOCKS);

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

void set_timebomb(block_id_t const blk, int const ttl)
{
  if (blk <= NUM_BLOCKS)
    {
      blocks_[blk].timebomb = ttl;
    }
}

void nandemu_reset(void)
{
  memset(&stats_, 0, sizeof(stats_));
  memset(blocks_, 0, sizeof(blocks_));
  memset(flash_.bytes, 0x55, sizeof(flash_.bytes));

  for (int i = 0; i < NUM_BLOCKS; i++)
    {
      blocks_[i].next_page = PAGES_PER_BLOCK;
    }
}

int nandemu_block_erase(block_id_t const blk, nandemu_error_t * error)
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
      set_error_(error, NANDEMU_E_BAD_BLOCK);
      return -1;
    }

  memset(block, 0xff, BLOCK_SIZE);
  return 0;

  return 0;
}

int nandemu_nand_sector_prog(page_id_t sector_id, uint8_t const * data, nandemu_error_t * err)
{
  block_id_t const bno = sector_id >> LOG2_ZONES_PER_BLOCK;
  page_id_t const  pno = sector_id & ((1ull << LOG2_ZONES_PER_BLOCK) - 1);

  if (bno >= NUM_BLOCKS)
    {
      fprintf(stderr, "sim: NAND_prog called on invalid block: %d\n", bno);
      abort();
    }

  if (blocks_[bno].flags & BLOCK_BAD_MARK)
    {
      fprintf(stderr, "nandemu: NAND_prog called on block which is marked bad: %d\n", bno);
      abort();
    }

  if (pno < blocks_[bno].next_page)
    {
      fprintf(stderr,
              "nandemu: NAND_prog: out-of-order sector programming. Block %d, sector %d (expected %d)\n",
              bno,
              pno,
              blocks_[bno].next_page);
      abort();
    }

  if (!stats_.frozen)
    {
      stats_.prog++;
    }

  blocks_[bno].next_page = (int) (pno + 1);

  timebomb_tick_(bno);

  uint8_t * sector = flash_.bytes + (ptrdiff_t) ((uint64_t) sector_id << LOG2_PAGE_SIZE);

  if (blocks_[bno].flags & BLOCK_FAILED)
    {
      if (!stats_.frozen)
        {
          stats_.prog_fail++;
        }

      seq_gen_(sector_id * 57 + 29, sector, PAGE_SIZE);
      set_error_(err, NANDEMU_E_BAD_BLOCK);
      return -1;
    }

  memcpy(sector, data, ZONE_SIZE);
  return 0;
}

int nandemu_page_prog(block_id_t const blk, page_id_t const pg, uint8_t const * data)
{
  if (blk >= NUM_BLOCKS)
    {
      fprintf(stderr, "nandemu: nandemu_page_prog called on invalid block: %d\n", blk);
      return NANDEMU_E_NOT_FOUND;
    }

  if (pg >= PAGES_PER_BLOCK)
    {
      fprintf(stderr, "nandemu: nandemu_page_prog called on invalid page: %d\n", pg);
      return NANDEMU_E_MAX;
    }

  if (blocks_[blk].flags & BLOCK_BAD_MARK)
    {
      fprintf(stderr, "nandemu: nandemu_page_prog called on block which is marked bad: %d\n", blk);
      return NANDEMU_E_BAD_BLOCK;
    }

  if (pg < blocks_[blk].next_page)
    {
      fprintf(stderr,
              "nandemu: nandemu_page_prog: out-of-order sector programming. Block %d, sector %d (expected %d)\n",
              blk,
              pg,
              blocks_[blk].next_page);
      abort();
    }

  if (!stats_.frozen)
    {
      stats_.prog++;
    }

  blocks_[blk].next_page = (int) (pg + 1);

  timebomb_tick_(blk);

  uint8_t * dest = flash_.blocks[blk].pages[pg].page;

  if (blocks_[blk].flags & BLOCK_FAILED)
    {
      if (!stats_.frozen)
        {
          stats_.prog_fail++;
        }

      seq_gen_(pg * 57 + 29, dest, PAGE_SIZE);

      return NANDEMU_E_BAD_BLOCK;
    }

  memcpy(dest, data, PAGE_SIZE);

  return NANDEMU_E_NONE;
}

int nandemu_page_read(block_id_t blk, page_id_t pg, uint8_t * dest)
{
  if (blk >= NUM_BLOCKS)
    {
      fprintf(stderr, "nandemu: nandemu_page_read called on invalid block: %d\n", blk);
      return NANDEMU_E_NOT_FOUND;
    }

  if (pg >= PAGES_PER_BLOCK)
    {
      fprintf(stderr, "nandemu: nandemu_page_read called on invalid page: %d\n", pg);
      return NANDEMU_E_MAX;
    }

  if (!stats_.frozen)
    {
      stats_.read++;
      stats_.read_bytes += PAGE_SIZE;
    }

  timebomb_tick_(blk);

  uint8_t * src = flash_.blocks[blk].pages[pg].page;

  if (blocks_[blk].flags & BLOCK_FAILED)
    {
      if (!stats_.frozen)
        {
          stats_.prog_fail++;
        }

      seq_gen_(pg * 57 + 29, dest, PAGE_SIZE);

      return NANDEMU_E_ECC;
    }

  if (!stats_.frozen)
    {
      stats_.read++;
      stats_.read_bytes += PAGE_SIZE;
    }

  memcpy(dest, src, PAGE_SIZE);

  return NANDEMU_E_NONE;
}

