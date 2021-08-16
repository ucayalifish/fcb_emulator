#include <stdio.h>

#include <nandemu.h>

int main()
{
  nandemu_reset();

  nandemu_error_t err = NANDEMU_E_NONE;

  nandemu_block_erase(15, &err);

  return 0;
}
