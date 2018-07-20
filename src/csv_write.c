/**
 * @cond INTERNAL
 * @file csv_write.c
 */

#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csv.h"
#include "dialect_private.h"

// #include "csv/definitions.h"
// #include "csv/version.h"
//
// #include "csv/dialect.h"
//
// #include "csv/stream.h"
// #include "csv/write.h"

/*
 * private forward declarations
 */
typedef struct csv_file_writer *csvfilewriter;

csvfilewriter     csvfilewriter_init(void);
csvfilewriter     csvfilewriter_filepath_init(const char *filepath);
csvfilewriter     csvfilewriter_file_init(FILE *fileobj);
void              csvfilewriter_filepath_closer(csvstream_type streamdata);
void              csvfilewriter_file_closer(csvstream_type streamdata);
void              csvwriter_setrecord(csvstream_type streamdata,
                                      const char **  record,
                                      size_t         length);
CSV_STREAM_SIGNAL csvwriter_setnextfield(csvstream_type streamdata,
                                         size_t *       length);
void              csvwriter_resetfield(csvstream_type streamdata);
CSV_STREAM_SIGNAL csvwriter_getnextchar(csvstream_type            streamdata,
                                        csv_comparison_char_type *value);
void              csvwriter_writechar(csvstream_type           streamdata,
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

csvwriter csvwriter_init(csvdialect dialect, const char *filepath) {
  csvfilewriter filewriter = NULL;
  csvwriter     writer     = NULL;

  if ((filewriter = csvfilewriter_filepath_init(filepath)) == NULL) {
    ZF_LOGE("CSV Writer initialization from filepath failed");
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
    ZF_LOGE(
        "CSV Writer initialization from filepath failed after advanced "
        "initializer");
    csvfilewriter_filepath_closer(filewriter);
    return NULL;
  }

  return writer;
}

csvwriter csvwriter_file_init(csvdialect dialect, FILE *fileobj) {
  csvfilewriter filewriter = NULL;
  csvwriter     writer     = NULL;

  if ((filewriter = csvfilewriter_file_init(fileobj)) == NULL) {
    ZF_LOGE("CSV Writer initialization from file pointer failed");
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
  writer = csvwriter_set_closer(writer, &csvfilewriter_file_closer);

  if (writer == NULL) {
    ZF_LOGE(
        "CSV Writer initialization from file pointer failed after advanced "
        "initializer");
    csvfilewriter_file_closer(filewriter);
    return NULL;
  }

  return writer;
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

csvwriter csvwriter_set_closer(csvwriter writer, csvstream_close closer) {
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
csvreturn csvwriter_next_record(csvwriter    writer,
                                const char **record,
                                size_t       length) {
  if (writer == NULL) {
    ZF_LOGE("CSV Writer is NULL");
    return csvreturn_init(false);
  } else if (record == NULL) {
    ZF_LOGE("CSV Record is NULL");
    return csvreturn_init(false);
  }

  size_t i, j, field_len, lineterminator_idx, lineterminator_length;
  csv_comparison_char_type value, escapechar, quotechar, delimiter;
  CSV_STREAM_SIGNAL        field_signal, stream_signal;
  QUOTE_STYLE              quote_style;
  bool                     needs_quoting = true;
  const char *             lineterminator =
      csvdialect_get_lineterminator(writer->dialect, &lineterminator_length);

  escapechar  = csvdialect_get_escapechar(writer->dialect);
  quotechar   = csvdialect_get_quotechar(writer->dialect);
  delimiter   = csvdialect_get_delimiter(writer->dialect);
  quote_style = csvdialect_get_quotestyle(writer->dialect);

  if (lineterminator == NULL) {
    ZF_LOGW("Lineterminator is NULL, setting to system default");
    csvdialect_set_lineterminator(writer->dialect,
                                  CSV_LINETERMINATOR_SYSTEM_DEFAULT,
                                  strlen(CSV_LINETERMINATOR_SYSTEM_DEFAULT));
    lineterminator =
        csvdialect_get_lineterminator(writer->dialect, &lineterminator_length);
  }

  if (lineterminator_length == 0) {
    ZF_LOGD("Lineterminator length returned is zero");
    lineterminator_length = strlen(lineterminator);
  }

  (*writer->setrecord)(writer->streamdata, record, length);
  for (i = 0; i < length; ++i) {
    field_signal = (*writer->setnextfield)(writer->streamdata, &field_len);
    ZF_LOGD("Field# %lu - Field Length: %lu - Field Signal: %u",
            (long unsigned)i,
            (long unsigned)field_len,
            field_signal);

    if ((field_signal == CSV_ERROR) || (field_signal == CSV_EOR)) {
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
        ZF_LOGD("QUOTE_STYLE_MINIMAL - must determine if quoting is required");
        needs_quoting = false;

        /* need to check the field for the following to determine quoting:
         * - delimiter
         * - any newline characters
         * - quoting character
         * - escape character
         */
        for (j = 0; j < field_len; ++j) {
          stream_signal = (*writer->getnextchar)(writer->streamdata, &value);

          if (stream_signal == CSV_ERROR) {
            break;
          }

          if ((value == delimiter) || (value == quotechar) ||
              (value == escapechar) || (value == '\n') || (value == '\r')) {
            ZF_LOGV("value == delimiter? %s",
                    (value == delimiter) ? "true" : "false");
            ZF_LOGV("value == quotechar? %s",
                    (value == quotechar) ? "true" : "false");
            ZF_LOGV("value == escapechar? %s",
                    (value == escapechar) ? "true" : "false");
            ZF_LOGV("value == newline? %s", (value == '\n') ? "true" : "false");
            ZF_LOGV("value == carriage return? %s",
                    (value == '\r') ? "true" : "false");
            needs_quoting = true;
            break;
          }
        }

        ZF_LOGD("QUOTE_STYLE_MINIMAL - %s",
                needs_quoting ? "field requires quoting"
                              : "field does not require quoting");
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
      (*writer->writechar)(writer->streamdata, delimiter);
    }

    /* ensure field position value is set to zero */
    (*writer->resetfield)(writer->streamdata);

    /* initial quote -- outside the loop */
    if (needs_quoting) {
      (*writer->writechar)(writer->streamdata, quotechar);
    }

    for (j = 0; j < field_len; ++j) {
      stream_signal = (*writer->getnextchar)(writer->streamdata, &value);

      if (stream_signal == CSV_ERROR) {
        ZF_LOGD("Encountered CSV_ERROR");
        break;
      } else if (stream_signal == CSV_END_OF_FIELD) {
        ZF_LOGD("Encountered CSV_END_OF_FIELD");
        break;
      }

      ZF_LOGD("Value `%c`", (char)value);

      /* apply escape character, if neccessary */
      if (!needs_quoting) {
        if ((value == delimiter) || (value == quotechar) ||
            (value == escapechar) || (value == '\n') || (value == '\r')) {
          ZF_LOGV("value == delimiter? %s",
                  (value == delimiter) ? "true" : "false");
          ZF_LOGV("value == quotechar? %s",
                  (value == quotechar) ? "true" : "false");
          ZF_LOGV("value == escapechar? %s",
                  (value == escapechar) ? "true" : "false");
          ZF_LOGV("value == newline? %s", (value == '\n') ? "true" : "false");
          ZF_LOGV("value == carriage return? %s",
                  (value == '\r') ? "true" : "false");

          (*writer->writechar)(writer->streamdata, escapechar);
        }
      } else {
        /* quote in quoted field */
        if (value == quotechar) {
          if (csvdialect_get_doublequote(writer->dialect)) {
            /* double the quoting character to escape */
            (*writer->writechar)(writer->streamdata, quotechar);
          } else {
            /* apply the escape character */
            (*writer->writechar)(writer->streamdata, escapechar);
          }
        }
      }

      /* write the actual character to the stream */
      (*writer->writechar)(writer->streamdata, value);
    }

    /* final quote -- outside the loop */
    if (needs_quoting) {
      (*writer->writechar)(writer->streamdata, quotechar);
    }
  }

  value = 0;

  for (lineterminator_idx = 0; lineterminator_idx < lineterminator_length;
       ++lineterminator_idx) {
    if ((value = lineterminator[lineterminator_idx]) == '\0') {
      break;
    }
    ZF_LOGV("Lineterminator Index: %lu Value: `%c`",
            (long unsigned)lineterminator_idx,
            value);
    (*writer->writechar)(writer->streamdata, value);
  }

  return csvreturn_init(true);
}

/**
 * @brief internal struct for @c streamdata
 *
 * no public API provided, implementation not guaranteed
 */
struct csv_file_writer {
  const char *filepath;   /**< filepath, if provided to data source */
  FILE *      file;       /**< output stream */
  char **     record;     /**< input record */
  char *      field;      /**< input field */
  size_t      capacity_r; /**< length of the input record */
  size_t      capacity_f; /**< length of the current field */
  size_t      position_r; /**< current position in the record */
  size_t      position_f; /**< current position in the field */
};

/*
 * initialize raw csv file writer pointer
 */
csvfilewriter csvfilewriter_init(void) {
  ZF_LOGD("Initializing base CSV File Writer");
  csvfilewriter output = NULL;

  if ((output = malloc(sizeof *output)) == NULL) {
    ZF_LOGE("Could not allocate base CSV File Writer");
    return NULL;
  }

  output->filepath   = NULL;
  output->file       = NULL;
  output->record     = NULL;
  output->field      = NULL;
  output->capacity_r = 0;
  output->capacity_f = 0;
  output->position_r = 0;
  output->position_f = 0;

  return output;
}

/**
 * @brief Initialize CSV File Writer from filepath
 *
 * @param[in] filepath filepath to output destination
 *
 * @return fully initialized @c struct @c csv_file_writer which can be used as
 *         @p streamdata in @c csvwriter_advanced_init
 */
csvfilewriter csvfilewriter_filepath_init(const char *filepath) {
  ZF_LOGD("Initializing filepath CSV File Writer");
  FILE *        outfile    = NULL;
  csvfilewriter filewriter = NULL;

  if (filepath == NULL) {
    ZF_LOGE("`filepath` is NULL");
    return NULL;
  }

  if ((outfile = fopen(filepath, "wb")) == NULL) {
    ZF_LOGE("Could not allocate `FILE*` for filepath: `%s`", filepath);
    return NULL;
  }

  if ((filewriter = csvfilewriter_init()) == NULL) {
    ZF_LOGE("Could not allocate `csvfilewriter`");
    fclose(outfile);
    return NULL;
  }

  filewriter->filepath = filepath;
  filewriter->file     = outfile;
  return filewriter;
}

/**
 * @brief CSV File Writer - File pointer initializer
 *
 * @param  fileobj @c stdio @c FILE* object, previously initialized
 *
 * @return         Fully initiailize CSV File Writer
 */
csvfilewriter csvfilewriter_file_init(FILE *fileobj) {
  ZF_LOGD("Initializing file pointer CSV File Writer");

  csvfilewriter filewriter = NULL;

  if (fileobj == NULL) {
    ZF_LOGE("`fileobj` is NULL");
    return NULL;
  }

  if ((filewriter = csvfilewriter_init()) == NULL) {
    ZF_LOGE("Could not allocate `csvfilewriter`");
    return NULL;
  }

  filewriter->file = fileobj;
  return filewriter;
}

/**
 * @brief Close CSV File Writer struct
 *
 * Implements @c csvstream_close callback interface
 *
 * @param[in] streamdata  pointer to @c struct @c csv_file_writer to be closed
 *
 * @see csvstream_close
 */
void csvfilewriter_filepath_closer(csvstream_type streamdata) {
  ZF_LOGD("Closing CSV File Writer from filepath");

  if (streamdata == NULL) {
    ZF_LOGE("`streamdata` was NULL, cannot close any resources");
    return;
  }

  csvfilewriter filewriter = (csvfilewriter)streamdata;
  if (filewriter->file != NULL) {
    ZF_LOGD("Closing `file` attribute");
    fclose(filewriter->file);
  }

  ZF_LOGI("Freeing CSV File Writer from filepath resources");
  free(filewriter);
}

/**
 * @brief CSV File Writer closer for file pointer
 *
 * @param streamdata [description]
 */
void csvfilewriter_file_closer(csvstream_type streamdata) {
  ZF_LOGD("Closing CSV File Writer from file pointer");
  if (streamdata == NULL) {
    ZF_LOGE("`streamdata` was NULL, cannot close any resources");
    return;
  }

  csvfilewriter filewriter = (csvfilewriter)streamdata;

  ZF_LOGI("Freeing CSV File Writer from file pointer resources");
  free(filewriter);
}

/**
 * sets provided record as active
 *
 * @see csvstream_setrecord
 */
void csvwriter_setrecord(csvstream_type streamdata,
                         const char **  record,
                         size_t         length) {
  ZF_LOGD("Setting next record for CSV Writer - length %lu",
          (long unsigned)length);

  if (streamdata == NULL) {
    ZF_LOGE("`streamdata` is NULL");
    return;
  } else if (record == NULL) {
    ZF_LOGE("`record` is NULL");
    return;
  } else if (length == 0) {
    ZF_LOGE("`length` is 0");
    return;
  }

  csvfilewriter filewriter = (csvfilewriter)streamdata;
  filewriter->record       = (char **)record;
  filewriter->capacity_r   = length;
  filewriter->position_r   = 0;
}

/**
 * TODO: size_t (*csvstream_setnextfield)(csvstream_type streamdata);
 */
CSV_STREAM_SIGNAL csvwriter_setnextfield(csvstream_type streamdata,
                                         size_t *       length) {
  ZF_LOGD("Setting next field for CSV Writer");

  if (streamdata == NULL) {
    ZF_LOGD("`streamdata` is NULL -- exiting early");
    return CSV_ERROR;
  }

  csvfilewriter filewriter = (csvfilewriter)streamdata;

  if (filewriter->position_r >= filewriter->capacity_r) {
    ZF_LOGD("Reached end of record");
    return CSV_EOR;
  }

  size_t next_index = filewriter->position_r;
  char **record, *field;

  filewriter->position_r++;
  record = (char **)(filewriter->record);
  field  = record[next_index];
  ZF_LOGD("Field # %lu Value: %s", (long unsigned)next_index, field);

  filewriter->field      = field;
  filewriter->capacity_f = strlen(field) + 1;
  filewriter->position_f = 0;

  *length = filewriter->capacity_f;

  return CSV_GOOD;
}

/**
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

  filewriter->capacity_f = strlen((char *)filewriter->field) + 1;
  filewriter->position_f = 0;
}

/**
 * TODO: CSV_STREAM_SIGNAL (*csvstream_getnextchar)(csvstream_type
                                                    streamdata,
                                                    csv_comparison_char_type
 **value);
 */
CSV_STREAM_SIGNAL csvwriter_getnextchar(csvstream_type            streamdata,
                                        csv_comparison_char_type *value) {
  ZF_LOGD("Getting next character from current active input field");

  if (streamdata == NULL) {
    ZF_LOGD("`streamdata` is NULL -- exiting early");
    return CSV_ERROR;
  }
  csvfilewriter filewriter = (csvfilewriter)streamdata;

  if ((filewriter->position_f + 1) >= filewriter->capacity_f) {
    ZF_LOGD("Reached end of field");
    return CSV_END_OF_FIELD;
  }

  char *field = (char *)filewriter->field;
  *value      = field[filewriter->position_f++];
  ZF_LOGD("Retrieved next character: `%c`", (char)(*value));

  return CSV_GOOD;
}

/**
 * TODO: void (*csvstream_writechar)(csvstream_type           streamdata,
                                     CSV_CHAR_TYPE            char_type,
                                     csv_comparison_char_type value);
 */
void csvwriter_writechar(csvstream_type           streamdata,
                         csv_comparison_char_type value) {
  ZF_LOGD("Writing next character to output stream");

  if (streamdata == NULL) {
    ZF_LOGD("`streamdata` is NULL -- exiting early");
    return;
  }

  csvfilewriter filewriter = (csvfilewriter)streamdata;
  int           rc         = 0;

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
    fprintf(stderr,
            "putc(`%c`, filewriter->file) failed. Raw value: `%zd`",
            (char)value,
            value);
    exit(EXIT_FAILURE);
  }
}

/**
 * @endcond
 */
