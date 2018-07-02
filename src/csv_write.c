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

/*
 * private forward declarations
 */
typedef struct csv_file_writer *csvfilewriter;
csvfilewriter csvfilewriter_filepath_init(const char *filepath);
void          csvfilewriter_filepath_closer(csvstream_type streamdata);
void          csvwriter_setrecord(csvstream_type       streamdata,
                                  CSV_CHAR_TYPE        char_type,
                                  const csvrecord_type record,
                                  size_t               length);
CSV_STREAM_SIGNAL csvwriter_setnextfield(csvstream_type streamdata,
                                         size_t        *length);
void              csvwriter_resetfield(csvstream_type streamdata);
CSV_STREAM_SIGNAL csvwriter_getnextchar(csvstream_type            streamdata,
                                        CSV_CHAR_TYPE            *char_type,
                                        csv_comparison_char_type *value);
void              csvwriter_writechar(csvstream_type           streamdata,
                                      CSV_CHAR_TYPE            char_type,
                                      csv_comparison_char_type value);

/*
 * public implmentations
 */
struct csv_writer {
  csvdialect             dialect;
  csvstream_type         streamdata;
  csvstream_setrecord    setrecord;
  csvstream_setnextfield setnextfield;
  csvstream_resetfield   resetfield;
  csvstream_getnextchar  getnextchar;
  csvstream_writechar    writechar;
  csvstream_close        closer;
};

csvwriter csvwriter_init(csvdialect  dialect,
                         const char *filepath) {
  csvfilewriter filewriter = NULL;
  csvwriter     writer     = NULL;

  if ((filewriter = csvfilewriter_filepath_init(filepath)) == NULL) {
    return NULL;
  }

  writer = csvwriter_advanced_init(dialect,
                                   &csvwriter_setrecord,
                                   &csvwriter_setnextfield,
                                   &csvwriter_resetfield,
                                   &csvwriter_getnextchar,
                                   &csvwriter_writechar,
                                   filewriter);

  /* csvwriter_set_closer is NULL safe */
  writer = csvwriter_set_closer(writer, &csvfilewriter_filepath_closer);

  if (writer == NULL) {
    csvfilewriter_filepath_closer(filewriter);
    return NULL;
  }

  return writer;
}

csvwriter csvwriter_file_init(csvdialect dialect,
                              FILE      *fileobj) {
  /* TODO: implement :) */
  return NULL;
}

csvwriter csvwriter_advanced_init(csvdialect             dialect,
                                  csvstream_setrecord    setrecord,
                                  csvstream_setnextfield setnextfield,
                                  csvstream_resetfield   resetfield,
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
  writer->resetfield   = resetfield;
  writer->getnextchar  = getnextchar;
  writer->writechar    = writechar;
  writer->closer       = NULL;

  return writer;
}

csvwriter csvwriter_set_closer(csvwriter       writer,
                               csvstream_close closer) {
  /* short circuit if bad writer is supplied */
  if (writer == NULL) {
    return NULL;
  }

  writer->closer = closer;
  return writer;
}

