#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "csv.h"
#include "dialect_private.h"

struct csv_dialect {
  QUOTE_STYLE quotestyle;
  char        delimiter, escapechar, quotechar,
              lineterminator[CSV_LINETERMINATOR_MAX + 1];
  bool doublequote, skipinitialspace;
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

  if (csv_failure(csvdialect_set_lineterminator(dialect, "\n", 1))) {
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

  size_t size_b     = 0;
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
                                                  output,
                                                  &size_b), size_b))) {
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
  csvreturn rc = csvreturn_init();

  if (dialect == NULL) {
    rc.dialect_null = 1;
    return rc;
  }

  if (csvdialect_get_delimiter(dialect) == CSV_UNDEFINED_CHAR) {
    rc.delimiter_error = 1;
    return rc;
  }

  size_t size_b;
  const char *string_b;

  string_b = csvdialect_get_lineterminator(dialect, &size_b);

  if ((size_b == 0) || (string_b == NULL)) {
    rc.lineterminator_error = 1;
    return rc;
  }

  free((void *)string_b);
  string_b = NULL;

  if (!csvdialect_get_doublequote(dialect)) {
    if (csvdialect_get_escapechar(dialect) == CSV_UNDEFINED_CHAR) {
      rc.quoteescape_error = 1;
      return rc;
    }
  }

  /* all checks passed, return success */
  rc.succeeded = 1;
  return rc;
}

csvreturn csvdialect_set_delimiter(csvdialect dialect, char delimiter) {
  csvreturn rc = csvreturn_init();

  if (dialect == NULL) return rc;

  dialect->delimiter = delimiter;
  rc.succeeded       = 1;
  return rc;
}

char csvdialect_get_delimiter(csvdialect dialect) {
  if (dialect == NULL) return CSV_UNDEFINED_CHAR;

  return dialect->delimiter;
}

csvreturn csvdialect_set_doublequote(csvdialect dialect,
                                     bool       doublequote) {
  csvreturn rc = csvreturn_init();

  if (dialect == NULL) return rc;

  dialect->doublequote = doublequote;
  rc.succeeded         = 1;
  return rc;
}

bool csvdialect_get_doublequote(csvdialect dialect) {
  if (dialect == NULL) return true;

  return dialect->doublequote;
}

csvreturn csvdialect_set_escapechar(csvdialect dialect,
                                    char       escapechar) {
  csvreturn rc = csvreturn_init();

  if (dialect == NULL) return rc;

  dialect->escapechar = escapechar;
  rc.succeeded        = 1;
  return rc;
}

char csvdialect_get_escapechar(csvdialect dialect) {
  if (dialect == NULL) return CSV_UNDEFINED_CHAR;

  return dialect->escapechar;
}

csvreturn csvdialect_set_lineterminator(csvdialect  dialect,
                                        const char *lineterminator,
                                        size_t      length) {
  size_t i;
  csvreturn rc = csvreturn_init();

  if ((dialect == NULL) || (lineterminator == NULL)) return rc;

  if (length == 0) {
    length = strlen(lineterminator);
  }
  memset(&dialect->lineterminator[0], 0, CSV_LINETERMINATOR_MAX + 1);

  for (i = 0; i < length; ++i) {
    dialect->lineterminator[i] = lineterminator[i];
  }
  rc.succeeded = 1;
  return rc;
}

const char* csvdialect_get_lineterminator(csvdialect dialect,
                                          size_t    *length) {
  if (dialect == NULL) {
    if (length != NULL) *length = CSV_UNDEFINED_STRING_LENGTH;
    return NULL;
  }
  else if (dialect->lineterminator == NULL) {
    if (length != NULL) *length = 0;
    return NULL;
  }

  if (length != NULL) *length = strlen(dialect->lineterminator);
  return (const char *)&dialect->lineterminator[0];
}

csvreturn csvdialect_set_quotechar(csvdialect dialect,
                                   char       quotechar) {
  csvreturn rc = csvreturn_init();

  if (dialect == NULL) return rc;

  dialect->quotechar = quotechar;
  rc.succeeded       = 1;
  return rc;
}

char csvdialect_get_quotechar(csvdialect dialect) {
  if (dialect == NULL) return CSV_UNDEFINED_CHAR;

  return dialect->quotechar;
}

csvreturn csvdialect_set_quotestyle(csvdialect  dialect,
                                    QUOTE_STYLE quotestyle) {
  csvreturn rc = csvreturn_init();

  if (dialect == NULL) return rc;

  dialect->quotestyle = quotestyle;
  rc.succeeded        = 1;
  return rc;
}

QUOTE_STYLE csvdialect_get_quotestyle(csvdialect dialect) {
  if (dialect == NULL) return QUOTE_STYLE_MINIMAL;

  return dialect->quotestyle;
}

csvreturn csvdialect_set_skipinitialspace(csvdialect dialect,
                                          bool       skipinitialspace) {
  csvreturn rc = csvreturn_init();

  if (dialect == NULL) return rc;

  dialect->skipinitialspace = skipinitialspace;
  rc.succeeded              = 1;
  return rc;
}

bool csvdialect_get_skipinitialspace(csvdialect dialect) {
  if (dialect == NULL) return false;

  return dialect->skipinitialspace;
}
