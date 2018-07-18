/**
 * @cond INTERNAL
 *
 * @file csv_dialect.c
 * @author Robert W. Smith
 * @date 2018-06-27
 * @brief Implementation of CSV Dialect object
 *
 * Private documentation, API subject to change.
 *
 * @see csv/definitions.h
 * @see csv/write.h
 * @see csv/stream.h
 * @see csv/read.h
 * @see csv/version.h
 */

#ifdef __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif /* __STDC_LIB_EXT1__ */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ZF_LOG_LEVEL
#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#endif /* ZF_LOG_LEVEL */
#include "zf_log.h"

#include "csv.h"
// #include "csv/definitions.h"
// #include "csv/version.h"

// #include "csv/dialect.h"
#include "dialect_private.h"

/*
 * private function declarations - begin
 */

/**
 * @brief CSV Dialect private allocator / default setup.
 *
 * @return  null / zero initialized CSV Dialect
 */
csvdialect csvdialect_alloc(void);

/*
 * private function declarations - end
 */

/*
 * @brief implementation of the @c csvdialect pointer.
 */
struct csv_dialect {
  const char *             lineterminator;
  size_t                   lineterminator_length;
  csv_comparison_char_type delimiter;
  csv_comparison_char_type escapechar;
  csv_comparison_char_type quotechar;
  QUOTE_STYLE              quotestyle;
  bool                     doublequote;
  bool                     skipinitialspace;
};

csvdialect csvdialect_alloc(void) {
  csvdialect dialect;

  if ((dialect = malloc(sizeof *dialect)) == NULL) {
    return NULL;
  }

  dialect->lineterminator        = NULL;
  dialect->lineterminator_length = 0;
  dialect->delimiter             = 0;
  dialect->escapechar            = 0;
  dialect->quotechar             = 0;
  dialect->quotestyle            = QUOTE_STYLE_MINIMAL;
  dialect->doublequote           = false;
  dialect->skipinitialspace      = false;

  return dialect;
}

csvdialect csvdialect_init(void) {
  csvdialect dialect;

  if ((dialect = csvdialect_alloc()) == NULL) {
    ZF_LOGE("default initialization failure - allocation");
    return NULL;
  }

  if (csv_failure(csvdialect_set_delimiter(dialect, ','))) {
    ZF_LOGE("default initialization failure - delimiter");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_doublequote(dialect, true))) {
    ZF_LOGE("default initialization failure - doublequote");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_escapechar(dialect, CSV_UNDEFINED_CHAR))) {
    ZF_LOGE("default initialization failure - escapechar");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_lineterminator(dialect, NULL, 0))) {
    ZF_LOGE("default initialization failure - lineterminator");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_quotechar(dialect, '\"'))) {
    ZF_LOGE("default initialization failure - quotechar");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_quotestyle(dialect, QUOTE_STYLE_MINIMAL))) {
    ZF_LOGE("default initialization failure - quotestyle");
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_skipinitialspace(dialect, false))) {
    ZF_LOGE("default initialization failure - skipinitialspace");
    csvdialect_close(&dialect);
    return NULL;
  }

  ZF_LOGD("Dialect successfully initialized: %p", (void *)dialect);
  return dialect;
}

