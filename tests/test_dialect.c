#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csv.h"
#include "dialect_private.h"
#include "unity.h"

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
	csvdialect source, destination;

	source = csvdialect_init();
	TEST_ASSERT_NOT_NULL( source);

	destination = csvdialect_copy(source);
	TEST_ASSERT_NOT_NULL( destination);

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
		csvdialect_get_delimiter(destination) );
	TEST_ASSERT_EQUAL_INT(csvdialect_get_doublequote(source),
		csvdialect_get_doublequote(destination) );
	TEST_ASSERT_EQUAL(csvdialect_get_lineterminator(source),
		csvdialect_get_lineterminator(destination) );
	TEST_ASSERT_EQUAL_INT(csvdialect_get_escapechar(source),
		csvdialect_get_escapechar(destination) );
	TEST_ASSERT_EQUAL_INT(csvdialect_get_quotechar(source),
		csvdialect_get_quotechar(destination) );
	TEST_ASSERT_EQUAL(csvdialect_get_quotestyle(source),
		csvdialect_get_quotestyle(destination) );
	TEST_ASSERT_EQUAL_INT(csvdialect_get_skipinitialspace(source),
		csvdialect_get_skipinitialspace(destination) );

	csvdialect_close( &source);
	csvdialect_close( &destination);
	TEST_ASSERT_NULL( source);
	TEST_ASSERT_NULL( destination);

  /*
   * validate if the source CSV Dialect is NULL, CSV Dialect copy also
   * returns NULL
   */
	source = NULL;
	destination = NULL;
	destination = csvdialect_copy(source);
	TEST_ASSERT_NULL( source);
	TEST_ASSERT_NULL( destination);
}

/*
 * Validate that `csv_validate` returns the correct analysis of a dialect.
 */
void test_CSVDialectValidate(void) {
	csvdialect dialect;

	dialect = csvdialect_init();
	TEST_ASSERT_NOT_NULL(dialect);

  /* default CSV dialect must be valid */
	TEST_ASSERT_TRUE(csv_success(csvdialect_validate(dialect) ) );

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
	TEST_ASSERT_EQUAL_INT(',', csvdialect_get_delimiter(dialect) );

	TEST_ASSERT_TRUE(csv_success(csvdialect_set_delimiter(dialect, '\t') ) );

  /* validate that after setting new value, the new value is returned */
	TEST_ASSERT_EQUAL_INT('\t', csvdialect_get_delimiter(dialect) );

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
	TEST_ASSERT_TRUE( csvdialect_get_doublequote(dialect) );

	TEST_ASSERT_TRUE( csv_success(csvdialect_set_doublequote(dialect, false) ) );

  /* validate that after setting new value, the new value is returned */
	TEST_ASSERT_FALSE(csvdialect_get_doublequote(dialect) );

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
	TEST_ASSERT_EQUAL_INT(CSV_UNDEFINED_CHAR,
		csvdialect_get_escapechar(dialect) );

	TEST_ASSERT_TRUE(csv_success(csvdialect_set_escapechar(dialect, '\0') ) );

  /* validate that after setting new value, the new value is returned */
	TEST_ASSERT_EQUAL_INT('\0', csvdialect_get_escapechar(dialect) );

	csvdialect_close(&dialect);
	TEST_ASSERT_NULL(dialect);
}

/*
 * Validate that setting and getting the line terminator attribute works
 * and that the defaults are in alignment with the documentation
 */
void test_CSVDialectSetGetLineterminator(void) {
	csvdialect dialect;

	dialect = csvdialect_init();
	TEST_ASSERT_NOT_NULL(dialect);

	TEST_ASSERT_EQUAL(LINETERMINATOR_SYSTEM_DEFAULT,
		csvdialect_get_lineterminator(dialect) );
	TEST_ASSERT_TRUE(csv_success(
			csvdialect_set_lineterminator(dialect, LINETERMINATOR_CRNL) ) );
	TEST_ASSERT_EQUAL(LINETERMINATOR_CRNL,
		csvdialect_get_lineterminator(dialect) );

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
	TEST_ASSERT_EQUAL_INT('"', csvdialect_get_quotechar(dialect) );

	TEST_ASSERT_TRUE(csv_success(csvdialect_set_quotechar(dialect, '\'') ) );

  /* validate that after setting new value, the new value is returned */
	TEST_ASSERT_EQUAL_INT('\'', csvdialect_get_quotechar(dialect) );

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
	TEST_ASSERT_EQUAL(QUOTE_STYLE_MINIMAL, csvdialect_get_quotestyle(dialect) );

	TEST_ASSERT_TRUE(csv_success(csvdialect_set_quotestyle(dialect,
				QUOTE_STYLE_NONE) ) );
	TEST_ASSERT_EQUAL(QUOTE_STYLE_NONE, csvdialect_get_quotestyle(dialect) );

	TEST_ASSERT_TRUE(csv_success(csvdialect_set_quotestyle(dialect,
				QUOTE_STYLE_ALL) ) );
	TEST_ASSERT_EQUAL(QUOTE_STYLE_ALL, csvdialect_get_quotestyle(dialect) );

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
	TEST_ASSERT_EQUAL_INT(false, csvdialect_get_skipinitialspace(dialect) );

	TEST_ASSERT_TRUE(csv_success(csvdialect_set_skipinitialspace(dialect,
				true) ) );

  /* validate that after setting new value, the new value is returned */
	TEST_ASSERT_EQUAL_INT(true, csvdialect_get_skipinitialspace(dialect) );

	csvdialect_close(&dialect);
	TEST_ASSERT_NULL(dialect);
}

/*
 * Run the tests
 */
int main(int	argc,
         char **argv) {
	UNITY_BEGIN();

	RUN_TEST(test_CSVDialectInitDestroy);

  /*
   * RUN_TEST(test_CSVDialectCopy);
   * RUN_TEST(test_CSVDialectValidate);
   */

	RUN_TEST( test_CSVDialectSetGetDelimiter);
	RUN_TEST( test_CSVDialectSetGetDoublequote);
	RUN_TEST( test_CSVDialectSetGetEscapechar);
	RUN_TEST( test_CSVDialectSetGetLineterminator);
	RUN_TEST( test_CSVDialectSetGetQuotechar);
	RUN_TEST( test_CSVDialectSetGetQuotestyle);
	RUN_TEST( test_CSVDialectSetGetSkipInitialSpace);

	return UNITY_END();
}
