#include "csv.h"
#include "unity.h"

void test_CSVReaderInitDestroy(void) {
  csvdialect dialect = csvdialect_init();
  csvreader  reader  = csvreader_init(dialect, NULL);

  TEST_ASSERT_NOT_NULL(reader);

  csvreader_close(&reader);
  TEST_ASSERT_NULL(reader);

  csvdialect_close(&dialect);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();

  RUN_TEST(test_CSVReaderInitDestroy);

  return UNITY_END();
}
