#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ZF_LOG_LEVEL
#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#endif /* ZF_LOG_LEVEL */
#include "zf_log.h"

#include "csv.h"
#include "dialect_private.h"
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

/*
 * Validate that a default initialized CSV Dialect can be safely initialized
 * and destroyed. Also validate that the final state is NULL as documented.
 */
void test_CSVDialectInitDestroy(void) {
  csvdialect dialect = NULL;

  dialect = csvdialect_init();
  TEST_ASSERT_NOT_NULL(dialect);

  csvdialect_close(&dialect);
  TEST_ASSERT_NULL(dialect);
}

/*
 * Validate copies of CSV Dialect are created accurately
 */
void test_CSVDialectCopy(void) {
  csvdialect  source, destination;
  size_t      lh_size, rh_size;
  const char *lh, *rh;

  source = csvdialect_init();
  TEST_ASSERT_NOT_NULL(source);

  destination = csvdialect_copy(source);
  TEST_ASSERT_NOT_NULL(destination);

  /*
   * dialect is a pointer, therefore we should be able to use the equal
   *  operator
   * per the C11 standard.
   * https://stackoverflow.com/questions/9086372/how-to-compare-pointers
   */
  TEST_ASSERT_NOT_EQUAL(source, destination);

  /*
   * validate a deep copy was made by comparing each element.
   */
  TEST_ASSERT_EQUAL_INT(csvdialect_get_delimiter(source),
                        csvdialect_get_delimiter(destination));
  TEST_ASSERT_EQUAL_INT(csvdialect_get_doublequote(source),
                        csvdialect_get_doublequote(destination));

  // lineterminator string should not be freed, as it assumed to be a string
  // constant
  lh_size = 0;
  rh_size = 0;
  lh      = csvdialect_get_lineterminator(source, &lh_size);
  rh      = csvdialect_get_lineterminator(destination, &rh_size);
  TEST_ASSERT_EQUAL_STRING(lh, rh);
  TEST_ASSERT_EQUAL_UINT(lh_size, rh_size);

  TEST_ASSERT_EQUAL_INT(csvdialect_get_escapechar(source),
                        csvdialect_get_escapechar(destination));
  TEST_ASSERT_EQUAL_INT(csvdialect_get_quotechar(source),
                        csvdialect_get_quotechar(destination));
  TEST_ASSERT_EQUAL(csvdialect_get_quotestyle(source),
                    csvdialect_get_quotestyle(destination));
  TEST_ASSERT_EQUAL_INT(csvdialect_get_skipinitialspace(source),
                        csvdialect_get_skipinitialspace(destination));

  csvdialect_close(&source);
  csvdialect_close(&destination);
  TEST_ASSERT_NULL(source);
  TEST_ASSERT_NULL(destination);

  /*
   * validate if the source CSV Dialect is NULL, CSV Dialect copy also
   * returns NULL
   */
  source      = NULL;
  destination = NULL;
  destination = csvdialect_copy(source);
  TEST_ASSERT_NULL(source);
  TEST_ASSERT_NULL(destination);
}

/*
 * Validate that `csv_validate` returns the correct analysis of a dialect.
 */
void test_CSVDialectValidate(void) {
  csvdialect dialect;

  dialect = csvdialect_init();
  TEST_ASSERT_NOT_NULL(dialect);

  /* default CSV dialect must be valid */
  TEST_ASSERT_TRUE(csv_success(csvdialect_validate(dialect)));

  /*
   * TODO: define invalid states and ensure they're evaluated
   * as invalid.
   */

  csvdialect_close(&dialect);
  TEST_ASSERT_NULL(dialect);
}

/*
 * Validate that setting and getting the delimiter character attribute works
 * and that the defaults are in alignment with the documentation
 */
void test_CSVDialectSetGetDelimiter(void) {
  csvdialect dialect;

  dialect = csvdialect_init();
  TEST_ASSERT_NOT_NULL(dialect);

  /* default is documented as ',' (comma) */
  TEST_ASSERT_EQUAL_INT(',', csvdialect_get_delimiter(dialect));

  TEST_ASSERT_TRUE(csv_success(csvdialect_set_delimiter(dialect, '\t')));

  /* validate that after setting new value, the new value is returned */
  TEST_ASSERT_EQUAL_INT('\t', csvdialect_get_delimiter(dialect));

  csvdialect_close(&dialect);
  TEST_ASSERT_NULL(dialect);
}

/*
 * Validate that setting and getting the double quote escape attribute works
 * and that the defaults are in alignment with the documentation
 */
void test_CSVDialectSetGetDoublequote(void) {
  csvdialect dialect;

  dialect = csvdialect_init();
  TEST_ASSERT_NOT_NULL(dialect);

  /* default is documented as `true` */
  TEST_ASSERT_TRUE(csvdialect_get_doublequote(dialect));

  TEST_ASSERT_TRUE(csv_success(csvdialect_set_doublequote(dialect, false)));

  /* validate that after setting new value, the new value is returned */
  TEST_ASSERT_FALSE(csvdialect_get_doublequote(dialect));

  csvdialect_close(&dialect);
  TEST_ASSERT_NULL(dialect);
}

/*
 * Validate that setting and getting the escape character attribute works
 * and that the defaults are in alignment with the documentation
 */
