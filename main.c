#include <stdio.h>

#include <nandemu.h>

int main()
{
  int r = nandemu_init();

  printf("Hello, World! %d\n", r);
  return 0;
}
