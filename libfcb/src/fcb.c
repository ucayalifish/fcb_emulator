#include <fcb.h>
#include "utils.h"

void fcb_init_block_header(struct fcb_block_header_s * header)
{
  header->magic = FCB_MAGIC;
  header->ts_marker = timestamp_marker();
}
