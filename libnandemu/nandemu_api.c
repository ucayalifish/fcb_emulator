#include <stdint.h>
#include <strings.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "nandemu.h"

/**
 * NAND emulator based on one from https://github.com/dlbeer/dhara.
 */

#define LOG2_ZONE_SIZE    9u
#define LOG2_ZONES_PER_PAGE  2u
#define LOG2_PAGE_SIZE  (LOG2_ZONE_SIZE + LOG2_ZONES_PER_PAGE)
#define LOG2_PAGES_PER_BLOCK  6u
#define LOG2_ZONES_PER_BLOCK  (LOG2_ZONES_PER_PAGE + LOG2_PAGES_PER_BLOCK)
#define LOG2_BLOCK_SIZE (LOG2_PAGE_SIZE + LOG2_PAGES_PER_BLOCK)
#define NUM_BLOCKS   4096U

_Static_assert(LOG2_PAGE_SIZE == 11U);
_Static_assert(LOG2_BLOCK_SIZE == 17U);

#define ZONE_SIZE  (1u << LOG2_ZONE_SIZE)
#define ZONES_PER_PAGE (1u << LOG2_ZONES_PER_PAGE)
#define PAGE_SIZE    (1u << LOG2_PAGE_SIZE)
#define ZONES_PER_BLOCK  (1u << LOG2_ZONES_PER_BLOCK)
#define PAGES_PER_BLOCK    (1u << LOG2_PAGES_PER_BLOCK)
#define BLOCK_SIZE    (1u << LOG2_BLOCK_SIZE)
#define NUM_SECTORS (NUM_BLOCKS * PAGES_PER_BLOCK * ZONES_PER_PAGE)
#define MEM_SIZE    (NUM_BLOCKS * BLOCK_SIZE)

_Static_assert(ZONE_SIZE == 512U);
_Static_assert(PAGE_SIZE == 2048U);
_Static_assert(BLOCK_SIZE == 131072U);

#define BLOCK_BAD_MARK    0x01u
#define BLOCK_FAILED    0x02u

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

static void seq_gen_(unsigned int seed, uint8_t * buf, size_t length)
{
  size_t i;

  srand(seed);
  for (i = 0; i < length; i++)
    {
      buf[i] = rand();
    }
}

static inline void set_error_(nandemu_error_t * err, nandemu_error_t v)
{
  if (err)
    {
      *err = v;
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
      fprintf(stderr, "sim: NAND_erase called on invalid block: %d\n", blk);
      abort();
    }

  if (blocks_[blk].flags & BLOCK_BAD_MARK)
    {
      fprintf(stderr, "sim: NAND_erase called on block which is marked bad: %d\n", blk);
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

