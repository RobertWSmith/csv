#include <stdlib.h>
#include <stdio.h>

#include "csv.h"
#include "unity.h"

void test_CSVReaderInitDestroy(void) {
  printf("beginning test_CSVReaderInitDestroy\n");
  csvdialect dialect = csvdialect_init();
  csvreader  reader  = csvreader_init(dialect, NULL);

  // if no filepath is passed, should be a NULL reader
  TEST_ASSERT_NULL(reader);

  // file doesn't exist generates null reader
  reader = csvreader_init(dialect, "file-does-not-exist.csv");
  TEST_ASSERT_NULL(reader);

  // file does exist generate non-null reader
  reader = csvreader_init(dialect, "data/iris.csv");
  TEST_ASSERT_NOT_NULL(reader);

  printf("before csvreader_close\n");
  csvreader_close(&reader);
  TEST_ASSERT_NULL(reader);

  printf("before csvdialect_close\n");
  csvdialect_close(&dialect);
  printf("ending test_CSVReaderInitDestroy\n");
}

void test_CSVReaderIrisDataset(void) {
  csvdialect dialect      = csvdialect_init();
  csvreader  reader       = NULL;
  CSV_CHAR_TYPE char_type = CSV_CHAR;
  char **record           = NULL;
  size_t record_length    = 0;
  size_t i                = 0; // loop counter
  csvreturn rc;

  reader = csvreader_init(dialect, "data/iris.csv");

  TEST_ASSERT_NOT_NULL(reader);

  rc = csvreader_next_record(reader,
                             &char_type,
                             (csvrecord_type *)&record,
                             &record_length);

  // successful return
  TEST_ASSERT_TRUE(csv_success(rc));

  // no eof indicated
  TEST_ASSERT_FALSE(rc.io_eof);

  // there are exactly 5 fields in this dataset
  TEST_ASSERT_EQUAL_UINT(5U, record_length);

  // value equivalence for the header row
  TEST_ASSERT_EQUAL_STRING("sepal_length", record[0]);
  TEST_ASSERT_EQUAL_STRING("sepal_width",  record[1]);
  TEST_ASSERT_EQUAL_STRING("petal_length", record[2]);
  TEST_ASSERT_EQUAL_STRING("petal_width",  record[3]);
  TEST_ASSERT_EQUAL_STRING("species",      record[4]);

  // excellent, now must free the memory
  for (i = 0; i < record_length; ++i) {
    free(record[i]);
  }
  free(record);

  rc = csvreader_next_record(reader,
                             &char_type,
                             (csvrecord_type *)&record,
                             &record_length);

  // successful return
  TEST_ASSERT_TRUE(csv_success(rc));

  // no eof indicated
  TEST_ASSERT_FALSE(rc.io_eof);

  // there are exactly 5 fields in this dataset
  TEST_ASSERT_EQUAL_UINT(5U, record_length);

  // value equivalence for the header row
  TEST_ASSERT_EQUAL_STRING("5.1",    record[0]);
  TEST_ASSERT_EQUAL_STRING("3.5",    record[1]);
  TEST_ASSERT_EQUAL_STRING("1.4",    record[2]);
  TEST_ASSERT_EQUAL_STRING("0.2",    record[3]);
  TEST_ASSERT_EQUAL_STRING("setosa", record[4]);

  // excellent, now must free the memory
  for (i = 0; i < record_length; ++i) {
    free(record[i]);
  }
  free(record);

  while (true) {
    rc = csvreader_next_record(reader,
                               &char_type,
                               (csvrecord_type *)&record,
                               &record_length);

    // successful return
    TEST_ASSERT_TRUE(csv_success(rc));

    // no eof indicated
    TEST_ASSERT_FALSE(rc.io_eof);

    // there are exactly 5 fields in this dataset
    TEST_ASSERT_EQUAL_UINT(5U, record_length);

    // excellent, now must free the memory
    for (i = 0; i < record_length; ++i) {
      free(record[i]);
    }
    free(record);
  }

  csvreader_close(&reader);
  TEST_ASSERT_NULL(reader);

  csvdialect_close(&dialect);
}

int main(int argc, char **argv) {
  printf("Before all tests\n");
  UNITY_BEGIN();

  RUN_TEST(test_CSVReaderInitDestroy);
  RUN_TEST(test_CSVReaderIrisDataset);

  return UNITY_END();
}
