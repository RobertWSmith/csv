#include "csv.h"
#include "unity.h"

void test_CSVFieldInitDestroy(void) {
  csvfield field = csvfield_init();

  TEST_ASSERT_NOT_NULL(field);

  csvfield_close(&field);
  TEST_ASSERT_NULL(field);
}

int test_field(int argc, char **argv) {
  UNITY_BEGIN();

  RUN_TEST(test_CSVFieldInitDestroy);

  return UNITY_END();
}
