#include <stdio.h>

#include <nandemu.h>
#include "utils.h"

int main()
{
  nandemu_reset();

  nandemu_error_t err = NANDEMU_E_NONE;

  nandemu_block_erase(15, &err);

  int r;

  uint8_t buf[PAGE_SIZE];

  for (page_id_t pg = 0; pg < PAGES_PER_BLOCK; ++pg)
    {
      fill_random_buffer(pg * 57 + 29, buf, PAGE_SIZE);
      r = nandemu_page_prog(15, pg, buf);
    }

  r = nandemu_page_prog(15, PAGES_PER_BLOCK, buf);

  return 0;
}
