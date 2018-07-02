#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "csv.h"
#include "unity.h"

#ifndef ZF_LOG_LEVEL
# define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#endif /* ZF_LOG_LEVEL */
#include "zf_log.h"

FILE *_log_file;

static void file_output_callback(const zf_log_message *msg, void *arg)
{
  (void)arg;
  *msg->p = '\n';
  fwrite(msg->buf, msg->p - msg->buf + 1, 1, _log_file);
  fflush(_log_file);
}

static void file_output_close(void)
{
  fclose(_log_file);
}

static void file_output_open(const char *const log_path)
{
  _log_file = fopen(log_path, "w");

  if (!_log_file)
  {
    ZF_LOGW("Failed to open log file %s", log_path);
    return;
  }
  atexit(file_output_close);
  zf_log_set_output_v(ZF_LOG_PUT_STD, 0, file_output_callback);
}

void test_CSVWriterInitDestroy(void) {
  ZF_LOGI("Beginning test_CSVWriterInitDestroy");
  csvdialect dialect = csvdialect_init();

  ZF_LOGI("Testing passing NULL for filepath");
  csvwriter  writer  = csvwriter_init(dialect, NULL);

  TEST_ASSERT_NULL(writer);

  ZF_LOGI("Testing passing real filepath");
  writer = csvwriter_init(dialect, "data/test_writer_no_output.csv");

  TEST_ASSERT_NOT_NULL(writer);

  ZF_LOGI("Testing close open CSV Writer");
  csvwriter_close(&writer);
  TEST_ASSERT_NULL(writer);

  csvdialect_close(&dialect);
  ZF_LOGI("Ending test_CSVWriterInitDestroy");
}

void test_CSVWriterTwoLines(void) {
  ZF_LOGI("Beginning test_CSVWriterTwoLines");
  csvreturn  rc;
  csvdialect dialect = csvdialect_init();
  csvwriter  writer  = csvwriter_init(dialect, "data/test_writer_two_lines.csv");

  TEST_ASSERT_NOT_NULL(writer);

  const char **record = NULL;
  record = malloc(sizeof *record * 3);
  TEST_ASSERT_NOT_NULL(record);
  record[0] = "field_0";
  record[1] = "field_1";
  record[2] = "field_2";

  rc = csvwriter_next_record(writer, CSV_CHAR, (csvrecord_type)record, 3);
  TEST_ASSERT_TRUE(csv_success(rc));

  record[0] = "a";
  record[1] = "1.2";
  record[2] = "true";

  rc = csvwriter_next_record(writer, CSV_CHAR, (csvrecord_type)record, 3);
  TEST_ASSERT_TRUE(csv_success(rc));

  csvwriter_close(&writer);
  TEST_ASSERT_NULL(writer);

  csvdialect_close(&dialect);
  free(record);

  ZF_LOGI("Ending test_CSVWriterTwoLines");
}

int main(int argc, char **argv) {
  int output = 0;

  file_output_open("test_writer.log");

  ZF_LOGI("Beginning CSV Writer Test");

  UNITY_BEGIN();

  RUN_TEST(test_CSVWriterInitDestroy);
  RUN_TEST(test_CSVWriterTwoLines);

  output = UNITY_END();
  ZF_LOGI("Ending CSV Writer Test, result: %d", output);
  return output;
}
