#include <stdio.h>

#include <nandemu.h>

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
  printf("nand_erase_test: is bad-'%d', marked_bad-'%d', is erased-'%d'\n",
         nandemu_number_of_failed(),
         nandemu_number_of_marked_bad(),
         nandemu_is_erased_number());

  for (block_id_t blk = 0; blk < NUM_BLOCKS; ++blk)
    {
      if (nandemu_is_marked_bad(blk))
        {
          printf("Block %d is factory marked bad\n", blk);
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

              printf("\tblock %d recoverable erase error %d\n", blk, r);
              r = nandemu_block_erase(blk);
            }
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

  return 0;
}
