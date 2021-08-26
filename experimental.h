#ifndef FCB_EMULATOR_EXPERIMENTAL_H
#define FCB_EMULATOR_EXPERIMENTAL_H

#include <stdbool.h>


#define NUM_OF_DECISIONS 100000

#define NUM_OF_DO_RESTORE_RUNS 1000

bool do_restore_test_generator(void);

struct decision_result_s
{
  int positive;
  int negative;
};

struct decision_result_s test_do_restore(void);

void test_number_of_attempts_before_failure(int * max, int * min, int * mean);

#endif //FCB_EMULATOR_EXPERIMENTAL_H
