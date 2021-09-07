#include <stdlib.h>
#include <stdio.h>

#include <nandemu.h>
#include <membuf.h>
#include <string.h>
#include "experimental.h"
#include "utils.h"
#include "libfcb/inc/fcb.h"

__attribute__((unused)) static void nand_init_test(void)
{
  nandemu_reset();
  printf("nand_init_test: is bad-'%d', marked_bad-'%d', is erased-'%d'\n",
         nandemu_number_of_failed(),
         nandemu_number_of_marked_bad(),
         nandemu_is_erased_number());
}

__attribute__((unused)) static void nand_erase_test(void)
{
  nandemu_reset();
  printf("nand_erase_test: is bad-'%d', marked_bad-'%d', is erased-'%d', is timebombed-'%d'\n",
         nandemu_number_of_failed(),
         nandemu_number_of_marked_bad(),
         nandemu_is_erased_number(),
         nandemu_timebombed_number());

  for (int run = 0; run < 1024; run++)
    {
      for (block_id_t blk = 0; blk < NUM_BLOCKS; ++blk)
        {
          if (nandemu_is_marked_bad(blk))
            {
//              printf("Block %d is factory marked bad\n", blk);
            }
          else
            {
              int r = nandemu_block_erase(blk);
              while (r != NANDEMU_E_NONE)
                {
                  if (r == NANDEMU_E_BAD_BLOCK)
                    {
                      printf("\tblock %d unrecoverable erase error %d\n", blk, r);
                      nandemu_mark_bad(blk);
                      break;
                    }

//                  printf("\tblock %d recoverable erase error %d\n", blk, r);
                  r = nandemu_block_erase(blk);
                }
            }
        }
      printf("!!!!! Run %d: is bad-'%d', marked_bad-'%d', is erased-'%d', timebombs left-'%d'\n",
             run,
             nandemu_number_of_failed(),
             nandemu_number_of_marked_bad(),
             nandemu_is_erased_number(),
             nandemu_timebombed_number());
    }
}

__attribute__((unused)) static void do_restore_experiment(void)
{
  struct decision_result_s means = test_do_restore();

  printf("Do restore means: positive-'%d', negative-'%d'\n", means.positive, means.negative);
}

__attribute__((unused)) static void num_of_attempts_experiment(void)
{
  int max, min, mean;
  test_number_of_attempts_before_failure(&max, &min, &mean);
  printf("Attempts before failure: max-'%d', min-'%d', mean-'%d'\n", max, min, mean);
}

__attribute__((unused)) static void membuf_shuffle_de_shuffle_test(void)
{
  membuf_reset();
  uint8_t * dest = membuf_current_position();
  size_t const copy_size = membuf_bytes_available();
  uint8_t      reference[copy_size];
  fill_random_buffer(reference, copy_size);
  membuf_write_bytes(reference, copy_size);
  membuf_shuffle_buffer();
  for (size_t i = 0; i < copy_size; ++i)
    {
      if (dest[i] == reference[i])
        {
          printf("Byte %d not shuffled: found 0x'%u', expected 0x'%u'\n", i, dest[i], reference[i]);
        }
    }
  membuf_shuffle_buffer();
  for (size_t i     = 0; i < copy_size; ++i)
    {
      if (dest[i] != reference[i])
        {
          printf("Byte %d not de-shuffled: found 0x'%u', expected 0x'%u'\n", i, dest[i], reference[i]);
        }
    }
  uint64_t    magic = FCB_MAGIC;
  (void) magic;
}

__attribute__((unused)) static void test_format_nand(void)
{
  nandemu_reset();

  for (block_id_t blk = 0; blk < NUM_BLOCKS; ++blk)
    {
      if (!nandemu_is_marked_bad(blk))
        {
          int r = nandemu_block_erase(blk);
          char const * result = r == 0 ? "success" : "failure";
          printf("<> Erasing block %d: %s, returned %d\n", blk, result, r);
        }
      else
        {
          printf("!!! Block %d is factory marked bad\n", blk);
        }
    }
}

__attribute__((unused)) static void test_empty_block_iteration(void)
{
  static int16_t found_[NUM_BLOCKS];

  for (int i = 0; i < 10000; ++i)
    {
      nandemu_reset();

      printf("NAND prepared, factory bad blocks: %d\n", nandemu_number_of_marked_bad());

      for (unsigned j = 0; j < NUM_BLOCKS; ++j)
        {
          found_[j] = -1;
        }

      block_id_t const first_protected = (i + rand()) % NUM_BLOCKS;
      block_id_t const last_protected = (first_protected + (rand() % 7) + 1) % NUM_BLOCKS;
      printf("Previously written data in blocks %d - %d\n", first_protected, last_protected);

      block_id_t       to_check  = (last_protected + 1) % NUM_BLOCKS;

      do
        {
          to_check = exp_find_empty_ready(to_check, first_protected);
          if (to_check != 0xffffffffU)
            {
              found_[to_check] = 0;
              to_check = (to_check + 1) % NUM_BLOCKS;
            }
        }
      while (to_check != 0xffffffffU);

      int count_good = 0;
      int count_bad  = 0;

      for (unsigned j = 0; j < NUM_BLOCKS; ++j)
        {
          if (found_[j] == -1)
            {
              ++count_bad;
            }
          else if (found_[j] == 0)
            {
              ++count_good;
            }
        }

      printf("NAND after iteration, bad blocks: %d\n", nandemu_number_of_marked_bad());
      printf("Iteration %d, protected blocks %d - %d: good-'%d', bad-'%d'\n\n", i, first_protected, last_protected, count_good, count_bad);
    }
}

static void test_first_write(void)
{
  nandemu_reset();

  printf("NAND prepared, factory bad blocks: %d\n", nandemu_number_of_marked_bad());

  block_id_t const found = exp_find_empty_ready(0, NUM_BLOCKS - 1);

  printf("Found and erased blok %d, bad blocks %d\n", found, nandemu_number_of_marked_bad());
}

int main()
{
//  do_restore_experiment();
//  num_of_attempts_experiment();
//  nand_init_test();
//  nand_erase_test();
//  test_xorshift32();
//  test_xorshift16();
//  membuf_shuffle_de_shuffle_test();
//  test_format_nand();
//  test_empty_block_iteration();
  test_first_write();

  return 0;
}
