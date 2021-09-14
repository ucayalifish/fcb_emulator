#include "tables.h"
#include "utils.h"

static uint8_t table_buffer_[BIG_1_TABLE_SIZE];

uint8_t const * generate_single_table(void)
{
  fill_random_buffer(table_buffer_, SINGLE_RECORD_TABLE_SIZE);
  return table_buffer_;
}

uint8_t const * generate_big_one(void)
{
  fill_random_buffer(table_buffer_, BIG_1_TABLE_SIZE);
  return table_buffer_;
}

uint8_t const * generate_big_two(void)
{
  fill_random_buffer(table_buffer_, BIG_2_TABLE_SIZE);
  return table_buffer_;
}