void csvwriter_close(csvwriter *writer) {
  /* short circuit if bad writer is supplied */
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
  CSV_STREAM_SIGNAL field_signal, stream_signal;
  csvfield_type     lineterminator;
  size_t lineterminator_cnt;

  if (writer == NULL) {
    rc = csvreturn_init(false);
    return rc;
  }

  (*writer->setrecord)(writer->streamdata, char_type, record, length);
  quote_style = csvdialect_get_quotestyle(writer->dialect);

  for (i = 0; i < length; ++i) {
    field_signal = (*writer->setnextfield)(writer->streamdata, &field_len);
    ZF_LOGD("Field # %zu - Field Length: %zu - Field Signal: %u", i, field_len, field_signal);

    if (field_signal == CSV_ERROR) {
      break;
    }

    switch (quote_style) {
    /* never need to check to see if quoting is required on these two */
    case QUOTE_STYLE_ALL:
      ZF_LOGD("QUOTE_STYLE_ALL - force quoting");
      needs_quoting = true;
      break;

    case QUOTE_STYLE_NONE:
      ZF_LOGD("QUOTE_STYLE_NONE - force no quoting");
      needs_quoting = false;
      break;

    case QUOTE_STYLE_MINIMAL:
    default:
      ZF_LOGD("QUOTE_STYLE_MINIMAL - must determine if quoting is required");

      /* need to check the field for the following to determine quoting:
       * - delimiter
       * - any newline characters
       * - quoting character
       * - escape character
       */
      for (j = 0; j < field_len; ++j) {
        stream_signal =
          (*writer->getnextchar)(writer->streamdata, &char_type, &value);

        if (stream_signal == CSV_ERROR) {
          break;
        }

        if (
          (value == csvdialect_get_delimiter(writer->dialect)) ||
          (value == csvdialect_get_quotechar(writer->dialect)) ||
          (value == csvdialect_get_escapechar(writer->dialect)) ||
          (value == '\n') ||
          (value == '\r')
          ) {
          ZF_LOGV("value == delimiter? %s",
                  (value == csvdialect_get_delimiter(writer->dialect)) ? "true" : "false");
          ZF_LOGV("value == quotechar? %s",
                  (value == csvdialect_get_quotechar(writer->dialect)) ? "true" : "false");
          ZF_LOGV("value == escapechar? %s",
                  (value == csvdialect_get_escapechar(writer->dialect)) ? "true" : "false");
          ZF_LOGV("value == newline? %s",
                  (value == '\n') ? "true" : "false");
          ZF_LOGV("value == carriage return? %s",
                  (value == '\r') ? "true" : "false");
          needs_quoting = true;
          break;
        }
      }

      ZF_LOGD(
        needs_quoting ? "QUOTE_STYLE_MINIMAL - field requires quoting" : "QUOTE_STYLE_MINIMAL - field does not require quoting");
      break;
    }

    /*
     * write the delimiter after the first iteration. Ensures trailing
     * delimiter is not written to the stream.
     *
     * avoids writing a trailing delimiter
     */
    if (i > 0) {
      ZF_LOGD("Writing delimiter character");
      (*writer->writechar)(writer->streamdata, char_type,
                           csvdialect_get_delimiter(writer->dialect));
    }

    /* ensure field position value is set to zero */
    (*writer->resetfield)(writer->streamdata);

    /* initial quote -- outside the loop */
    if (needs_quoting) {
      (*writer->writechar)(writer->streamdata, char_type,
                           csvdialect_get_quotechar(writer->dialect));
    }

    for (j = 0; j < field_len; ++j) {
      stream_signal =
        (*writer->getnextchar)(writer->streamdata, &char_type, &value);

      if (stream_signal == CSV_ERROR) {
        ZF_LOGD("Encountered CSV_ERROR");
        break;
      }
      else if (stream_signal == CSV_END_OF_FIELD) {
        ZF_LOGD("Encountered CSV_END_OF_FIELD");
        break;
      }

      ZF_LOGD("Value `%c`", (char)value);

      /* apply escape character, if neccessary */
      if (!needs_quoting) {
        if (
          (value == csvdialect_get_delimiter(writer->dialect)) ||
          (value == csvdialect_get_quotechar(writer->dialect)) ||
          (value == csvdialect_get_escapechar(writer->dialect)) ||
          (value == '\n') ||
          (value == '\r')
          ) {
          ZF_LOGV("value == delimiter? %s",
                  (value == csvdialect_get_delimiter(writer->dialect)) ? "true" : "false");
          ZF_LOGV("value == quotechar? %s",
                  (value == csvdialect_get_quotechar(writer->dialect)) ? "true" : "false");
          ZF_LOGV("value == escapechar? %s",
                  (value == csvdialect_get_escapechar(writer->dialect)) ? "true" : "false");
          ZF_LOGV("value == newline? %s",
                  (value == '\n') ? "true" : "false");
          ZF_LOGV("value == carriage return? %s",
                  (value == '\r') ? "true" : "false");

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

  lineterminator = (csvfield_type)csv_lineterminator_type(
    csvdialect_get_lineterminator(writer->dialect), char_type);

  lineterminator_cnt = 0;
  value              = 0;

  /* max line terminator length is 2 */
  while (lineterminator_cnt < 2) {
    switch (char_type) {
    case CSV_CHAR:
    case CSV_UCHAR8:
    default:
      value =
        (csv_comparison_char_type)((char **)(lineterminator))[lineterminator_cnt++
        ];
      break;
    }

    if (value == 0) {
      break;
    }

    (*writer->writechar)(writer->streamdata, char_type, value);
  }

  rc = csvreturn_init(true);
  return rc;
}

/*
 * internal struct for @c streamdata
 *
 * no public API provided, implementation not guaranteed
 */
struct csv_file_writer {
  const char    *filepath;   /*< filepath, if provided to data source */
  FILE          *file;       /*< output stream */
  CSV_CHAR_TYPE  char_type;  /*< character type of the record and field */
  csvrecord_type record;     /*< input record */
  csvfield_type  field;      /*< input field */
  size_t         capacity_r; /*< length of the input record */
  size_t         capacity_f; /*< length of the current field */
  size_t         position_r; /*< current position in the record */
  size_t         position_f; /*< current position in the field */
};

/*
 * initialize raw csv file writer pointer
 */
csvfilewriter csvfilewriter_init(void) {
  ZF_LOGD("Initializing base CSV File Writer");
  csvfilewriter output = NULL;

  if ((output = malloc(sizeof *output)) == NULL) {
    ZF_LOGD("ERROR - Could not allocate base CSV File Writer");
    return NULL;
  }

  output->filepath   = NULL;
  output->file       = NULL;
  output->char_type  = CSV_UNDEFINED;
  output->record     = NULL;
  output->field      = NULL;
  output->capacity_r = 0;
  output->capacity_f = 0;
  output->position_r = 0;
  output->position_f = 0;

  return output;
}

/*
 * @brief Initialize CSV File Writer from filepath
 *
 * @param[in] filepath filepath to output destination
 *
 * @return fully initialized @c struct @c csv_file_writer which can be used as
 *         @p streamdata in @c csvwriter_advanced_init
 */
csvfilewriter csvfilewriter_filepath_init(const char *filepath) {
  ZF_LOGD("Initializing filepath CSV File Writer");

  if (filepath == NULL) {
    ZF_LOGD("ERROR - NULL value passed for `filepath`");
    return NULL;
  }

  FILE *outfile = NULL;

  if ((outfile = fopen(filepath, "wb")) == NULL) {
    ZF_LOGD("ERROR - could not allocate `FILE*` for filepath: `%s`", filepath);
    return NULL;
  }

  csvfilewriter filewriter = csvfilewriter_init();

  filewriter->filepath = filepath;
  filewriter->file     = outfile;

  return filewriter;
}

/*
 * @brief Close CSV File Writer struct
 *
 * Implements @c csvstream_close callback interface
 *
 * @param[in] filewriter pointer to @c struct @c csv_file_writer to be closed
 *
 * @see csvstream_close
 */
void csvfilewriter_filepath_closer(csvstream_type streamdata) {
  ZF_LOGD("Closing CSV File Writer");

  if (streamdata == NULL) {
    ZF_LOGD("`streamdata` was NULL, cannot close any resources");
    return;
  }
  csvfilewriter filewriter = (csvfilewriter)streamdata;

  if (filewriter->file != NULL) {
    ZF_LOGD("Closing `file` attribute");
    fclose(filewriter->file);
  }
  ZF_LOGD("Freeing CSV File Writer resources");
  free(filewriter);
}

/*
 * sets provided record as active
 *
 * @see csvstream_setrecord
 */
void csvwriter_setrecord(csvstream_type       streamdata,
                         CSV_CHAR_TYPE        char_type,
                         const csvrecord_type record,
                         size_t               length) {
  ZF_LOGD("Setting next record for CSV Writer - length %zu", length);

  if (streamdata == NULL) {
    ZF_LOGD("`streamdata` is NULL -- exiting early");
    return;
  }
  else if (char_type == CSV_UNDEFINED) {
    ZF_LOGD("`char_type` set to CSV_UNDEFINED -- exiting early");
    return;
  }
  else if (record == NULL) {
    ZF_LOGD("`record` is NULL -- exiting early");
    return;
  }
  else if (length == 0) {
    ZF_LOGD("`length` is 0 -- exiting early");
    return;
  }

  csvfilewriter filewriter = (csvfilewriter)streamdata;
  filewriter->char_type  = char_type;
  filewriter->record     = (csvrecord_type)record;
  filewriter->capacity_r = length;
  filewriter->position_r = 0;
}

/*
 * TODO: size_t (*csvstream_setnextfield)(csvstream_type streamdata);
 */
CSV_STREAM_SIGNAL csvwriter_setnextfield(csvstream_type streamdata,
                                         size_t        *length) {
  ZF_LOGD("Setting next field for CSV Writer");

  if (streamdata == NULL) {
    ZF_LOGD("`streamdata` is NULL -- exiting early");
    return CSV_ERROR;
  }

  csvfilewriter filewriter = (csvfilewriter)streamdata;

  if ((filewriter->position_r + 1) >= filewriter->capacity_r) {
    ZF_LOGD("Reached end of record");
    return CSV_EOR;
  }

  size_t next_index = filewriter->position_r;
  char **record, *field;

  filewriter->position_r++;
  record = (char **)(filewriter->record);
  field  = record[next_index];
  ZF_LOGD("Field # %lu Value: %s", (long unsigned)next_index, field);

  /* default implementation is for stdio `char` returning files */
  filewriter->char_type  = CSV_CHAR;
  filewriter->field      = field;
  filewriter->capacity_f = strlen(field) + 1;
  filewriter->position_f = 0;

  *length = filewriter->capacity_f;

  return CSV_GOOD;
}

/*
 * @brief reset field to beginning of stream
 *
 * @param[in] streamdata container for field and field pointer counter
 */
void csvwriter_resetfield(csvstream_type streamdata) {
  ZF_LOGD("Resetting field position");

  if (streamdata == NULL) {
    ZF_LOGD("`streamdata` is NULL -- exiting early");
    return;
  }

  csvfilewriter filewriter = (csvfilewriter)streamdata;

  filewriter->char_type  = CSV_CHAR;
  filewriter->capacity_f = strlen((char *)filewriter->field) + 1;
  filewriter->position_f = 0;
}

/*
 * TODO: CSV_STREAM_SIGNAL (*csvstream_getnextchar)(csvstream_type
                                                    streamdata,
                                                    csv_comparison_char_type
 **value);
 */
CSV_STREAM_SIGNAL csvwriter_getnextchar(csvstream_type            streamdata,
                                        CSV_CHAR_TYPE            *char_type,
                                        csv_comparison_char_type *value) {
  ZF_LOGD("Getting next character from current active input field");

  if (streamdata == NULL) {
    ZF_LOGD("`streamdata` is NULL -- exiting early");
    return CSV_ERROR;
  }
  *char_type = CSV_CHAR;
  csvfilewriter filewriter = (csvfilewriter)streamdata;

  if ((filewriter->position_f + 1) >= filewriter->capacity_f) {
    ZF_LOGD("Reached end of field");
    return CSV_END_OF_FIELD;
  }

  char *field = (char *)filewriter->field;
  *value = field[filewriter->position_f++];
  ZF_LOGD("Retrieved next character: `%c`", (char)(*value));

  return CSV_GOOD;
}

/*
 * TODO: void (*csvstream_writechar)(csvstream_type           streamdata,
                                     CSV_CHAR_TYPE            char_type,
                                     csv_comparison_char_type value);
 */
void csvwriter_writechar(csvstream_type           streamdata,
                         CSV_CHAR_TYPE            char_type,
                         csv_comparison_char_type value) {
  ZF_LOGD("Writing next character to output stream");

  if (streamdata == NULL) {
    ZF_LOGD("`streamdata` is NULL -- exiting early");
    return;
  }

  csvfilewriter filewriter = (csvfilewriter)streamdata;
  int rc                   = 0;

  if (filewriter->file == NULL) {
    ZF_LOGD("`streamdata->file` is NULL -- exiting early");
    return;
  }

  if ((rc = putc((char)value, filewriter->file)) != EOF) {
    ZF_LOGD("Wrote `%c` to output stream", (char)value);
    return;
  }

  if (ferror(filewriter->file)) {
    ZF_LOGD("putc() error");
    perror("putc() error");
    fprintf(stderr, "putc(`%c`, filewriter->file) failed. Raw value: `%lld`",
            (char)value, value);
    exit(EXIT_FAILURE);
  }
}
