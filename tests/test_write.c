#include "csv.h"
#include "unity.h"

#ifndef ZF_LOG_LEVEL
# define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#endif /* ZF_LOG_LEVEL */
#include "zf_log.h"

void test_CSVWriterInitDestroy(void) {
  csvdialect dialect = csvdialect_init();
  csvwriter  writer  = csvwriter_init(dialect, NULL);

  TEST_ASSERT_NOT_NULL(writer);

  csvwriter_close(&writer);
  TEST_ASSERT_NULL(writer);

  csvdialect_close(&dialect);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();

  RUN_TEST(test_CSVWriterInitDestroy);

  return UNITY_END();
}