void test_CSVDialectSetGetEscapechar(void) {
  csvdialect dialect;

  dialect = csvdialect_init();
  TEST_ASSERT_NOT_NULL(dialect);

  /* default is documented as `true` */
  TEST_ASSERT_EQUAL_INT(CSV_UNDEFINED_CHAR, csvdialect_get_escapechar(dialect));

  TEST_ASSERT_TRUE(csv_success(csvdialect_set_escapechar(dialect, '\0')));

  /* validate that after setting new value, the new value is returned */
  TEST_ASSERT_EQUAL_INT('\0', csvdialect_get_escapechar(dialect));

  csvdialect_close(&dialect);
  TEST_ASSERT_NULL(dialect);
}

/*
 * Validate that setting and getting the line terminator attribute works
 * and that the defaults are in alignment with the documentation
 */
void test_CSVDialectSetGetLineterminator(void) {
  csvdialect dialect;
  size_t     lt_size;

  dialect = csvdialect_init();
  TEST_ASSERT_NOT_NULL(dialect);

  TEST_ASSERT_NULL(csvdialect_get_lineterminator(dialect, &lt_size));
  TEST_ASSERT_EQUAL_UINT(0U, lt_size);

  TEST_ASSERT_TRUE(
      csv_success(csvdialect_set_lineterminator(dialect, "\r\n", 0)));
  TEST_ASSERT_EQUAL_STRING("\r\n",
                           csvdialect_get_lineterminator(dialect, &lt_size));
  TEST_ASSERT_EQUAL_UINT(2U, lt_size);

  csvdialect_close(&dialect);
  TEST_ASSERT_NULL(dialect);
}

/*
 * Validate that setting and getting the quoting character attribute works
 * and that the defaults are in alignment with the documentation
 */
void test_CSVDialectSetGetQuotechar(void) {
  csvdialect dialect;

  dialect = csvdialect_init();
  TEST_ASSERT_NOT_NULL(dialect);

  /* default is documented as '"' (doublequote character) */
  TEST_ASSERT_EQUAL_INT('"', csvdialect_get_quotechar(dialect));

  TEST_ASSERT_TRUE(csv_success(csvdialect_set_quotechar(dialect, '\'')));

  /* validate that after setting new value, the new value is returned */
  TEST_ASSERT_EQUAL_INT('\'', csvdialect_get_quotechar(dialect));

  csvdialect_close(&dialect);
  TEST_ASSERT_NULL(dialect);
}

/*
 * Validate that setting and getting the quote style attribute works
 * and that the defaults are in alignment with the documentation
 */
void test_CSVDialectSetGetQuotestyle(void) {
  csvdialect dialect;

  dialect = csvdialect_init();
  TEST_ASSERT_NOT_NULL(dialect);

  /* default is documented as QUOTE_STYLE_MINIMAL */
  TEST_ASSERT_EQUAL(QUOTE_STYLE_MINIMAL, csvdialect_get_quotestyle(dialect));

  TEST_ASSERT_TRUE(
      csv_success(csvdialect_set_quotestyle(dialect, QUOTE_STYLE_NONE)));
  TEST_ASSERT_EQUAL(QUOTE_STYLE_NONE, csvdialect_get_quotestyle(dialect));

  TEST_ASSERT_TRUE(
      csv_success(csvdialect_set_quotestyle(dialect, QUOTE_STYLE_ALL)));
  TEST_ASSERT_EQUAL(QUOTE_STYLE_ALL, csvdialect_get_quotestyle(dialect));

  csvdialect_close(&dialect);
  TEST_ASSERT_NULL(dialect);
}

/*
 * Validate that setting and getting the skip initial space attribute works
 * and that the defaults are in alignment with the documentation
 */
void test_CSVDialectSetGetSkipInitialSpace(void) {
  csvdialect dialect;

  dialect = csvdialect_init();
  TEST_ASSERT_NOT_NULL(dialect);

  /* default is documented as '"' (doublequote character) */
  TEST_ASSERT_EQUAL_INT(false, csvdialect_get_skipinitialspace(dialect));

  TEST_ASSERT_TRUE(csv_success(csvdialect_set_skipinitialspace(dialect, true)));

  /* validate that after setting new value, the new value is returned */
  TEST_ASSERT_EQUAL_INT(true, csvdialect_get_skipinitialspace(dialect));

  csvdialect_close(&dialect);
  TEST_ASSERT_NULL(dialect);
}

/*
 * Run the tests
 *
 * int main(int argc, char **argv) {
 */
int main(void) {
  int output = 0;

  file_output_open("test_dialect.log");

  ZF_LOGI("Beginning CSV Dialect Test");

  UNITY_BEGIN();

  RUN_TEST(test_CSVDialectInitDestroy);

  /*
   * RUN_TEST(test_CSVDialectCopy);
   * RUN_TEST(test_CSVDialectValidate);
   */

  RUN_TEST(test_CSVDialectSetGetDelimiter);
  RUN_TEST(test_CSVDialectSetGetDoublequote);
  RUN_TEST(test_CSVDialectSetGetEscapechar);
  RUN_TEST(test_CSVDialectSetGetLineterminator);
  RUN_TEST(test_CSVDialectSetGetQuotechar);
  RUN_TEST(test_CSVDialectSetGetQuotestyle);
  RUN_TEST(test_CSVDialectSetGetSkipInitialSpace);

  output = UNITY_END();
  ZF_LOGI("Ending CSV Dialect Test, result: %d", output);
  return output;
}
