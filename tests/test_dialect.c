#include "csv.h"
#include "dialect_private.h"
#include "unity.h"

void test_CSVDialectInitDestroy(void) {
  csvdialect dialect = csvdialect_init();

  TEST_ASSERT_NOT_NULL(dialect);

  csvdialect_close(&dialect);
  TEST_ASSERT_NULL(dialect);
}

int test_dialect(int argc, char **argv) {
  UNITY_BEGIN();

  RUN_TEST(test_CSVDialectInitDestroy);

  return UNITY_END();
}
