#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: define CSV log level namespace
#ifndef ZF_LOG_LEVEL
# define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#endif /* ZF_LOG_LEVEL */
#include "zf_log.h"

// #include "csv.h"
#include "csv/version.h"
#include "csv/definitions.h"
#include "csv/dialect.h"
#include "csv/stream.h"
#include "csv/write.h"

#include "dialect_private.h"

typedef struct csv_file_writer *csvfilewriter;

struct csv_writer {
  csvdialect             dialect;
  csvstream_type         streamdata;
  csvstream_setrecord    setrecord;
  csvstream_setnextfield setnextfield;
  csvstream_getnextchar  getnextchar;
  csvstream_writechar    writechar;
  csvstream_close        closer;
};

csvwriter csvwriter_init(csvdialect  dialect,
                         const char *filepath) {
  return NULL;
}

csvwriter csvwriter_file_init(csvdialect dialect,
                              FILE      *fileobj) {
  return NULL;
}

csvwriter csvwriter_advanced_init(csvdialect             dialect,
                                  csvstream_setrecord    setrecord,
                                  csvstream_setnextfield setnextfield,
                                  csvstream_getnextchar  getnextchar,
                                  csvstream_writechar    writechar,
                                  csvstream_type         streamdata) {
  csvwriter writer;

  dialect = (dialect == NULL) ? csvdialect_init() : csvdialect_copy(dialect);

  if (dialect == NULL) {
    return NULL;
  }

  if ((writer = malloc(sizeof *writer)) == NULL) {
    return NULL;
  }

  writer->dialect      = dialect;
  writer->streamdata   = streamdata;
  writer->setrecord    = setrecord;
  writer->setnextfield = setnextfield;
  writer->getnextchar  = getnextchar;
  writer->writechar    = writechar;
  writer->closer       = NULL;

  return writer;
}

csvwriter csvwriter_set_closer(csvwriter       writer,
                               csvstream_close closer) {
  if (writer == NULL) {
    return NULL;
  }

  writer->closer = closer;
  return writer;
}

void csvwriter_close(csvwriter *writer) {
  if ((*writer) == NULL) {
    return;
  }

  if ((*writer)->closer != NULL) {
    (*(*writer)->closer)((*writer)->streamdata);
  }

  if ((*writer)->dialect != NULL) {
    csvdialect_close(&((*writer)->dialect));
  }

  free(*writer);
  *writer = NULL;
}

/*
 * completed, pending validation
 */
csvreturn csvwriter_next_record(csvwriter            writer,
                                CSV_CHAR_TYPE        char_type,
                                const csvrecord_type record,
                                size_t               length) {
  csvreturn rc;
  size_t    i, j, field_len;
  csv_comparison_char_type value;
  bool needs_quoting;
  QUOTE_STYLE quote_style;
  CSV_STREAM_SIGNAL stream_signal;

  if (writer == NULL) {
    rc = csvreturn_init(false);
  }
  else {
    rc = csvreturn_init(true);
    (*writer->setrecord)(writer->streamdata, char_type, record, length);
    quote_style = csvdialect_get_quotestyle(writer->dialect);

    for (i = 0; i < length; ++i) {
      field_len = (*writer->setnextfield)(writer->streamdata);

      switch (quote_style) {
      case QUOTE_STYLE_ALL:
        needs_quoting = true;
        break;

      case QUOTE_STYLE_NONE:
        needs_quoting = false;

        /* never need to check to see if quoting is required on these two */
        break;

      case QUOTE_STYLE_MINIMAL:
      default:

        /* need to check the field for the following to determine quoting:
         * - delimiter
         * - any newline characters
         * - quoting character
         * - escape character
         */
        for (j = 0; j < field_len; ++j) {
          stream_signal = (*writer->getnextchar)(writer->streamdata, &value);

          if (
            (value == csvdialect_get_delimiter(writer->dialect)) ||
            (value == csvdialect_get_quotechar(writer->dialect)) ||
            (value == csvdialect_get_escapechar(writer->dialect)) ||
            (value == '\n') ||
            (value == '\r')
            ) {
            needs_quoting = true;
            break;
          }
        }
        break;
      }

      if (i > 0) {
        /*
         * write the delimiter after the first iteration. Ensures trailing
         * delimiter is not written to the stream
         */
        (*writer->writechar)(writer->streamdata, char_type,
                             csvdialect_get_delimiter(writer->dialect));
      }


      /* initial quote -- outside the loop */
      if (needs_quoting) {
        (*writer->writechar)(writer->streamdata, char_type,
                             csvdialect_get_quotechar(writer->dialect));
      }

      for (j = 0; j < field_len; ++j) {
        stream_signal = (*writer->getnextchar)(writer->streamdata, &value);

        /* apply escape character, if neccessary */
        if (!needs_quoting) {
          if (
            (value == csvdialect_get_delimiter(writer->dialect)) ||
            (value == csvdialect_get_quotechar(writer->dialect)) ||
            (value == csvdialect_get_escapechar(writer->dialect)) ||
            (value == '\n') ||
            (value == '\r')
            ) {
            (*writer->writechar)(writer->streamdata, char_type,
                                 csvdialect_get_escapechar(writer->dialect));
          }
        }
        else {
          /* quote in quoted field */
          if (value == csvdialect_get_quotechar(writer->dialect)) {
            if (csvdialect_get_doublequote(writer->dialect)) {
              /* double the quoting character to escape */
              (*writer->writechar)(writer->streamdata, char_type,
                                   csvdialect_get_quotechar(writer->dialect));
            }
            else {
              /* apply the escape character */
              (*writer->writechar)(writer->streamdata, char_type,
                                   csvdialect_get_escapechar(writer->dialect));
            }
          }
        }

        /* write the actual character to the stream */
        (*writer->writechar)(writer->streamdata, char_type, value);
      }

      /* final quote -- outside the loop */
      if (needs_quoting) {
        (*writer->writechar)(writer->streamdata, char_type,
                             csvdialect_get_quotechar(writer->dialect));
      }
    }
  }
  return rc;
}

/*
 * internal struct for @c streamdata
 *
 * no public API provided, implementation not guaranteed
 */
struct csv_file_writer {
  const char    *filepath;
  FILE          *file;
  csvrecord_type record;
  csvfield_type  field;
  char          *buffer;
  size_t         record_len;
  size_t         field_len;
  size_t         record_position;
  size_t         field_position;
};