csvdialect csvdialect_copy(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return NULL;
  }

  csvdialect               output;
  csvreturn                rc;
  size_t                   lt_length;
  const char *             lt;
  csv_comparison_char_type delimiter;
  csv_comparison_char_type escapechar;
  csv_comparison_char_type quotechar;
  QUOTE_STYLE              quotestyle;
  bool                     doublequote;
  bool                     skipinitialspace;

  if ((output = csvdialect_init()) == NULL) {
    ZF_LOGE("dialect copy failure - output allocation");
    return NULL;
  }

  delimiter = csvdialect_get_delimiter(dialect);
  rc        = csvdialect_set_delimiter(output, delimiter);
  if (csv_failure(rc)) {
    ZF_LOGE("dialect copy failure - delimiter");
    csvdialect_close(&output);
    return NULL;
  }

  doublequote = csvdialect_get_doublequote(dialect);
  rc          = csvdialect_set_doublequote(output, doublequote);
  if (csv_failure(rc)) {
    ZF_LOGE("dialect copy failure - doublequote");
    csvdialect_close(&output);
    return NULL;
  }

  escapechar = csvdialect_get_escapechar(dialect);
  rc         = csvdialect_set_escapechar(output, escapechar);
  if (csv_failure(rc)) {
    ZF_LOGE("dialect copy failure - escapechar");
    csvdialect_close(&output);
    return NULL;
  }

  lt = csvdialect_get_lineterminator(dialect, &lt_length);
  rc = csvdialect_set_lineterminator(output, lt, lt_length);
  if (csv_failure(rc)) {
    ZF_LOGE("dialect copy failure - lineterminator");
    csvdialect_close(&output);
    return NULL;
  }

  quotechar = csvdialect_get_quotechar(dialect);
  rc        = csvdialect_set_quotechar(output, quotechar);
  if (csv_failure(rc)) {
    ZF_LOGE("dialect copy failure - quotechar");
    csvdialect_close(&output);
    return NULL;
  }

  quotestyle = csvdialect_get_quotestyle(dialect);
  rc         = csvdialect_set_quotestyle(output, quotestyle);
  if (csv_failure(rc)) {
    ZF_LOGE("dialect copy failure - quotestyle");
    csvdialect_close(&output);
    return NULL;
  }

  skipinitialspace = csvdialect_get_skipinitialspace(dialect);
  rc               = csvdialect_set_skipinitialspace(output, skipinitialspace);
  if (csv_failure(rc)) {
    ZF_LOGE("dialect copy failure - skipinitialspace");
    csvdialect_close(&output);
    return NULL;
  }

  ZF_LOGD("dialect successfully copied from source `%p` to target `%p`",
          (void *)dialect,
          (void *)output);
  return output;
}

void csvdialect_close(csvdialect *dialect) {
  if ((*dialect) == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return;
  }

  ZF_LOGD("freeing dialect resources: %p", (void *)(*dialect));
  free((*dialect));
  *dialect = NULL;
}

/*
 * TODO: update and document...
 */
csvreturn csvdialect_validate(csvdialect dialect) {
  ZF_LOGD("`csvdialect_validate` called at: `%p`", (void *)dialect);
  csvreturn rc = csvreturn_init(false);

  if (dialect == NULL) {
    ZF_LOGE("`csvdialect_validate` identified a NULL dialect");
    rc.dialect_null = 1;
    return rc;
  }

  if (csvdialect_get_delimiter(dialect) == CSV_UNDEFINED_CHAR) {
    ZF_LOGE("`csvdialect_validate` identified a undefined delimiter character");
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
      (csvdialect_get_escapechar(dialect) == CSV_UNDEFINED_CHAR)) {
    ZF_LOGE(
        "`csvdialect_validate` identified a quoting rule in an invalid state");
    rc.quoteescape_error = 1;
    return rc;
  }

  ZF_LOGI(
      "`csvdialect_validate` did not identify any issues with the CSV "
      "Dialect");

  /* all checks passed, return success */
  rc.succeeded = 1;
  return rc;
}

csvreturn csvdialect_set_delimiter(csvdialect               dialect,
                                   csv_comparison_char_type delimiter) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return csvreturn_init(false);
  } else if (delimiter == CSV_UNDEFINED_CHAR) {
    ZF_LOGE("`delimiter` set to undefined character");
    return csvreturn_init(false);
  }

  ZF_LOGD("delimiter `%c`", (char)delimiter);
  dialect->delimiter = delimiter;
  return csvreturn_init(true);
}

csv_comparison_char_type csvdialect_get_delimiter(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return CSV_UNDEFINED_CHAR;
  }

  ZF_LOGD("delimiter `%c`", (char)dialect->delimiter);
  return dialect->delimiter;
}

csvreturn csvdialect_set_doublequote(csvdialect dialect, bool doublequote) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return csvreturn_init(false);
  }

  ZF_LOGD("doublequote `%s`", doublequote ? "true" : "false");
  dialect->doublequote = doublequote;
  return csvreturn_init(true);
}

bool csvdialect_get_doublequote(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return true;
  }

  ZF_LOGD("doublequote `%s`", dialect->doublequote ? "true" : "false");
  return dialect->doublequote;
}

csvreturn csvdialect_set_escapechar(csvdialect               dialect,
                                    csv_comparison_char_type escapechar) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return csvreturn_init(false);
  }

  ZF_LOGD("escapechar `%c`", (char)escapechar);
  dialect->escapechar = escapechar;
  return csvreturn_init(true);
}

