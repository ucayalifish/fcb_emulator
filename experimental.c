#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <nandemudef.h>
#include <nandemu.h>

#include "experimental.h"

#define RESTORE_ODDS_NUMERATOR_ 3

#define RESTORE_ODDS_DENOMINATOR_ 13

#define SCALING_FACTOR_ 397U

#define MID_OF_DISTRIBUTION_ (0x7fffU * SCALING_FACTOR_)

#define CUT_OFF_ (MID_OF_DISTRIBUTION_ * RESTORE_ODDS_NUMERATOR_ / RESTORE_ODDS_DENOMINATOR_);

bool do_restore_test_generator(void)
{
  unsigned const rng    = (unsigned) rand();
  unsigned const scaled = rng * SCALING_FACTOR_;
  return (scaled % MID_OF_DISTRIBUTION_) > CUT_OFF_;
}

static bool decisions_[NUM_OF_DECISIONS];

static struct decision_result_s results_[NUM_OF_DO_RESTORE_RUNS];

static void do_restore_run_(int const run)
{
  memset(decisions_, 0, sizeof decisions_);
  struct decision_result_s * result = &results_[run];
  assert(result->positive == 0);
  assert(result->negative == 0);

  for (int test = 0; test < NUM_OF_DECISIONS; ++test)
    {
      bool const decision = do_restore_test_generator();
      decisions_[test] = decision;
    }

  for (int test = 0; test < NUM_OF_DECISIONS; ++test)
    {
      if (decisions_[test])
        {
          result->positive++;
        }
      else
        {
          result->negative++;
        }
    }

  printf("Run %d: positives: %d, negatives: %d\n", run, result->positive, result->negative);
}

struct decision_result_s test_do_restore(void)
{
  memset(results_, 0, sizeof results_);
  srand(time(NULL) * 57 + 29);

  for (int run = 0; run < NUM_OF_DO_RESTORE_RUNS; ++run)
    {
      do_restore_run_(run);
    }

  uint64_t positives = 0;
  uint64_t negatives = 0;

  for (int run = 0; run < NUM_OF_DO_RESTORE_RUNS; ++run)
    {
      positives += results_[run].positive;
      negatives += results_[run].negative;
    }

  return (struct decision_result_s) {.positive=(int) (positives / NUM_OF_DO_RESTORE_RUNS), .negative=(int) (negatives
                                                                                                            / NUM_OF_DO_RESTORE_RUNS)};
}

static int attempts_before_failure_[NUM_OF_DO_RESTORE_RUNS];

void test_number_of_attempts_before_failure(int * max, int * min, int * mean)
{
  memset(attempts_before_failure_, 0, sizeof attempts_before_failure_);
  srand(time(NULL) * 57 + 29);

  for (int run = 0; run < NUM_OF_DO_RESTORE_RUNS; ++run)
    {
      do
        {
          attempts_before_failure_[run]++;
        }
      while (do_restore_test_generator());
      printf("\tattempt %d: %d\n", run, attempts_before_failure_[run]);
    }

  uint64_t local_mean = 0;
  *max = 0;
  *min = INT_MAX;
  for (int run = 0; run < NUM_OF_DO_RESTORE_RUNS; ++run)
    {
      local_mean += attempts_before_failure_[run];

      if (attempts_before_failure_[run] > *max)
        {
          *max = attempts_before_failure_[run];
        }

      if (attempts_before_failure_[run] < *min)
        {
          *min = attempts_before_failure_[run];
        }
    }

  *mean = (int) (local_mean / NUM_OF_DO_RESTORE_RUNS);
}

static uint32_t xorshift32_(uint32_t in)
{
#if 0
  in |= in == 0;
  in ^= (in & 0x0007ffff) << 13U;
  in ^= in >> 17U;
  in ^= (in & 0x07ffffffU) << 5U;
  return in & 0xffffffffU;
#else
  in ^= in << 13U;
  in ^= in >> 17U;
  in ^= in << 5U;
  return in;
#endif
}

void test_xorshift32(void)
{
  uint32_t const start  = 0xace1U;
  uint32_t       value  = start;
  uint64_t       period = 0;

  do
    {
      value = xorshift32_(value);
      ++period;
//      printf("\tvalue = 0x%x, period = '%d'\n", value, period);
    }
  while (value != start && period < ULLONG_MAX - 1);

#ifdef __GNUC__
#ifdef __MINGW32__
  printf("test_xorshift32: period = '%I64u' of max '%I64u'\n", period, ULLONG_MAX);
#else
  printf("test_xorshift32: period = '%llu' of max '%llu'\n", period, ULLONG_MAX);
#endif
#endif
}

static uint16_t xorshift16_1_(uint32_t in)
{
  in |= in == 0;
  in ^= (in & 0x07ffU) << 5U;
  in ^= in >> 7U;
  in ^= (in & 0x0003U) << 14U;
  return (uint16_t) in & 0xffffU;
}

__attribute__((unused)) static uint16_t xorshift16_2_(uint32_t in)
{
  in ^= in >> 7U;
  in ^= in << 9U;
  in ^= in >> 13U;
  return (uint16_t) in & 0xffffU;
}

void test_xorshift16(void)
{
  uint16_t const start  = 0xace1U;
  uint16_t       value  = start;
  uint64_t       period = 0;

  do
    {
      value = xorshift16_1_(value);
      ++period;
//      printf("\tvalue = 0x%x, period = '%d'\n", value, period);
    }
  while (value != start && period < ULLONG_MAX - 1);

#ifdef __GNUC__
#ifdef __MINGW32__
  printf("test_xorshift32: period = '%I64u' of max '%I64u'\n", period, ULLONG_MAX);
#else
  printf("test_xorshift32: period = '%llu' of max '%llu'\n", period, ULLONG_MAX);
#endif
#endif
}

block_id_t exp_find_empty_block(block_id_t const start_from)
{
  block_id_t current = start_from;
  do
    {
      if (nandemu_is_not_bad_and_empty(current))
        {
          return current;
        }

      current = (current + 1) % NUM_BLOCKS;
    }
  while (current != start_from);

  return 0xffffffffU;
}
