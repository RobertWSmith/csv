#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef ZF_LOG_LEVEL
#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#endif /* ZF_LOG_LEVEL */
#include "zf_log.h"

#include "csv.h"
#include "unity.h"

FILE *_log_file;

static void file_output_callback(const zf_log_message *msg, void *arg) {
  (void)arg;
  *msg->p = '\n';
  fwrite(msg->buf, msg->p - msg->buf + 1, 1, _log_file);
  fflush(_log_file);
}

static void file_output_close(void) { fclose(_log_file); }

static void file_output_open(const char *const log_path) {
  _log_file = fopen(log_path, "w");

  if (!_log_file) {
    ZF_LOGW("Failed to open log file %s", log_path);
    return;
  }
  atexit(file_output_close);
  zf_log_set_output_v(ZF_LOG_PUT_STD, 0, file_output_callback);
}

void test_CSVReaderInitDestroy(void) {
  ZF_LOGI("`test_CSVReaderInitDestroy` called");
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

  ZF_LOGD("before calling `csvreader_close`");
  csvreader_close(&reader);
  TEST_ASSERT_NULL(reader);

  ZF_LOGD("before calling `csvdialect_close`");
  csvdialect_close(&dialect);
  ZF_LOGI("`test_CSVReaderInitDestroy` completed");
}

void test_CSVReaderIrisDataset(void) {
  ZF_LOGI("`test_CSVReaderIrisDataset` called");
  csvdialect    dialect       = csvdialect_init();
  csvreader     reader        = NULL;
  CSV_CHAR_TYPE char_type     = CSV_CHAR;
  char **       record        = NULL;
  size_t        record_length = 0;
  size_t        i             = 0;  // loop counter
  csvreturn     rc;

  ZF_LOGI("`csvreader_init` before");
  reader = csvreader_init(dialect, "data/iris.csv");
  ZF_LOGI("`csvreader_init` completed");

  TEST_ASSERT_NOT_NULL(reader);

  ZF_LOGI("record length: %lu", record_length);
  ZF_LOGI("`csvreader_next_record` called");
  rc = csvreader_next_record(
      reader, &char_type, (csvrecord_type *)&record, &record_length);
  ZF_LOGI("`csvreader_next_record` called");

  // successful return
  TEST_ASSERT_TRUE(csv_success(rc));

  // no eof indicated
  ZF_LOGI("`csvreturn.io_eof`: %s", rc.io_eof ? "true" : "false");
  TEST_ASSERT_FALSE(rc.io_eof);

  // no error indicated
  ZF_LOGI("`csvreturn.io_error`: %s", rc.io_error ? "true" : "false");
  TEST_ASSERT_FALSE(rc.io_error);

  // there are exactly 5 fields in this dataset
  ZF_LOGI("record length: %lu", record_length);
  TEST_ASSERT_EQUAL_UINT(5U, record_length);

  // value equivalence for the header row
  TEST_ASSERT_EQUAL_STRING("sepal_length", record[0]);
  TEST_ASSERT_EQUAL_STRING("sepal_width", record[1]);
  TEST_ASSERT_EQUAL_STRING("petal_length", record[2]);
  TEST_ASSERT_EQUAL_STRING("petal_width", record[3]);
  TEST_ASSERT_EQUAL_STRING("species", record[4]);

  // excellent, now must free the memory
  for (i = 0; i < record_length; ++i) {
    free(record[i]);
  }
  free(record);

  rc = csvreader_next_record(
      reader, &char_type, (csvrecord_type *)&record, &record_length);

  // successful return
  TEST_ASSERT_TRUE(csv_success(rc));

  // no eof indicated
  ZF_LOGI("`csvreturn.io_eof`: %s", rc.io_eof ? "true" : "false");
  TEST_ASSERT_FALSE(rc.io_eof);

  // no error indicated
  ZF_LOGI("`csvreturn.io_error`: %s", rc.io_error ? "true" : "false");
  TEST_ASSERT_FALSE(rc.io_error);

  // there are exactly 5 fields in this dataset
  ZF_LOGI("record length: %zu", record_length);
  TEST_ASSERT_EQUAL_UINT(5U, record_length);

  // value equivalence for the header row
  TEST_ASSERT_EQUAL_STRING("5.1", record[0]);
  TEST_ASSERT_EQUAL_STRING("3.5", record[1]);
  TEST_ASSERT_EQUAL_STRING("1.4", record[2]);
  TEST_ASSERT_EQUAL_STRING("0.2", record[3]);
  TEST_ASSERT_EQUAL_STRING("setosa", record[4]);

  // excellent, now must free the memory
  for (i = 0; i < record_length; ++i) {
    free(record[i]);
  }
  free(record);

  int cnt = 0;

  while (true) {
    ZF_LOGI("Row #%d", ++cnt);
    rc = csvreader_next_record(
        reader, &char_type, (csvrecord_type *)&record, &record_length);

    if (csv_success(rc)) {
      // there are exactly 5 fields in this dataset
      TEST_ASSERT_EQUAL_UINT(5U, record_length);

      // excellent, now must free the memory
      for (i = 0; i < record_length; ++i) {
        free(record[i]);
      }
      free(record);
    } else {
      break;
    }
  }

  ZF_LOGI("`csvreader_close` before");
  csvreader_close(&reader);
  TEST_ASSERT_NULL(reader);
  ZF_LOGI("`csvreader_close` completed");

  ZF_LOGI("`csvdialect_close` before");
  csvdialect_close(&dialect);
  ZF_LOGI("`csvdialect_close` completed");
  ZF_LOGI("`test_CSVReaderIrisDataset` completed");
}

int main(int argc, char **argv) {
  int output = 0;

  file_output_open("test_reader.log");

  ZF_LOGI("Beginning CSV Reader Test");

  UNITY_BEGIN();

  RUN_TEST(test_CSVReaderInitDestroy);
  RUN_TEST(test_CSVReaderIrisDataset);

  output = UNITY_END();
  ZF_LOGI("Ending CSV Reader Test, result: %d", output);
  return output;
}
