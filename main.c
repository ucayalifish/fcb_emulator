#include <stdlib.h>
#include <stdio.h>

#include <nandemu.h>
#include <membuf.h>
#include <string.h>
#include <assert.h>
#include "experimental.h"
#include "utils.h"
#include "libfcb/inc/fcb.h"
#include "tables.h"

__attribute__((unused)) static void nand_init_test(void)
{
  nandemu_reset();
  printf("nand_init_test: is bad-'%d', marked_bad-'%d', is erased-'%d'\n",
         nandemu_number_of_failed(),
         nandemu_number_of_marked_bad(),
         nandemu_is_erased_number());
}

__attribute__((unused)) static void nand_erase_test(void)
{
  nandemu_reset();
  printf("nand_erase_test: is bad-'%d', marked_bad-'%d', is erased-'%d', is timebombed-'%d'\n",
         nandemu_number_of_failed(),
         nandemu_number_of_marked_bad(),
         nandemu_is_erased_number(),
         nandemu_timebombed_number());

  for (int run = 0; run < 1024; run++)
    {
      for (block_id_t blk = 0; blk < NUM_BLOCKS; ++blk)
        {
          if (nandemu_is_marked_bad(blk))
            {
//              printf("Block %d is factory marked bad\n", blk);
            }
          else
            {
              int r = nandemu_block_erase(blk);
              while (r != NANDEMU_E_NONE)
                {
                  if (r == NANDEMU_E_BAD_BLOCK)
                    {
                      printf("\tblock %d unrecoverable erase error %d\n", blk, r);
                      nandemu_mark_bad(blk);
                      break;
                    }

//                  printf("\tblock %d recoverable erase error %d\n", blk, r);
                  r = nandemu_block_erase(blk);
                }
            }
        }
      printf("!!!!! Run %d: is bad-'%d', marked_bad-'%d', is erased-'%d', timebombs left-'%d'\n",
             run,
             nandemu_number_of_failed(),
             nandemu_number_of_marked_bad(),
             nandemu_is_erased_number(),
             nandemu_timebombed_number());
    }
}

__attribute__((unused)) static void do_restore_experiment(void)
{
  struct decision_result_s means = test_do_restore();

  printf("Do restore means: positive-'%d', negative-'%d'\n", means.positive, means.negative);
}

__attribute__((unused)) static void num_of_attempts_experiment(void)
{
  int max, min, mean;
  test_number_of_attempts_before_failure(&max, &min, &mean);
  printf("Attempts before failure: max-'%d', min-'%d', mean-'%d'\n", max, min, mean);
}

__attribute__((unused)) static void membuf_shuffle_de_shuffle_test(void)
{
  membuf_reset();
  uint8_t * dest = membuf_current_position();
  size_t const copy_size = membuf_bytes_available();
  uint8_t      reference[copy_size];
  fill_random_buffer(reference, copy_size);
  membuf_write_bytes(reference, copy_size);
  membuf_shuffle_buffer();
  for (size_t i = 0; i < copy_size; ++i)
    {
      if (dest[i] == reference[i])
        {
          printf("Byte %d not shuffled: found 0x'%u', expected 0x'%u'\n", i, dest[i], reference[i]);
        }
    }
  membuf_shuffle_buffer();
  for (size_t i     = 0; i < copy_size; ++i)
    {
      if (dest[i] != reference[i])
        {
          printf("Byte %d not de-shuffled: found 0x'%u', expected 0x'%u'\n", i, dest[i], reference[i]);
        }
    }
  uint64_t    magic = FCB_MAGIC;
  (void) magic;
}

