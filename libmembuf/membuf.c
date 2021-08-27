#include <nandemudef.h>
#include <string.h>
#include <assert.h>
#include "membuf.h"

_Static_assert(BLOCK_SIZE % 2 == 0, "OK");

#define BUFFER_SIZE_ (BLOCK_SIZE / sizeof(uint16_t))

// Buffer is optimized for external SDRAM with 16-bit access.
static uint16_t buffer_[BUFFER_SIZE_];

static ptrdiff_t offset_;

// Xorshift LFSR implementation from https://stackoverflow.com/a/65668437
static uint16_t xorshift16_(uint32_t in)
{
  in |= in == 0;
  in ^= (in & 0x07ffU) << 5U;
  in ^= in >> 7U;
  in ^= (in & 0x0003U) << 14U;
  return (uint16_t) in & 0xffffU;
}

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
  return (uint8_t *) buffer_ + offset_;
}

ptrdiff_t membuf_skip_bytes(size_t const how_many)
{
  if (offset_ + how_many < BLOCK_SIZE)
    {
      offset_ += (ptrdiff_t) how_many;
      return offset_;
    }

  return -1;
}

ptrdiff_t membuf_rewind(ptrdiff_t const to)
{
  assert(to >= 0);

  if (offset_ >= to)
    {
      offset_ = to;
      return offset_;
    }

  return -1;
}

ptrdiff_t membuf_write_bytes(uint8_t const * data, size_t const sz)
{
  if (data && offset_ + sz >= BLOCK_SIZE)
    {
      uint8_t * dest = (uint8_t *) buffer_ + sz;
      memcpy(dest, data, (ptrdiff_t) sz);
      offset_ += (ptrdiff_t) sz;
      return offset_;
    }

  return -1;
}

void membuf_shuffle_buffer(void)
{
  uint16_t lfsr = 0xace1U;

  for (size_t i = 0; i < BUFFER_SIZE_; ++i)
    {
      buffer_[i] ^= lfsr;
      lfsr = xorshift16_(lfsr);
    }
}

