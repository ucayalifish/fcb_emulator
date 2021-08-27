#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <nandemudef.h>

#include "block_state.h"


#define BLOCK_BAD_MARK_ (1U << 0U)

#define BLOCK_FAILED_ (1U << 1U)

#define BLOCK_FAILED_CNT_SHIFT_ 2U

#define BLOCK_FAILED_CNT_MASK_ (7U << BLOCK_FAILED_CNT_SHIFT_)

#define BLOCK_FAILED_CNT_WIDTH_ ((1U << 3U) - 1)

#define BLOCK_ERASED_ (1U << 5U)

_Static_assert(BLOCK_FAILED_CNT_WIDTH_ == 7, "");

#define TIMEBOMB_FIRED_ -1

struct block_status
{
  /* bits:
   *  0 - bad mark (permanent failure)
   *  1 - recoverable failure
   *  2-4 - recoverable failure count
   *  5 - is erased
   */
  unsigned int flags;
  /* the number of operations until permanent failure. */
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
  return (int) ((flags & BLOCK_FAILED_CNT_MASK_) >> BLOCK_FAILED_CNT_SHIFT_);
}

static inline void set_failure_count_(struct block_status * b, int const new_val)
{
  b->flags &= ~BLOCK_FAILED_CNT_MASK_;
  b->flags |= ((unsigned) new_val & BLOCK_FAILED_CNT_WIDTH_) << BLOCK_FAILED_CNT_SHIFT_;
}

static bool set_timebomb_(block_id_t const blk, int const ttl)
{
  assert(blk < NUM_BLOCKS);
  assert(ttl > 0);

  if ((blocks_[blk].flags & (BLOCK_BAD_MARK_ | BLOCK_FAILED_)) == 0)
    {
      blocks_[blk].timebomb = ttl;
      return true;
    }

  return false;
}

static void inject_bads_(void);

static void inject_timebombs_(void);

static bool roll_the_dice_(unsigned num, unsigned denom);

#if 0

static bool do_restore_(void);

#endif

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

  int const failed_count = get_failure_count_(b->flags);

  if (b->timebomb > 0)
    {
      b->timebomb--;

      if (!b->timebomb)
        {
          assert(get_failure_count_(b->flags) == 0);
          assert((b->flags & BLOCK_FAILED_) == 0);
          b->timebomb = TIMEBOMB_FIRED_;
        }
    }

  if ((b->flags & BLOCK_FAILED_) != 0 || b->timebomb == TIMEBOMB_FIRED_)
    {

      if (failed_count == 0)
        {
          set_failure_count_(b, 1);
          b->flags |= BLOCK_FAILED_;
        }
      else if (failed_count >= MAX_FAILURE_CNT)
        {
          b->flags |= BLOCK_FAILED_; // always failed after MAX_FAILURE_CNT failures
        }
      else
        {
          if (roll_the_dice_(RESTORE_ODDS_NUMERATOR, RESTORE_ODDS_DENOMINATOR))
            {
              b->flags &= ~BLOCK_FAILED_;
            }
          else
            {
              set_failure_count_(b, failed_count + 1);
              b->flags |= BLOCK_FAILED_;
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

bool block_state_fail_count_exhausted(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  return get_failure_count_(blocks_[blk].flags) >= MAX_FAILURE_CNT;
}

void block_state_mark_bad(block_id_t const blk)
{
  assert(blk < NUM_BLOCKS);

  blocks_[blk].flags &= ~BLOCK_FAILED_;
  blocks_[blk].flags |= BLOCK_BAD_MARK_;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "cert-msc50-cpp"

static void inject_bads_(void)
{
  int count = 0;

  while (count < MAX_BAD_BLOCKS)
    {
      int const bno = rand() % NUM_BLOCKS;
      if ((blocks_[bno].flags & BLOCK_BAD_MARK_) == 0)
        {
          block_state_mark_bad(bno);
          ++count;
        }
    }
}

static void inject_timebombs_(void)
{
  int count = 0;

  while (count < 2 * MAX_BAD_BLOCKS)
    {
      int const blk = rand() % NUM_BLOCKS;
      int const ttl = MIN_TIMEBOMB_TTL + rand() % MAX_TIMEBOMB_TTL;

      if (set_timebomb_(blk, ttl))
        {
          ++count;
        }
    }
}

#define SCALING_FACTOR_ 397U

#define MID_OF_DISTRIBUTION_ (0x7fffU * SCALING_FACTOR_)

#define CUT_OFF_ (MID_OF_DISTRIBUTION_ * RESTORE_ODDS_NUMERATOR / RESTORE_ODDS_DENOMINATOR);

static bool roll_the_dice_(unsigned const num, unsigned const denom)
{
  if (num == 0)
    {
      return false;
    }
  else if (denom == 0)
    {
      return true;
    }
  else
    {
      unsigned const cut_off = MID_OF_DISTRIBUTION_ * num / denom;
      unsigned const rn      = (unsigned) rand();
      unsigned const scaled  = rn * SCALING_FACTOR_;
      return (scaled % MID_OF_DISTRIBUTION_) > cut_off;
    }
}

#pragma clang diagnostic pop

int nandemu_number_of_failed(void)
{
  int count = 0;

  for (size_t i = 0; i < NUM_BLOCKS; ++i)
    {
      count += (int) ((blocks_[i].flags & BLOCK_FAILED_) != 0);
    }

  return count;
}


int nandemu_number_of_marked_bad(void)
{
  int count = 0;

  for (size_t i = 0; i < NUM_BLOCKS; ++i)
    {
      count += (int) ((blocks_[i].flags & BLOCK_BAD_MARK_) != 0);
    }

  return count;
}

int nandemu_is_erased_number(void)
{
  int count = 0;

  for (size_t i = 0; i < NUM_BLOCKS; ++i)
    {
      count += block_state_is_erased(i) ? 1 : 0;
    }

  return count;
}

int nandemu_timebombed_number(void)
{
  int count = 0;

  for (size_t i = 0; i < NUM_BLOCKS; ++i)
  {
    count += blocks_[i].timebomb > 0;
  }

  return count;
}
