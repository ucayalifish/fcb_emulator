#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <nandemudef.h>

#include "block_state.h"


#define BLOCK_BAD_MARK_ (1U << 0)

#define BLOCK_FAILED_MASK_ 14u

#define BLOCK_FAILED_ (1U << 1)

#define BLOCK_ERASED_ (1U << 4U)

struct block_status
{
  /* bits:
   *  0 - bad mark (permanent failure)
   *  1 - recoverable failure
   *  2-4 - recoverable failure count
   *  5 - is erased
   */
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

static inline int get_failure_count_(unsigned const flags)
{
  return (int) ((flags & BLOCK_FAILED_MASK_) >> 1U);
}

static inline void set_failure_count_(struct block_status * b, int const new_val)
{
  b->flags &= ~BLOCK_FAILED_MASK_;
  b->flags |= ((unsigned) new_val & 0x03U) << 2U;
}

static bool set_timebomb_(block_id_t const blk, int const ttl)
{
  assert(blk < NUM_BLOCKS);

  if ((blocks_[blk].flags & (BLOCK_BAD_MARK_ | BLOCK_FAILED_)) == 0)
    {
      blocks_[blk].timebomb = ttl;
      return true;
    }

  return false;
}

static void inject_bads_(void);

static void inject_timebombs_(void);

static bool do_restore_(void);

void block_state_reset(void)
{
  memset(blocks_, 0, sizeof(blocks_));
  inject_bads_();
  inject_timebombs_();
}

bool block_state_is_marked_bad(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  return (blocks_[blk].flags & BLOCK_BAD_MARK_) != 0;
}

void block_state_timebomb_tick(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  struct block_status * b = &blocks_[blk];

  if (b->timebomb)
    {
      b->timebomb--;

      if (!b->timebomb)
        {
          int const failed_count = get_failure_count_(b->flags);
          switch (failed_count)
            {
              case 0:
                {
                  set_failure_count_(b, 1);
                  b->flags |= BLOCK_FAILED_;
                }
              break;
              case 3:
                {
                  b->flags |= BLOCK_FAILED_;
                }
              break;
              default:
                {
                  int const count_to_set = do_restore_() ? 0 : failed_count + 1;

                  if (do_restore_())
                    {
                      set_failure_count_(b, 1);
                      b->flags |= ~BLOCK_FAILED_;
                    }
                  else
                    {
                      set_failure_count_(b, failed_count + 1);
                      b->flags |= BLOCK_FAILED_;
                    }
                }
            }
        }
    }
}

bool block_state_failed(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  return (blocks_[blk].flags & BLOCK_FAILED_) != 0;
}

void block_state_inc_erase_failed(block_id_t blk)
{
  assert(blk < NUM_BLOCKS);

  blocks_[blk].erase_failed++;
}

void block_state_inc_erase_success(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  blocks_[blk].erased++;
}

void block_state_inc_prog_failed(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  blocks_[blk].prog_failed++;
}

void block_state_inc_prog_success(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  blocks_[blk].proged++;
}

void block_state_inc_read_failed(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  blocks_[blk].read_failed++;
}

void block_state_inc_read_success(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  blocks_[blk].read++;
}

bool block_state_is_erased(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  return (blocks_[blk].flags & BLOCK_ERASED_) != 0;
}

void block_state_set_erased(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  blocks_[blk].flags |= BLOCK_ERASED_;
}

void block_state_clear_erased(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  blocks_[blk].flags &= ~BLOCK_ERASED_;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "cert-msc50-cpp"

static void inject_bads_(void)
{
  srand(time(NULL) * 57 + 29);

  int count = 0;

  while (count < MAX_BAD_BLOCKS)
    {
      int const bno = rand() % NUM_BLOCKS;
      if ((blocks_[bno].flags & (BLOCK_BAD_MARK_ | BLOCK_FAILED_)) == 0)
        {
          blocks_[bno].flags |= BLOCK_BAD_MARK_ | BLOCK_FAILED_;
          ++count;
        }
    }
}

static void inject_timebombs_(void)
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

#define MID_OF_DISTRIBUTION_ (0x7fffffffULL * 397ull)

#define SCALING_FACTOR_ 397ull

static bool do_restore_(void)
{
#if RESTORE_ODDS_NUMERATOR == 0
  return false;
#elif RESTORE_ODDS_DENUMINATOR == 0
  return true;
#else
  srand(time(NULL) * 57 + 29);
  uint64_t const cut_off = MID_OF_DISTRIBUTION_ * RESTORE_ODDS_NUMERATOR / RESTORE_ODDS_DENUMINATOR;
  uint64_t const rng     = (unsigned) rand() * SCALING_FACTOR_;
  return rng % MID_OF_DISTRIBUTION_ <= cut_off;
#endif
}

#pragma clang diagnostic pop

int nandemu_number_of_failed(void)
{
  int count = 0;

  for (int i = 0; i < NUM_BLOCKS; ++i)
    {
      count += (int) ((blocks_[i].flags & BLOCK_FAILED_) != 0);
    }

  return count;
}


int nandemu_number_of_marked_bad(void)
{
  int count = 0;

  for (int i = 0; i < NUM_BLOCKS; ++i)
    {
      count += (int) ((blocks_[i].flags & BLOCK_BAD_MARK_) != 0);
    }

  return count;
}

int nandemu_is_erased_number(void)
{
  int count = 0;

  for (int i = 0; i < NUM_BLOCKS; ++i)
  {
    count += (int) ((blocks_[i].flags & BLOCK_ERASED_) != 0);
  }

  return count;
}
