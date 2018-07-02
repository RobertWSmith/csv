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
CSV_STREAM_SIGNAL csvwriter_setnextfield(csvstream_type streamdata);
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
  CSV_STREAM_SIGNAL stream_signal;

  if (writer == NULL) {
    rc = csvreturn_init(false);
    return rc;
  }

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
        stream_signal =
          (*writer->getnextchar)(writer->streamdata, &char_type, &value);

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
      stream_signal =
        (*writer->getnextchar)(writer->streamdata, &char_type, &value);

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
  csvfilewriter output = NULL;

  if ((output = malloc(sizeof *output)) == NULL) {
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
  if (filepath == NULL) {
    return NULL;
  }

  FILE *outfile = NULL;

  if ((outfile = fopen(filepath, "wb")) == NULL) {
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
  csvfilewriter filewriter = (csvfilewriter)streamdata;

  if (filewriter->file != NULL) {
    fclose(filewriter->file);
  }
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
  filewriter->record     = record;
  filewriter->capacity_r = length;
  filewriter->position_r = 0;
}

/*
 * TODO: size_t (*csvstream_setnextfield)(csvstream_type streamdata);
 */
CSV_STREAM_SIGNAL csvwriter_setnextfield(csvstream_type streamdata) {
  if (streamdata == NULL) {
    ZF_LOGD("`streamdata` is NULL -- exiting early");
    return CSV_ERROR;
  }

  csvfilewriter filewriter = (csvfilewriter)streamdata;

  if ((filewriter->position_r + 1) >= filewriter->capacity_r) {
    ZF_LOGD("Reached end of record");
    return CSV_EOR;
  }

  char **record, *field;
  record = (char **)(filewriter->record);
  field  = record[filewriter->position_r++];

  /* default implementation is for stdio `char` returning files */
  filewriter->char_type  = CSV_CHAR;
  filewriter->field      = field;
  filewriter->capacity_f = strlen(field);
  filewriter->position_f = 0;

  return CSV_GOOD;
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
  *value = (csv_comparison_char_type)field[filewriter->position_f++];
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
  if (streamdata == NULL) {
    ZF_LOGD("`streamdata` is NULL -- exiting early");
    return;
  }

  csvfilewriter filewriter = (csvfilewriter)streamdata;
  int rc = 0;

  if (filewriter->file == NULL) {
    ZF_LOGD("`streamdata->file` is NULL -- exiting early");
    return;
  }


  if ((rc = putc((char)value, filewriter->file)) != EOF) {
    return;
  }

  if (ferror(filewriter->file)) {
    perror("putc() error");
    fprintf(stderr, "putc(`%c`, filewriter->file) failed. Raw value: `%lld`", (char)value, value);
    exit(EXIT_FAILURE);
  }
}