csv_comparison_char_type csvdialect_get_escapechar(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return CSV_UNDEFINED_CHAR;
  }

  ZF_LOGD("escapechar `%c`", (char)dialect->escapechar);
  return dialect->escapechar;
}

csvreturn csvdialect_set_lineterminator(csvdialect  dialect,
                                        const char *lineterminator,
                                        size_t      length) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return csvreturn_init(false);
  }

  ZF_LOGD("lineterminator `%s` length `%lu`",
          lineterminator,
          (unsigned long)length);

  if (dialect->lineterminator != NULL) {
    free((void *)dialect->lineterminator);
  }

  if (lineterminator == NULL) {
    dialect->lineterminator_length = 0;
    dialect->lineterminator        = NULL;
    return csvreturn_init(true);
  } else if (length == 0) {
    dialect->lineterminator_length = strlen(lineterminator);
  } else {
    dialect->lineterminator_length = length;
  }

  dialect->lineterminator = malloc(sizeof *dialect->lineterminator *
                                   (dialect->lineterminator_length + 1));

  if (dialect->lineterminator == NULL) {
    ZF_LOGE("Could not allocate space for the lineterminator");
    return csvreturn_init(false);
  }

#ifdef __STDC_LIB_EXT1__
  set_constraint_handler_s(ignore_handler_s);
  errno_t r = strncpy_s((char *)dialect->lineterminator,
                        sizeof dialect->lineterminator,
                        lineterminator,
                        sizeof lineterminator);
  /* r == 0 on success */
  if (r != 0) {
    ZF_LOGE("Error using strncpy_s");
    free((void *)dialect->lineterminator);
    dialect->lineterminator = NULL;
    return csvreturn_init(false);
  }
#else
  if (strncpy((char *)dialect->lineterminator,
              lineterminator,
              sizeof dialect->lineterminator) == NULL) {
    ZF_LOGE("Error using strncpy");
    free((void *)dialect->lineterminator);
    dialect->lineterminator = NULL;
    return csvreturn_init(false);
  }
#endif /* __STDC_LIB_EXT1__ */

  ZF_LOGV("Source lineterminator: '%s' Destination Lineterminator: `%s`",
          lineterminator,
          dialect->lineterminator);

  return csvreturn_init(true);
}

const char *csvdialect_get_lineterminator(csvdialect dialect, size_t *length) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    *length = 0;
    return NULL;
  }

  ZF_LOGD("lineterminator `%s` length `%lu`",
          dialect->lineterminator,
          (unsigned long)dialect->lineterminator_length);
  *length = dialect->lineterminator_length;
  return dialect->lineterminator;
}

csvreturn csvdialect_set_quotechar(csvdialect               dialect,
                                   csv_comparison_char_type quotechar) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return csvreturn_init(false);
  }

  ZF_LOGD("quotechar `%c`", (char)quotechar);
  dialect->quotechar = quotechar;
  return csvreturn_init(true);
}

csv_comparison_char_type csvdialect_get_quotechar(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return CSV_UNDEFINED_CHAR;
  }

  ZF_LOGD("quotechar `%c`", (char)dialect->quotechar);
  return dialect->quotechar;
}

csvreturn csvdialect_set_quotestyle(csvdialect  dialect,
                                    QUOTE_STYLE quotestyle) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return csvreturn_init(false);
  }

  ZF_LOGD("quotestyle `%s`", quote_style(quotestyle));
  dialect->quotestyle = quotestyle;
  return csvreturn_init(true);
}

QUOTE_STYLE csvdialect_get_quotestyle(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return QUOTE_STYLE_MINIMAL;
  }

  ZF_LOGD("quotestyle `%s`", quote_style(dialect->quotestyle));
  return dialect->quotestyle;
}

csvreturn csvdialect_set_skipinitialspace(csvdialect dialect,
                                          bool       skipinitialspace) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return csvreturn_init(false);
  }

  ZF_LOGD("skipinitialspace `%s`", skipinitialspace ? "true" : "false");
  dialect->skipinitialspace = skipinitialspace;
  return csvreturn_init(true);
}

bool csvdialect_get_skipinitialspace(csvdialect dialect) {
  if (dialect == NULL) {
    ZF_LOGE("`dialect` value is NULL");
    return false;
  }

  ZF_LOGD("skipinitialspace `%s`",
          dialect->skipinitialspace ? "true" : "false");
  return dialect->skipinitialspace;
}

/**
 * @endcond
 */
