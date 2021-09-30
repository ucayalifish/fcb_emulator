#include <malloc.h>
#include "tables.h"
#include "utils.h"

uint8_t const * generate_single_table(void)
{
  uint8_t * tbl = malloc(SINGLE_RECORD_TABLE_SIZE);

  if (tbl != NULL)
    {
      fill_random_buffer(tbl, SINGLE_RECORD_TABLE_SIZE);
      return tbl;
    }

  return NULL;
}

uint8_t const * generate_big_one(void)
{
  uint8_t * tbl = malloc(BIG_1_TABLE_SIZE);

  if (tbl != NULL)
    {
      fill_random_buffer(tbl, BIG_1_TABLE_SIZE);
      return tbl;
    }

  return NULL;
}

uint8_t const * generate_big_two(void)
{
  uint8_t * tbl = malloc(BIG_2_TABLE_SIZE);

  if (tbl != NULL)
    {
      fill_random_buffer(tbl, BIG_2_TABLE_SIZE);
      return tbl;
    }

  return NULL;
}

void free_table(void const * tbl)
{
  free((void *) tbl);
}
