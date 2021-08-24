#include <stdio.h>

#include <nandemu.h>
#include "utils.h"

static void nand_init_test(void)
{
  for (int run = 0; run < 10; ++run)
    {
      nandemu_reset();
      printf("Run %d: is bad-'%d', marked_bad-'%d', is erased-'%d'\n",
             run,
             nandemu_number_of_failed(),
             nandemu_number_of_marked_bad(),
             nandemu_is_erased_number());
    }

}

static void nand_erase_test(void)
{
  nandemu_reset();

  for (block_id_t blk = 0; blk < NUM_BLOCKS; ++blk)
    {
      if (nandemu_is_marked_bad(blk))
        {
          printf("Block %d is factory marked bad\n", blk);
        }
      else
        {
          int r = nandemu_block_erase(blk);
          printf("Block %d erase result %d\n", blk, r);
        }
    }

    printf("Results: is bad-'%d', marked_bad-'%d', is erased-'%d'\n",
           nandemu_number_of_failed(),
           nandemu_number_of_marked_bad(),
           nandemu_is_erased_number());
}

int main()
{
  nand_init_test();
  nand_erase_test();

/*
  nandemu_error_t err = NANDEMU_E_NONE;

  nandemu_block_erase(15, &err);

  int r;

  uint8_t buf[PAGE_SIZE];

  for (page_id_t pg = 0; pg < PAGES_PER_BLOCK; ++pg)
    {
      fill_random_buffer(pg * 57 + 29, buf, PAGE_SIZE);
      r = nandemu_page_prog(15, pg, buf);
    }

  r = nandemu_block_prog(15, PAGES_PER_BLOCK, buf);
*/

  return 0;
}
