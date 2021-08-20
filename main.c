#include <stdio.h>

#include <nandemu.h>
#include "utils.h"

static void nand_init_test()
{
  for (int run = 0; run < 10; ++run)
    {
      nandemu_reset();
      printf("Run %d: is bad-'%d', marked_bad-'%d', is erased-'%d'\n",
             run,
             nandemu_is_bad_number(),
             nandemu_marked_bad_number(),
             nandemu_is_erased_number());
    }

}

int main()
{
  nand_init_test();

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
