#include <nandemudef.h>
#include <string.h>
#include "membuf.h"

static uint8_t buffer_[BLOCK_SIZE];

void membuf_reset(void)
{
  memset(buffer_, 0xffU, sizeof buffer_);
}
