#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <nandemudef.h>

#include "block_state.h"

struct block_status
  {
  unsigned int flags;
  /* Timebomb counter: if non-zero, this is the number of
	 * operations until permanent failure.
	 */
  int          timebomb;
  int          erased;
  int          erase_failed;
  int          read;
  int          read_failed;
  int          proged;
  int          prog_failed;
  };

static struct block_status blocks_[NUM_BLOCKS];

static bool set_timebomb_(block_id_t const blk, int const ttl)
{
  assert(blk <= NUM_BLOCKS);

  if ((blocks_[blk].flags & (BLOCK_BAD_MARK | BLOCK_FAILED)) == 0)
  {
    blocks_[blk].timebomb = ttl;
    return true;
  }

  return false;
}

static void nandemu_inject_bads_(void);

static void nandemu_inject_timebombs_(void);

void block_state_reset(void)
{
  memset(blocks_, 0, sizeof(blocks_));
  nandemu_inject_bads_();
  nandemu_inject_timebombs_();
}

bool block_state_is_marked_bad(block_id_t const blk)
{
  assert(blk <= NUM_BLOCKS);

  return (blocks_[blk].flags & BLOCK_BAD_MARK) != 0;
}

void block_state_timebomb_tick(block_id_t const blk)
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

bool block_state_failed(block_id_t const blk)
{
  assert(blk <= NUM_BLOCKS);

  return (blocks_[blk].flags & BLOCK_FAILED) != 0;
}

void block_state_inc_erase_failed(block_id_t blk)
{
  assert(blk <= NUM_BLOCKS);

  blocks_[blk].erase_failed++;
}

void block_state_inc_erase_success(block_id_t const blk)
{
  assert(blk <= NUM_BLOCKS);

  blocks_[blk].erased++;
}

void block_state_inc_prog_failed(block_id_t const blk)
{
  assert(blk <= NUM_BLOCKS);

  blocks_[blk].prog_failed++;
}

void block_state_inc_prog_success(block_id_t const blk)
{
  assert(blk <= NUM_BLOCKS);

  blocks_[blk].proged++;
}

void block_state_inc_read_failed(block_id_t const blk)
{
  assert(blk <= NUM_BLOCKS);

  blocks_[blk].read_failed++;
}

void block_state_inc_read_success(block_id_t const blk)
{
  assert(blk <= NUM_BLOCKS);

  blocks_[blk].read++;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "cert-msc50-cpp"

static void nandemu_inject_bads_(void)
{
  srand(time(NULL) * 57 + 29);

  int count = 0;

  while (count < MAX_BAD_BLOCKS)
  {
    int const bno = rand() % NUM_BLOCKS;
    if ((blocks_[bno].flags & (BLOCK_BAD_MARK | BLOCK_FAILED)) == 0)
    {
      blocks_[bno].flags |= BLOCK_BAD_MARK | BLOCK_FAILED;
      ++count;
    }
  }
}

static void nandemu_inject_timebombs_(void)
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

#pragma clang diagnostic pop

int nandemu_number_of_bad(void)
{
  int count = 0;

  for (int i = 0; i < NUM_BLOCKS; ++i)
  {
    count += (int) ((blocks_[i].flags & (BLOCK_BAD_MARK | BLOCK_FAILED)) != 0);
  }

  return count;
}


int nandemu_number_of_marked_bad(void)
{
  int count = 0;

  for (int i = 0; i < NUM_BLOCKS; ++i)
  {
    count += (int) ((blocks_[i].flags & BLOCK_BAD_MARK) != 0);
  }

  return count;
}
