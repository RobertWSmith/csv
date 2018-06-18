#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csv.h"
#include "dialect_private.h"

struct csv_dialect {
  QUOTE_STYLE             quotestyle;
  CSV_LINETERMINATOR_TYPE lineterminator;
  unsigned int            delimiter, escapechar, quotechar;
  int                     doublequote, skipinitialspace;
};

csvdialect csvdialect_init(void) {
  csvdialect dialect = NULL;

  if ((dialect = malloc(sizeof *dialect)) == NULL) return NULL;

  if (csv_failure(csvdialect_set_delimiter(dialect, ','))) {
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_doublequote(dialect, true))) {
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_escapechar(dialect, CSV_UNDEFINED_CHAR))) {
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_lineterminator(dialect,
                                                LINETERMINATOR_SYSTEM_DEFAULT))) {
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_quotechar(dialect, '"'))) {
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_quotestyle(dialect, QUOTE_STYLE_MINIMAL))) {
    csvdialect_close(&dialect);
    return NULL;
  }

  if (csv_failure(csvdialect_set_skipinitialspace(dialect, false))) {
    csvdialect_close(&dialect);
    return NULL;
  }

  return dialect;
}

csvdialect csvdialect_copy(csvdialect dialect) {
  if (dialect == NULL) return NULL;

  csvdialect output = csvdialect_init();

  if (csv_failure(csvdialect_set_delimiter(dialect,
                                           csvdialect_get_delimiter(output)))) {
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_doublequote(dialect,
                                             csvdialect_get_doublequote(output))))
  {
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_escapechar(dialect,
                                            csvdialect_get_escapechar(output)))) {
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_lineterminator(dialect,
                                                csvdialect_get_lineterminator(
                                                  output)))) {
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_quotechar(dialect,
                                           csvdialect_get_quotechar(output)))) {
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_quotestyle(dialect,
                                            csvdialect_get_quotestyle(output)))) {
    csvdialect_close(&output);
    return NULL;
  }

  if (csv_failure(csvdialect_set_skipinitialspace(dialect,
                                                  csvdialect_get_skipinitialspace(
                                                    output)))) {
    csvdialect_close(&output);
    return NULL;
  }

  return output;
}

void csvdialect_close(csvdialect *dialect) {
  if ((*dialect) == NULL) return;

  free(*dialect);
  *dialect = NULL;
}

csvreturn csvdialect_validate(csvdialect dialect) {
  csvreturn rc = csvreturn_init(false);

  if (dialect == NULL) {
    rc.dialect_null = 1;
    return rc;
  }

  if (csvdialect_get_delimiter(dialect) == CSV_UNDEFINED_CHAR) {
    rc.delimiter_error = 1;
    return rc;
  }

  /*
   * size_t size_b;
   * const unsigned int *string_b;
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

  if (!csvdialect_get_doublequote(dialect))
    if (csvdialect_get_escapechar(dialect) == CSV_UNDEFINED_CHAR) {
      rc.quoteescape_error = 1;
      return rc;
    }

  /* all checks passed, return success */
  rc.succeeded = 1;
  return rc;
}

csvreturn csvdialect_set_delimiter(csvdialect   dialect,
                                   unsigned int delimiter) {
  if (dialect == NULL) return csvreturn_init(false);

  dialect->delimiter = delimiter;
  return csvreturn_init(true);
}

unsigned int csvdialect_get_delimiter(csvdialect dialect) {
  if (dialect == NULL) return CSV_UNDEFINED_CHAR;

  return dialect->delimiter;
}

csvreturn csvdialect_set_doublequote(csvdialect dialect,
                                     int        doublequote) {
  if (dialect == NULL) return csvreturn_init(false);

  dialect->doublequote = doublequote;
  return csvreturn_init(true);
}

int csvdialect_get_doublequote(csvdialect dialect) {
  if (dialect == NULL) return true;

  return dialect->doublequote;
}

csvreturn csvdialect_set_escapechar(csvdialect   dialect,
                                    unsigned int escapechar) {
  if (dialect == NULL) return csvreturn_init(false);

  dialect->escapechar = escapechar;
  return csvreturn_init(true);
}

unsigned int csvdialect_get_escapechar(csvdialect dialect) {
  if (dialect == NULL) return CSV_UNDEFINED_CHAR;

  return dialect->escapechar;
}

csvreturn csvdialect_set_lineterminator(csvdialect              dialect,
                                        CSV_LINETERMINATOR_TYPE lineterminator) {
  if (dialect == NULL) return csvreturn_init(false);

  dialect->lineterminator = lineterminator;
  return csvreturn_init(true);
}

CSV_LINETERMINATOR_TYPE csvdialect_get_lineterminator(csvdialect dialect) {
  if (dialect == NULL) return LINETERMINATOR_SYSTEM_DEFAULT;

  return dialect->lineterminator;
}

csvreturn csvdialect_set_quotechar(csvdialect   dialect,
                                   unsigned int quotechar) {
  if (dialect == NULL) return csvreturn_init(false);

  dialect->quotechar = quotechar;
  return csvreturn_init(true);
}

unsigned int csvdialect_get_quotechar(csvdialect dialect) {
  if (dialect == NULL) return CSV_UNDEFINED_CHAR;

  return dialect->quotechar;
}

csvreturn csvdialect_set_quotestyle(csvdialect  dialect,
                                    QUOTE_STYLE quotestyle) {
  if (dialect == NULL) return csvreturn_init(false);

  dialect->quotestyle = quotestyle;
  return csvreturn_init(true);
}

QUOTE_STYLE csvdialect_get_quotestyle(csvdialect dialect) {
  if (dialect == NULL) return QUOTE_STYLE_MINIMAL;

  return dialect->quotestyle;
}

csvreturn csvdialect_set_skipinitialspace(csvdialect dialect,
                                          int        skipinitialspace) {
  if (dialect == NULL) return csvreturn_init(false);

  dialect->skipinitialspace = skipinitialspace;
  return csvreturn_init(true);
}

int csvdialect_get_skipinitialspace(csvdialect dialect) {
  if (dialect == NULL) return false;

  return dialect->skipinitialspace;
}
