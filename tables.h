#ifndef FCB_EMULATOR_TABLES_H
#define FCB_EMULATOR_TABLES_H

#include <stdint.h>

typedef enum table_ids_e
{
  SINGLE, BIG_1, BIG_2
} TABLE_IDS;

#define SINGLE_RECORD_TABLE_SIZE 180

#define BIG_1_RECORD_SIZE 48
#define BIG_1_RECORD_NUM 300000
#define BIG_1_TABLE_SIZE (BIG_1_RECORD_SIZE * BIG_1_RECORD_NUM)

#define BIG_2_RECORD_SIZE 200
#define BIG_2_RECORD_NUM 5000
#define BIG_2_TABLE_SIZE (BIG_2_RECORD_SIZE * BIG_2_RECORD_NUM)

uint8_t const * generate_single_table(void);

uint8_t const * generate_big_one(void);

uint8_t const * generate_big_two(void);

#endif //FCB_EMULATOR_TABLES_H
