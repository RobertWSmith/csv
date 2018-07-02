#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ZF_LOG_LEVEL
# define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#endif /* ZF_LOG_LEVEL */
#include "zf_log.h"

// #include "csv.h"
#include "csv/version.h"
#include "csv/definitions.h"
#include "csv/dialect.h"
#include "dialect_private.h"

/*
 * @brief implementation of the @c csvdialect pointer.
 */
struct csv_dialect {
  QUOTE_STYLE              quotestyle;
  CSV_LINETERMINATOR_TYPE  lineterminator;
  csv_comparison_char_type delimiter, escapechar, quotechar;
  bool                     doublequote, skipinitialspace;
};

csvdialect csvdialect_init(void) {
  csvdialect dialect = NULL;

  if ((dialect = malloc(sizeof *dialect)) == NULL) {
    ZF_LOGD("`csvdialect` default initialization failure.");
    return NULL;
  }

  if (csv_failure(csvdialect_set_delimiter(dialect, ','))) {
    ZF_LOGD("`csvdialect` default initialization failure.");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_doublequote(dialect, true))) {
    ZF_LOGD("`csvdialect` default initialization failure.");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_escapechar(dialect, CSV_UNDEFINED_CHAR))) {
    ZF_LOGD("`csvdialect` default initialization failure.");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_lineterminator(dialect,
                                                LINETERMINATOR_SYSTEM_DEFAULT))) {
    ZF_LOGD("`csvdialect` default initialization failure.");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_quotechar(dialect, '\"'))) {
    ZF_LOGD("`csvdialect` default initialization failure.");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_quotestyle(dialect, QUOTE_STYLE_MINIMAL))) {
    ZF_LOGD("`csvdialect` default initialization failure.");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_skipinitialspace(dialect, false))) {
    ZF_LOGD("`csvdialect` default initialization failure.");
    csvdialect_close(&dialect);
    return NULL;
  }

  ZF_LOGI("New dialect created at `%p`.", dialect);
  return dialect;
}

csvdialect csvdialect_copy(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGI("NULL dialect passed to `csvdialect_copy`.");
    return NULL;
  }

  csvdialect output = csvdialect_init();

  if (csv_failure(csvdialect_set_delimiter(dialect,
                                           csvdialect_get_delimiter(output)))) {
    ZF_LOGD("`csvdialect_copy` failure.");
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_doublequote(dialect,
                                             csvdialect_get_doublequote(output))))
  {
    ZF_LOGD("`csvdialect_copy` failure.");
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_escapechar(dialect,
                                            csvdialect_get_escapechar(output)))) {
    ZF_LOGD("`csvdialect_copy` failure.");
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_lineterminator(dialect,
                                                csvdialect_get_lineterminator(
                                                  output)))) {
    ZF_LOGD("`csvdialect_copy` failure.");
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_quotechar(dialect,
                                           csvdialect_get_quotechar(output)))) {
    ZF_LOGD("`csvdialect_copy` failure.");
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_quotestyle(dialect,
                                            csvdialect_get_quotestyle(output)))) {
    ZF_LOGD("`csvdialect_copy` failure.");
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_skipinitialspace(dialect,
                                                  csvdialect_get_skipinitialspace(
                                                    output)))) {
    ZF_LOGD("`csvdialect_copy` failure.");
    csvdialect_close(&output);
    return NULL;
  }

  return output;
}

void csvdialect_close(csvdialect *dialect) {
  ZF_LOGD("`csvdialect_close` called at: `%p`.", dialect);
  csvdialect d = *dialect;

  if (d == NULL) {
    ZF_LOGD("`csvdialect_close` called on NULL pointer.");
    return;
  }

  ZF_LOGI("Attempting to free `csvdialect` at: `%p`.", d);
  free(d);
  ZF_LOGD("Setting `csvdialect*` to NULL at: `%p`.", dialect);
  *dialect = NULL;
}

csvreturn csvdialect_validate(csvdialect dialect) {
  ZF_LOGD("`csvdialect_validate` called at: `%p`.", dialect);
  csvreturn rc = csvreturn_init(false);

  if (dialect == NULL) {
    ZF_LOGI("`csvdialect_validate` identified a NULL dialect.");
    rc.dialect_null = 1;
    return rc;
  }

  if (csvdialect_get_delimiter(dialect) == CSV_UNDEFINED_CHAR) {
    ZF_LOGI("`csvdialect_validate` identified a undefined delimiter character.");
    rc.delimiter_error = 1;
    return rc;
  }

  /*
   * size_t size_b;
   * const csv_comparison_char_type *string_b;
   */

  /* string_b = csvdialect_get_lineterminator(dialect, &size_b); */

  /*
   * if ( (size_b == 0) || (string_b == NULL) ) {
   *  rc.lineterminator_error = 1;
   *  return rc;
   *  }
   */

  /*
   * free( (void*)string_b );
   * string_b = NULL;
   */

  if ((!csvdialect_get_doublequote(dialect)) &&
      (csvdialect_get_escapechar(dialect) == CSV_UNDEFINED_CHAR))  {
    ZF_LOGI("`csvdialect_validate` identified a quoting rule in an invalid state.");
    rc.quoteescape_error = 1;
    return rc;
  }

  ZF_LOGD(
    "`csvdialect_validate` did not identify any issues with the CSV Dialect.");

  /* all checks passed, return success */
  rc.succeeded = 1;
  return rc;
}

