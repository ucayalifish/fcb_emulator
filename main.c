#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <nandemu.h>
#include "experimental.h"

static void nand_init_test(void)
{
  nandemu_reset();
  printf("nand_init_test: is bad-'%d', marked_bad-'%d', is erased-'%d'\n",
         nandemu_number_of_failed(),
         nandemu_number_of_marked_bad(),
         nandemu_is_erased_number());
}

static void nand_erase_test(void)
{
  nandemu_reset();
  printf("nand_erase_test: is bad-'%d', marked_bad-'%d', is erased-'%d', is timebombed-'%d'\n",
         nandemu_number_of_failed(),
         nandemu_number_of_marked_bad(),
         nandemu_is_erased_number(),
         nandemu_timebombed_number());

  for (int run = 0; run < 256; run++)
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

static void do_restore_experiment(void)
{
  struct decision_result_s means = test_do_restore();

  printf("Do restore means: positive-'%d', negative-'%d'\n", means.positive, means.negative);
}

static void num_of_attempts_experiment(void)
{
  int max, min, mean;
  test_number_of_attempts_before_failure(&max, &min, &mean);
  printf("Attempts before failure: max-'%d', min-'%d', mean-'%d'\n", max, min, mean);
}

int main()
{
//  do_restore_experiment();
//  num_of_attempts_experiment();
//  nand_init_test();
  nand_erase_test();

  return 0;
}
