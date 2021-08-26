#include <nandemudef.h>
#include <string.h>
#include "membuf.h"

static uint8_t buffer_[BLOCK_SIZE];

static ptrdiff_t offset_;

void membuf_reset(void)
{
  memset(buffer_, 0xffU, sizeof buffer_);
  offset_ = 0;
}

size_t membuf_bytes_available(void)
{
  return BLOCK_SIZE - offset_;
}

uint8_t * membuf_current_position(void)
{
  return buffer_ + offset_;
}

int membuf_skip_bytes(ptrdiff_t const how_many)
{
  if (how_many > 0 && offset_ + how_many < BLOCK_SIZE)
    {
      offset_ += how_many;
      return 0;
    }

  return -1;
}

void membuf_rewind(void)
{
  offset_ = 0;
}