csvreturn csvdialect_set_delimiter(csvdialect               dialect,
                                   csv_comparison_char_type delimiter) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_set_delimiter` was passed NULL in the dialect argument.");
    return csvreturn_init(false);
  }
  else if (delimiter == CSV_UNDEFINED_CHAR) {
    ZF_LOGI("`csvdialect_set_delimiter` was an invalid delimiter character.");
    return csvreturn_init(false);
  }

  ZF_LOGD("`csvdialect_set_delimiter` setting `%c`.", (char)delimiter);
  dialect->delimiter = delimiter;
  return csvreturn_init(true);
}

csv_comparison_char_type csvdialect_get_delimiter(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_get_delimiter` was passed NULL in the dialect argument.");
    return CSV_UNDEFINED_CHAR;
  }

  ZF_LOGD("`csvdialect_get_delimiter` getting `%c`.", (char)dialect->delimiter);
  return dialect->delimiter;
}

csvreturn csvdialect_set_doublequote(csvdialect dialect,
                                     bool       doublequote) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_set_doublequote` was passed NULL in the dialect argument.");
    return csvreturn_init(false);
  }

  ZF_LOGD("`csvdialect_set_doublequote` setting `%s`.",
          doublequote ? "true" : "false");
  dialect->doublequote = doublequote;
  return csvreturn_init(true);
}

bool csvdialect_get_doublequote(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_get_doublequote` was passed NULL in the dialect argument.");
    return true;
  }

  ZF_LOGD("`csvdialect_get_doublequote` setting `%s`",
          dialect->doublequote ? "true" : "false");
  return dialect->doublequote;
}

csvreturn csvdialect_set_escapechar(csvdialect               dialect,
                                    csv_comparison_char_type escapechar) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_set_escapechar` was passed NULL in the dialect argument.");
    return csvreturn_init(false);
  }

  ZF_LOGD("`csvdialect_set_escapechar` setting `%c`.", (char)escapechar);
  dialect->escapechar = escapechar;
  return csvreturn_init(true);
}

csv_comparison_char_type csvdialect_get_escapechar(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_get_escapechar` was passed NULL in the dialect argument.");
    return CSV_UNDEFINED_CHAR;
  }

  ZF_LOGD("`csvdialect_get_escapechar` setting `%c`.", (char)dialect->escapechar);
  return dialect->escapechar;
}

csvreturn csvdialect_set_lineterminator(csvdialect              dialect,
                                        CSV_LINETERMINATOR_TYPE lineterminator) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_set_lineterminator` was passed NULL in the dialect argument.");
    return csvreturn_init(false);
  }

  ZF_LOGD("`csvdialect_set_lineterminator` setting `%d`.", lineterminator);
  dialect->lineterminator = lineterminator;
  return csvreturn_init(true);
}

CSV_LINETERMINATOR_TYPE csvdialect_get_lineterminator(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`dialect` was passed NULL");
    return LINETERMINATOR_SYSTEM_DEFAULT;
  }

  ZF_LOGD("returning `%d`.",
          dialect->lineterminator);
  return dialect->lineterminator;
}

csvreturn csvdialect_set_quotechar(csvdialect               dialect,
                                   csv_comparison_char_type quotechar) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect` was passed NULL.");
    return csvreturn_init(false);
  }

  ZF_LOGD("returning `%c`.", (char)quotechar);
  dialect->quotechar = quotechar;
  return csvreturn_init(true);
}

csv_comparison_char_type csvdialect_get_quotechar(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_get_quotechar` was passed NULL in the dialect argument.");
    return CSV_UNDEFINED_CHAR;
  }

  ZF_LOGD("`csvdialect_get_quotechar` setting `%c`.", (char)dialect->quotechar);
  return dialect->quotechar;
}

csvreturn csvdialect_set_quotestyle(csvdialect  dialect,
                                    QUOTE_STYLE quotestyle) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_set_quotestyle` was passed NULL in the dialect argument.");
    return csvreturn_init(false);
  }

  ZF_LOGD("`csvdialect_set_quotestyle` setting `%d`.", quotestyle);
  dialect->quotestyle = quotestyle;
  return csvreturn_init(true);
}

QUOTE_STYLE csvdialect_get_quotestyle(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_get_quotestyle` was passed NULL in the dialect argument.");
    return QUOTE_STYLE_MINIMAL;
  }

  ZF_LOGD("`csvdialect_get_quotestyle` setting `%d`.", dialect->quotestyle);
  return dialect->quotestyle;
}

csvreturn csvdialect_set_skipinitialspace(csvdialect dialect,
                                          bool       skipinitialspace) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_set_skipinitialspace` was passed NULL in the dialect argument.");
    return csvreturn_init(false);
  }

  ZF_LOGD("`csvdialect_set_skipinitialspace` setting `%s`",
          skipinitialspace ? "true" : "false");
  dialect->skipinitialspace = skipinitialspace;
  return csvreturn_init(true);
}

bool csvdialect_get_skipinitialspace(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGI(
      "`csvdialect_get_skipinitialspace` was passed NULL in the dialect argument.");
    return false;
  }

  ZF_LOGD("`csvdialect_get_skipinitialspace` setting `%s`",
          dialect->skipinitialspace ? "true" : "false");
  return dialect->skipinitialspace;
}