__attribute__((unused)) static void test_format_nand(void)
{
  nandemu_reset();

  for (block_id_t blk = 0; blk < NUM_BLOCKS; ++blk)
    {
      if (!nandemu_is_marked_bad(blk))
        {
          int r = nandemu_block_erase(blk);
          char const * result = r == 0 ? "success" : "failure";
          printf("<> Erasing block %d: %s, returned %d\n", blk, result, r);
        }
      else
        {
          printf("!!! Block %d is factory marked bad\n", blk);
        }
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc50-cpp"

__attribute__((unused)) static void test_empty_block_iteration(void)
{
  static int16_t found_[NUM_BLOCKS];

  for (int i = 0; i < 10000; ++i)
    {
      nandemu_reset();

      printf("NAND prepared, factory bad blocks: %d\n", nandemu_number_of_marked_bad());

      for (unsigned j = 0; j < NUM_BLOCKS; ++j)
        {
          found_[j] = -1;
        }

      block_id_t const first_protected = (i + rand()) % NUM_BLOCKS;
      block_id_t const last_protected  = (first_protected + (rand() % 7) + 1) % NUM_BLOCKS;
      printf("Previously written data in blocks %d - %d\n", first_protected, last_protected);

      block_id_t to_check = (last_protected + 1) % NUM_BLOCKS;

      do
        {
          to_check = nandemu_find_and_erase_next_block(to_check, first_protected);
          if (to_check != 0xffffffffU)
            {
              found_[to_check] = 0;
              to_check = (to_check + 1) % NUM_BLOCKS;
            }
        }
      while (to_check != 0xffffffffU);

      int count_good = 0;
      int count_bad  = 0;

      for (unsigned j = 0; j < NUM_BLOCKS; ++j)
        {
          if (found_[j] == -1)
            {
              ++count_bad;
            }
          else if (found_[j] == 0)
            {
              ++count_good;
            }
        }

      printf("NAND after iteration, bad blocks: %d\n", nandemu_number_of_marked_bad());
      printf("Iteration %d, protected blocks %d - %d: good-'%d', bad-'%d'\n\n", i, first_protected, last_protected, count_good, count_bad);
    }
}

#pragma clang diagnostic pop

__attribute__((unused)) static void test_first_write(void)
{
  nandemu_reset();

  printf("NAND prepared, factory bad blocks: %d\n", nandemu_number_of_marked_bad());

  block_id_t found = exp_find_empty_ready(0, NUM_BLOCKS);

  printf("Found and erased block %d, bad blocks %d\n", found, nandemu_number_of_marked_bad());

  nandemu_reset();

  for (block_id_t blk = 0; blk < NUM_BLOCKS - 1; ++blk)
    {
      if (!nandemu_is_marked_bad(blk))
        {
          nandemu_mark_bad(blk);
        }
    }

  if (nandemu_is_marked_bad(4095))
    {
      printf("%s\n", "!!! the only block is bad!");
      exit(-1);
    }

  printf("NAND prepared, factory bad blocks: %d\n", nandemu_number_of_marked_bad());

  found = exp_find_empty_ready(0, NUM_BLOCKS);


  printf("Found and erased block %d, bad blocks %d\n", found, nandemu_number_of_marked_bad());
}

__attribute__((unused)) static void test_create_block(void)
{
  membuf_reset();
  struct fcb_block_header_s block_header = {.ordinal = 0};
  fcb_init_block_header(&block_header);
  char out_magic[sizeof(uint64_t) + 1];
  memset(out_magic, 0, sizeof out_magic);
  memcpy(out_magic, &block_header.magic, sizeof block_header.magic);
#ifdef __GNUC__
#ifdef __MINGW32__
  printf("Block header: magic='%s', ts='0x%I64u', corrected ts: '0x%I64u'\n",
         out_magic,
         block_header.ts_marker,
         block_header.ts_marker >> 7u);
#else
  printf("Block header: magic='%s', ts='0x%llu', corrected ts: '0x%llu'\n",
         out_magic,
         block_header.ts_marker,
         block_header.ts_marker >> 7u);
#endif
#endif
  time_t tmp_time = (time_t) (block_header.ts_marker >> 7u);
  char   tmp_buf[26];
  struct tm * tm_info = localtime(&tmp_time);
  strftime(tmp_buf, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  printf("Corrected ts: '%s'\n", tmp_buf);

  ptrdiff_t current = membuf_skip_bytes(sizeof(struct fcb_block_header_s));
  block_header.data_offset = current;
  printf("Block start data offset: %d\n", current);

  struct fcb_table_header_s th        = {.magic=TBL_MAGIC, .record_size=SINGLE_RECORD_TABLE_SIZE, .num_records=1, .first_record_id=0};
  ptrdiff_t const           crc_start = current;
  current = membuf_write_bytes((uint8_t const *) &th, sizeof th);
  uint8_t const * ptbl = generate_single_table();
  current = membuf_write_bytes(ptbl, SINGLE_RECORD_TABLE_SIZE);
  size_t       avail      = membuf_bytes_available();
  size_t const num_of_rec = (avail - sizeof th) / BIG_1_RECORD_SIZE;
  size_t const slop       = (avail - sizeof th) % BIG_1_RECORD_SIZE;
  assert(num_of_rec * BIG_1_RECORD_SIZE + slop + sizeof th == avail);
  th      = (struct fcb_table_header_s) {.magic=TBL_MAGIC, .record_size= BIG_1_RECORD_SIZE, .num_records=num_of_rec, .first_record_id=0};
  current = membuf_write_bytes((uint8_t const *) &th, sizeof th);
  ptbl    = generate_big_one();
  current = membuf_write_bytes(ptbl, num_of_rec * BIG_1_RECORD_SIZE);
  block_header.crc32 = membuf_calc_crc32(crc_start, current);
  membuf_rewind(0);
  (void) membuf_write_bytes((uint8_t const *) &block_header, sizeof block_header);
  membuf_shuffle_buffer();

  membuf_shuffle_buffer();
  uint32_t crc = membuf_calc_crc32(crc_start, current);
  assert(block_header.crc32 == crc);
}

int main()
{
//  do_restore_experiment();
//  num_of_attempts_experiment();
//  nand_init_test();
//  nand_erase_test();
//  test_xorshift32();
//  test_xorshift16();
//  membuf_shuffle_de_shuffle_test();
//  test_format_nand();
//  test_empty_block_iteration();
//  test_first_write();
  test_create_block();

  return 0;
}
