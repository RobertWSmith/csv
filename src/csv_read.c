/**
 * @cond INTERNAL
 *
 * @file csv_read.c
 * @author Robert W. Smith
 * @date 2018-06-27
 * @brief Implementation of CSV Input parser
 *
 * Private documentation, API subject to change. Parts of this file are useful
 * as example implementations of the callback API.
 *
 * Significantly inspired by Python's '_csv.c' implementation.
 *
 * https://github.com/python/cpython/blob/master/Modules/_csv.c
 *
 * @see csv/definitions.h
 * @see csv/dialect.h
 * @see csv/stream.h
 * @see csv/read.h
 * @see csv/version.h
 */

#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csv.h"
#include "dialect_private.h"

/**
 * @brief Flags to store the parser state
 *
 * These flags help determine how the next character in the stream will be
 * evaluated.
 */
typedef enum CSV_READER_PARSER_STATE {
  START_RECORD,    /**< Indicates that the parser is at the beginning of a
                      new record, current field and record buffer
                      positions should both be zero. */
  START_FIELD,     /**< Indicates the parser is at the beginning of a new
                      field, the field buffer position should be at zero
                    */
  ESCAPED_CHAR,    /**< Indicates the prior character indicates the next
                      character should be treated as though it has been
                      escaped */
  IN_FIELD,        /**< Indicates the parser is inside of a field, and is
                      watching for a delimiter to find the end of the
                      field */
  IN_QUOTED_FIELD, /**< Indicates the parser is in a quoted field, and is
                      watching for another quote character before the
                      delimiter */
  ESCAPE_IN_QUOTED_FIELD, /**< Indicates that the field is quoted and the prior
                             character was an escape */
  QUOTE_IN_QUOTED_FIELD,  /**< Indicates a quote character was encountered in a
                             quoted field, determines whether to consider this
                             as a field character or a special character for
                             parsing the CSV */
  EAT_CRNL,           /**< Indicates the parser should disregard any Carriage
                         Returns or New Line characters, generally if
                         they're encountered at the end of a line */
  AFTER_ESCAPED_CRNL, /**< Indicates that the parser is immediately after a
                         Carriage Return or New Line which should be
                         considered part of the text of a field */
} CSV_READER_PARSER_STATE;

/**
 * @brief convert CSV Reader Parser State enum value to string
 *
 * intended to use for logging
 *
 * @param[in] state CSV Reader Parser State
 *
 * @return string representation of CSV Reader Parser State enumeration value
 *
 * @see CSV_READER_PARSER_STATE
 */
const char *csv_reader_parser_state(CSV_READER_PARSER_STATE state) {
  switch (state) {
    case START_RECORD: return "START_RECORD";
    case START_FIELD: return "START_FIELD";
    case ESCAPED_CHAR: return "ESCAPED_CHAR";
    case IN_FIELD: return "IN_FIELD";
    case IN_QUOTED_FIELD: return "IN_QUOTED_FIELD";
    case ESCAPE_IN_QUOTED_FIELD: return "ESCAPE_IN_QUOTED_FIELD";
    case QUOTE_IN_QUOTED_FIELD: return "QUOTE_IN_QUOTED_FIELD";
    case EAT_CRNL: return "EAT_CRNL";
    case AFTER_ESCAPED_CRNL: return "AFTER_ESCAPED_CRNL";
  }
}

/**
 * @brief Implementation of the CSV Reader.
 *
 * With respect to the @p streamdata field, this is supplied as an opaque
 * pointer to the constructor, and is passed as an opaque pointer to the
 * functions defined by @p getnextchar, @p appendchar, @p savefield,
 * @p saverecord, and the optional @p closer callbacks. This structure should be
 * implemented in a way that it contains any information it needs to interact
 * with the input stream,  and a buffer for the current field, another buffer
 * for the current record, and likely a way to track the current length and
 * capacity of each of the field and record buffers.
 *
 * @see csvreader
 * @see csvdialect
 * @see csvreader_init
 * @see csvreader_advanced_init
 * @see csvdialect_init
 * @see csv/read.h
 * @see csv/dialect.h
 */
struct csv_reader {
  csvdialect dialect; /**< CSV Dialect, which is the configuration type for the
                         CSV parser */
  csvstream_type streamdata; /**< Arbitrary pointer to a user-defined struct.
                                See description above */
  csvstream_getnextchar getnextchar; /**< Callback which supplies the next
                                        character in the stream */
  csvstream_appendfield appendchar;  /**< Callback which appends the supplied
                      character to the end  of the current field buffer */
  csvstream_savefield savefield; /**< Callback which finalizes the current field
                    and appends the string to the end of the record array */
  csvstream_saverecord saverecord; /**< Callback which finalizes the record and
                                      prepares it to return to the caller */
  csvstream_close closer; /**< Optional callback which releases the resources
                             held by @p streamdata */
  CSV_READER_PARSER_STATE parser_state; /**< Holds the parser state, which
                                           controls the parser algorithm */
};

/**
 * @brief CSV File Reader
 *
 * Private struct pointer which is used as a @c streamdata input, maintains
 * data from @c stdio based streams.
 */
typedef struct csv_file_reader *csvfilereader;

/**
 * @brief Initializes the private struct used to read from @c stdio streams
 *
 * This function initializes the @c csvfilereader state to NULL on the stream
 * and filepath values, and a default allocated field buffer and record buffer.
 *
 * @return Default initialized @c csvfilereader
 */
csvfilereader csvfilereader_init(void);

/**
 * @brief Initializes the private struct used to read from a filepath
 *
 * This method uses @p filepath as a parameter when calling @c fopen. This
 * method also sets a @c closer callback which ensures that when @c
 * csvreader_close is called the stream and resources are appropriately closed.
 * This initializer works with standard C @c char types.
 *
 * @param[in] filepath  path to the input file
 *
 * @return              Fully initialized @c csvfilereader, if NULL is returned
 *                      then an IO error was encountered.
 *
 * @see csvfilereader_init
 */
csvfilereader csv_filepath_open(char const *filepath);

/**
 * @brief Initializes the private struct used to read from a @c FILE*
 *
 * This method uses @p fileobj as a parameter and does not call @c fopen. This
 * method also sets a @c closer callback which ensures that when @c
 * csvreader_close is called the stream is not closed, but the buffers created
 * are closed. This initializer works with standard C @c char types.
 *
 * @param[in] fileobj   path to the input file
 *
 * @return              Fully initialized @c csvfilereader, if NULL is returned
 *                      then an IO error was encountered.
 *
 * @see csvfilereader_init
 */
csvfilereader csv_file_open(FILE *fileobj);

/**
 * @brief Get next character in the input stream
 *
 * Callback conforming to the @c csvstream_getnextchar definition
 *
 * This callback was designed for the @c csvfilereader as the @p streamdata
 * value. This reads a single @c char from the IO buffer and returns it as the
 * value. The return value is defined by the API to allow validation of the
 * proper downcast size of @p value.
 *
 * @param[in,out] streamdata  opaque pointer to @c csvfilereader object
 * @param[out]    value       next character in the stream, upcast to
 *                            @c csv_comparison_char_type if neccessary
 *
 * @return                    enum constant which declares the proper downcast
 *                            size of @p value
 *
 * @see csv/stream.h
 */
CSV_STREAM_SIGNAL csv_file_getnextchar(csvstream_type            streamdata,
                                       csv_comparison_char_type *value);

/**
 * @brief Append a character to the end of the current field buffer
 *
 * Callback conforming to the @c csvstream_appendfield definition
 *
 * Called when a character which is not used for parsing is encountered, this
 * character is appended to the end of the current field buffer.
 *
 * @param[in,out] streamdata  opaque pointer to @c csvfilereader object
 * @param[in]     value       the character to append
 * ... might need to add input for CSV_STREAM_SIGNAL
 *
 * @see csv/stream.h
 */
void csv_file_appendchar(csvstream_type           streamdata,
                         csv_comparison_char_type value);

/**
 * @brief Save field as next value in current record buffer
 *
 * Callback conforming to the @c csvstream_savefield definition
 *
 * When this function is called, the internal field buffer data is used to
 * create a record buffer field of appropriate size. The record buffer's size
 * tracker increments by one, and the field buffer's position is set back to
 * zero.
 *
 * @param[in,out] streamdata  opaque pointer to @c csvfilereader object
 *
 * @see csv/stream.h
 */
void csv_file_savefield(csvstream_type streamdata);

/**
 * @brief Save record in preparation of returning it to the caller
 *
 * Callback conforming to the @c csvstream_saverecord definition
 *
 * This function prepares the current record buffer to return as a opaque
 * pointer to a void** with length elements. The return value informs the
 * base type of the @p fields
 *
 * @param[in,out] streamdata  opaque pointer to @c csvfilereader object
 * @param[out]    fields      reference to a appropriate pointer, the type can
 *                            either be documented by the API (@c char in this
 *                            case) or the caller can use the return value to
 *                            cast to the correct type.
 * @param[out]    length      the number of fields stored in @p fields
 *
 * @see csv/stream.h
 */
void csv_file_saverecord(csvstream_type streamdata,
                         char ***       fields,
                         size_t *       length);

/**
 * @brief Release resources for CSV readers initialized with a filepath
 *
 * Closes the @c FILE* opened with the filepath and releases the allocated
 * buffers.
 *
 * @param[in,out] streamdata  opaque pointer to @c csvfilereader object
 *
 * @see csv/stream.h
 */
void csv_read_filepath_close(csvstream_type streamdata);

/**
 * @brief Release resources for CSV readers initialized with a @c FILE*
 *
 * Frees the allocated buffers, does not release the file stream
 *
 * @param[in,out] streamdata  opaque pointer to @c csvfilereader object
 *
 * @see csv/stream.h
 */
void csv_read_file_close(csvstream_type streamdata);

/**
 * @brief Default initializer for the CSV Reader
 *
 * Initializes all fields to @c NULL, except the @p dialect. If @p dialect is
 * @c NULL then @c csvdialect_init is called and the default dialect is used in
 * its place.
 *
 * @param[in] dialect CSV Dialect configuration type
 *
 * @see csvreader_init
 * @see csvreader_advanced_init
 */
csvreader _csvreader_init(csvdialect dialect);

/**
 * @brief Determine what should be done with the next character in the stream
 *
 * This function takes into account the current state of the parser and then
 * uses the supplied character to determine if it should be added to the current
 * field, if it indicates a field boundary has been determined, a record
 * boundary or the end of the stream.
 */
void parse_value(csvreader reader, csv_comparison_char_type value);

/*
 * end of private forward declarations
 */

/*
 * API implementations
 */
csvreader csvreader_init(csvdialect dialect, const char *filepath) {
  ZF_LOGI("Initiailizing CSV Reader from filepath `%s`", filepath);
  ZF_LOGD("dialect: `%p`", (void *)dialect);

  csvreader     reader     = NULL;
  csvfilereader filereader = NULL;

  if ((filereader = csv_filepath_open(filepath)) == NULL) {
    ZF_LOGE("`csvfilereader` could not be allocated");
    return NULL;
  }

  /* broke out of if statement because I find that visually difficult to read */
  reader = csvreader_advanced_init(dialect,
                                   &csv_file_getnextchar,
                                   &csv_file_appendchar,
                                   &csv_file_savefield,
                                   &csv_file_saverecord,
                                   (csvstream_type)filereader);

  reader = csvreader_set_closer(reader, &csv_read_filepath_close);

  /* final validation */
  if (reader == NULL) {
    ZF_LOGE("`csvreader` could not be allocated");
    csv_read_filepath_close((csvstream_type)filereader);
    return NULL;
  }

  ZF_LOGD("`csvreader` successfully allocated `%p`", (void *)reader);
  return reader;
}

csvreader csvreader_file_init(csvdialect dialect, FILE *fileobj) {
  ZF_LOGI("Initiailizing CSV Reader from stdio file pointer");
  ZF_LOGD("dialect: `%p`", (void *)dialect);
  ZF_LOGD("fileobj: `%p`", (void *)fileobj);

  csvreader     reader     = NULL;
  csvfilereader filereader = NULL;

  if ((filereader = csv_file_open(fileobj)) == NULL) {
    ZF_LOGE("`csvfilereader` could not be allocated");
    return NULL;
  }

  /* broke out of if statement because I find that visually difficult to read */
  reader = csvreader_advanced_init(dialect,
                                   &csv_file_getnextchar,
                                   &csv_file_appendchar,
                                   &csv_file_savefield,
                                   &csv_file_saverecord,
                                   (csvstream_type)filereader);

  reader = csvreader_set_closer(reader, &csv_read_file_close);

  /* final validation */
  if (reader == NULL) {
    ZF_LOGE("`csvreader` could not be allocated");
    csv_read_file_close((csvstream_type)filereader);
    return NULL;
  }

  ZF_LOGD("`csvreader` successfully allocated `%p`", (void *)reader);
  return reader;
}

csvreader csvreader_advanced_init(csvdialect            dialect,
                                  csvstream_getnextchar getnextchar,
                                  csvstream_appendfield appendchar,
                                  csvstream_savefield   savefield,
                                  csvstream_saverecord  saverecord,
                                  csvstream_type        streamdata) {
  ZF_LOGI("CSV Reader Advanced Initializer called");
  ZF_LOGD("dialect:     `%p`", (void *)dialect);
  ZF_LOGD("streamdata:  `%p`", (void *)streamdata);

  /* validation step */
  /* other than dialect, all arguments must be non-null */
  if ((getnextchar == NULL) || (appendchar == NULL) || (savefield == NULL) ||
      (saverecord == NULL) || (streamdata == NULL)) {
    ZF_LOGE("At least one required advanced initializer argument is NULL");
    ZF_LOGD("NULL getnextchar: `%s`",
            (getnextchar == NULL) ? "NULL" : "NOT NULL");
    ZF_LOGD("NULL appendchar:  `%s`",
            (appendchar == NULL) ? "NULL" : "NOT NULL");
    ZF_LOGD("NULL savefield:   `%s`",
            (savefield == NULL) ? "NULL" : "NOT NULL");
    ZF_LOGD("NULL saverecord:  `%s`",
            (saverecord == NULL) ? "NULL" : "NOT NULL");
    ZF_LOGD("NULL streamdata:  `%s`",
            (streamdata == NULL) ? "NULL" : "NOT NULL");
    return NULL;
  }

  csvreader reader = _csvreader_init(dialect);

  reader->streamdata  = streamdata;
  reader->getnextchar = getnextchar;
  reader->appendchar  = appendchar;
  reader->savefield   = savefield;
  reader->saverecord  = saverecord;

  return reader;
}

csvreader csvreader_set_closer(csvreader reader, csvstream_close closer) {
  if (reader == NULL) {
    ZF_LOGE("`called with NULL `reader`");
    return NULL;
  }

  reader->closer = closer;
  return reader;
}

void csvreader_close(csvreader *reader) {
  ZF_LOGI("called reader: `%p`", (void *)(*reader));

  if ((*reader) == NULL) {
    ZF_LOGI("`reader` referenced a NULL pointer, exiting function early");
    return;
  }

  csvdialect_close(&((*reader)->dialect));

  if (((*reader)->closer) != NULL) {
    ZF_LOGI(
        "Calling the `csvstream_close` function supplied to close the stream "
        "data");
    (*((*reader)->closer))((*reader)->streamdata);
  }

  ZF_LOGD("Freeing the `csvreader`");
  free((*reader));
  *reader = NULL;
}

csvreturn csvreader_next_record(csvreader reader,
                                char ***  record,
                                size_t *  record_length) {
  ZF_LOGI("called reader: `%p`", (void *)reader);
  csv_comparison_char_type value  = 0;
  CSV_STREAM_SIGNAL        signal = CSV_GOOD;
  csvreturn                rc;

  ZF_LOGD("Beginning `getnextchar` loop");

  /* burn through any chars that exist at the beginning of the record which
     don't add to a field */
  while (reader->parser_state == START_RECORD) {
    signal = (*reader->getnextchar)(reader->streamdata, &value);
    ZF_LOGD(
        "signal returned: `%d`, character returned: `%c`", signal, (char)value);

    if (value == '\0') {
      ZF_LOGI("line contains NULL byte");
      rc          = csvreturn_init(false);
      rc.io_error = 1;
      return rc;
    }

    if (signal == CSV_GOOD) {
      ZF_LOGD("Signal indicates good value returned, beginning to parse");
      parse_value(reader, value);
    } else {
      ZF_LOGD("Signal indicates EOF or Error, ending loop");
      break;
    }
  }

  /* as long as the input is in a good state, extract chars until we hit the
     next start of record */
  if (signal == CSV_GOOD) {
    do {
      signal = (*reader->getnextchar)(reader->streamdata, &value);
      ZF_LOGD("signal returned: `%d`, character returned: `%c`",
              signal,
              (char)value);

      if (value == '\0') {
        ZF_LOGE("line contains NULL byte");
        rc          = csvreturn_init(false);
        rc.io_error = 1;
        return rc;
      }

      if (signal == CSV_GOOD) {
        ZF_LOGD("Signal indicates good value returned, beginning to parse");
        parse_value(reader, value);
      } else {
        ZF_LOGD("Signal indicates EOF or Error, ending loop");
        break;
      }
    } while (reader->parser_state != START_RECORD);
  }

  // size_t len = 0;
  // &len
  (*reader->saverecord)(reader->streamdata, record, record_length);
  // *record_length = len;

  if (signal == CSV_EOF) {
    ZF_LOGI("CSV Reader found EOF reached");
    rc        = csvreturn_init(true);
    rc.io_eof = 1;
    return rc;
  } else if (signal != CSV_GOOD) {
    ZF_LOGI("CSV Reader found IO error state encountered");
    rc          = csvreturn_init(false);
    rc.io_error = 1;
    return rc;
  }
  ZF_LOGI("CSV Reader end of record, IO state is good");
  return csvreturn_init(true);
}

/*
 * end of API implementations
 */

/*
 * Begin - FILE* based callback implementations
 */

/*
 * private implementation struct to manage CSVs which utilize stdio files
 */
struct csv_file_reader {
  /* if providex, might be null but useful for debug info */
  char const *filepath;

  /* needed for the input stream */
  FILE *file;

  char **record;
  char * field;
  size_t capacity_f;
  size_t size_f;
  size_t capacity_r;
  size_t size_r;
};

/*
 * core struct csv_file_reader * initializer for standardized creation between
 * both the char* filepath initializer and the FILE* initializer
 */
csvfilereader csvfilereader_init(void) {
  ZF_LOGI("`csvfilereader_init` called");

  csvfilereader fr = NULL;

  if ((fr = malloc(sizeof *fr)) == NULL) {
    ZF_LOGD("`csvfilereader` could not be allocated");
    return NULL;
  }

  fr->filepath = NULL;
  fr->file     = NULL;

  /* 256 chosen as a default because this is generally the max
   * witdth of a SQL database VARCHAR field.
   */
  fr->size_f     = 0;
  fr->capacity_f = 256;

  if ((fr->field = malloc(sizeof *fr->field * fr->capacity_f)) == NULL) {
    ZF_LOGD(
        "`csvfilereader->field` could not be allocated with a size of `%lu`",
        (long unsigned)fr->capacity_f);
    free(fr);
    return NULL;
  }

  /* 8 is arbitrary, subject to redesign on performance analysis */
  fr->size_r     = 0;
  fr->capacity_r = 8;

  if ((fr->record = malloc(sizeof *fr->record * fr->capacity_r)) == NULL) {
    ZF_LOGD(
        "`csvfilereader->record` could not be allocated with a size of `%lu`",
        (long unsigned)fr->capacity_r);
    free(fr->field);
    free(fr);
    return NULL;
  }
  ZF_LOGD("`csvfilereader` successfully allocated at `%p`", (void *)fr);
  return fr;
}

/*
 * make a file reader from a filepath, shares common logic and implementation
 * with the FILE* initializer.
 */
csvfilereader csv_filepath_open(char const *filepath) {
  ZF_LOGI("`csv_filepath_open` called with filepath: `%s`", filepath);

  if (filepath == NULL) {
    ZF_LOGD("`csvfilereader` filepath cannot be a NULL string, returning NULL");
    return NULL;
  }

  csvfilereader fr      = NULL;
  FILE *        fileobj = NULL;

  if ((fileobj = fopen(filepath, "rb")) == NULL) {
    ZF_LOGD(
        "`csvfilereader` Filepath could not be opened with a call to `fopen`");
    return NULL;
  }

  if ((fr = csvfilereader_init()) == NULL) {
    ZF_LOGD("`csvfilereader` could not be allocated");
    fclose(fileobj);
    return NULL;
  }

  fr->filepath = filepath;
  fr->file     = fileobj;
  ZF_LOGD("`csvfilereader` successfully allocated");
  return fr;
}

/*
 * make a file reader from a supplied FILE*
 */
csvfilereader csv_file_open(FILE *fileobj) {
  ZF_LOGI("`csv_file_open` called with `fileobj`: `%p`", (void *)fileobj);

  if (fileobj == NULL) {
    ZF_LOGD("`csvfilereader` `fileobj` cannot be NULL");
    return NULL;
  }

  csvfilereader fr = NULL;

  if ((fr = csvfilereader_init()) == NULL) {
    ZF_LOGD("`csvfilereader` could not be allocated");
    return NULL;
  }

  fr->filepath = NULL;
  fr->file     = fileobj;
  ZF_LOGD("`csvfilereader` successfully allocated");
  return fr;
}

CSV_STREAM_SIGNAL csv_file_getnextchar(csvstream_type            streamdata,
                                       csv_comparison_char_type *value) {
  ZF_LOGI("called w/ streamdata: `%p`", streamdata);
  int c = 0;

  if (streamdata == NULL) {
    *value = CSV_UNDEFINED_CHAR;
    ZF_LOGD(
        "`csvstream_type` provided was NULL, and must point to a valid memory "
        "address");
    return CSV_ERROR;
  }

  csvfilereader fr = (csvfilereader)streamdata;

  if (fr->file == NULL) {
    *value = CSV_UNDEFINED_CHAR;
    ZF_LOGD("`streamdata->file` provided was NULL -- exiting with error");
    return CSV_ERROR;
  }

  if ((c = getc(fr->file)) != EOF) {
    *value = c;
    ZF_LOGD("value: `%c` CSV_STREAM_SIGNAL: `CSV_GOOD`", (char)(*value));
    return CSV_GOOD;
  }

  if (feof(fr->file)) {
    ZF_LOGD("End of file indicator encountered");
    *value = 0;
    ZF_LOGD("value: `%c` CSV_STREAM_SIGNAL: `CSV_EOF`", (char)(*value));
    return CSV_EOF;
  }

  if (ferror(fr->file)) {
    ZF_LOGI("IO Error Encountered");
    *value = CSV_UNDEFINED_CHAR;
    perror("Error detected while reading CSV");
    ZF_LOGD("value: `%c` CSV_STREAM_SIGNAL: `CSV_ERROR`", (char)(*value));
    return CSV_ERROR;
  }

  return CSV_GOOD;
}

void csv_file_appendchar(csvstream_type           streamdata,
                         csv_comparison_char_type value) {
  ZF_LOGI("`csv_file_appendchar` called with value argument `%c`", (char)value);

  if (streamdata == NULL) {
    ZF_LOGD("`csvstream_type` provided was NULL, bad value");
    return;
  }

  csvfilereader fr = (csvfilereader)streamdata;

  /* expand field, if neccessary. hopefully efficiently, needs validation */
  if ((fr->size_f + 1) >= fr->capacity_f) {
    ZF_LOGD(
        "`csvfilereader` field size required exceeds capacity, calling "
        "`realloc` to expand");

    if (fr->capacity_f > 4096) {
      ZF_LOGD("`csvfilereader` field is greater than 4KiB, expanding by 1KiB");
      fr->capacity_f += 1024;
    } else {
      ZF_LOGD(
          "`csvfilereader` field is less than 4KiB, doubling current "
          "capacity");
      fr->capacity_f *= 2;
    }
    fr->field = realloc(fr->field, (fr->capacity_f + 1));
    ZF_LOGI("`csvfilereader` field reallocated to new size of: `%lu`",
            (long unsigned)fr->capacity_f);

    memset((fr->field + fr->size_f), 0, ((fr->capacity_f + 1) - fr->size_f));
  }

  fr->field[fr->size_f++] = (char)value;
  ZF_LOGD("Appending character to field: `%c` at position: `%lu`",
          (char)value,
          (long unsigned)fr->size_f);
}

void csv_file_savefield(csvstream_type streamdata) {
  ZF_LOGI("`csv_file_savefield` called");

  if (streamdata == NULL) {
    ZF_LOGD("`csvstream_type` provided was NULL, bad value");
    return;
  }

  csvfilereader fr   = (csvfilereader)streamdata;
  char *        temp = NULL;

  /* grow field, if neccessary */
  if ((fr->size_r + 1) >= fr->capacity_r) {
    ZF_LOGD(
        "`csvfilereader` record size required exceeds capacity, calling "
        "`realloc` to expand");

    if (fr->capacity_r > 128) {
      ZF_LOGD(
          "`csvfilereader` record capacity greater than 128, expanding by "
          "128");
      fr->capacity_r += 128;
    } else {
      ZF_LOGD(
          "`csvfilereader` record capacity less than 128, doubling capacity");
      fr->capacity_r *= 2;
    }
    fr->record = realloc(fr->record, fr->capacity_r);
    ZF_LOGI("`csvfilereader` record reallocated to new size of: `%lu`",
            (long unsigned)fr->capacity_r);
  }

  if ((temp = calloc(fr->size_f + 1, sizeof *fr->field)) == NULL) {
    ZF_LOGD("`csvfilereader` record field could not be allocated");
    return;
  }

  /* copy up to the size of the current field */
  ZF_LOGD("Beginning to copy field to temp");
  memcpy(temp, fr->field, fr->size_f);
  ZF_LOGD("Completed copying field to temp, value: `%s`", temp);
  fr->record[fr->size_r] = temp;
  fr->size_r += 1;

  /* set field back to the beginning of the field */
  fr->size_f = 0;
  memset(fr->field, 0, fr->capacity_f);
  ZF_LOGD("Reset field buffer to filled with \'\\0\' characters");
}

void csv_file_saverecord(csvstream_type streamdata,
                         char ***       fields,
                         size_t *       length) {
  ZF_LOGI("`csv_file_saverecord` called");

  if (streamdata == NULL) {
    ZF_LOGD("`csv_file_saverecord` streamdata is NULL");
    *fields = NULL;
    *length = 0;
    return;
  }

  csvfilereader fr = (csvfilereader)streamdata;

  /* allocate string array to pass the pointer list to caller */
  char **record = NULL;
  *length       = fr->size_r;
  ZF_LOGD("`csv_file_saverecord` record length `%lu`",
          (long unsigned)(*length));

  if ((record = malloc(sizeof *record * fr->size_r)) == NULL) {
    ZF_LOGD("`csv_file_saverecord` record could not be allocated");
    *fields = NULL;
    *length = 0;
    return;
  }

  ZF_LOGD("`csv_file_saverecord` copying records to output");

  for (size_t i = 0; i < fr->size_r; ++i) {
    record[i]     = fr->record[i];
    fr->record[i] = NULL;
    ZF_LOGV("`csv_file_saverecord` field: `%lu` value: `%s`",
            (long unsigned)i,
            record[i]);
  }
  *fields = (char **)record;

  /* reset internal field and record index */
  fr->size_f = 0;
  fr->size_r = 0;
  memset(fr->field, 0, fr->capacity_f);
}

void csv_read_filepath_close(csvstream_type streamdata) {
  ZF_LOGI("streamdata is %s", streamdata == NULL ? "NULL" : "NOT NULL");

  if (streamdata != NULL) {
    csvfilereader fr = (csvfilereader)streamdata;

    /* fr->filepath is allocated externally, not freed here because it could be
     *  a string literal */
    if (fr->file != NULL) {
      ZF_LOGD("file is not null, closing");
      fclose(fr->file);
    }

    if (fr->field != NULL) {
      ZF_LOGD("field is not null, freeing");
      free(fr->field);
    }

    if (fr->record != NULL) {
      ZF_LOGD("record is not null, freeing");

      for (size_t i = 0; i < fr->size_r; ++i) {
        if (fr->record[i] != NULL) {
          ZF_LOGD("csvfilereader->record[%lu] is not null, freeing",
                  (long unsigned)i);
          free(fr->record[i]);
        }
      }
      free(fr->record);
    }

    free(fr);
  }
}

void csv_read_file_close(csvstream_type streamdata) {
  ZF_LOGI("streamdata is %s", streamdata == NULL ? "NULL" : "NOT NULL");

  if (streamdata != NULL) {
    csvfilereader fr = (csvfilereader)streamdata;

    /* fr->filepath is allocated externally, not freed here because it could be
     *  a string literal */

    /* fr->file is allocated externally, therefore not freed here */

    if (fr->field != NULL) {
      ZF_LOGD("field is not null, freeing");
      free(fr->field);
    }

    if (fr->record != NULL) {
      ZF_LOGD("record is not null, freeing");

      for (size_t i = 0; i < fr->size_r; ++i) {
        if (fr->record[i] != NULL) {
          ZF_LOGD("csvfilereader->record[%lu] is not null, freeing",
                  (long unsigned)i);
          free(fr->record[i]);
        }
      }
      free(fr->record);
    }
    free(fr);
  }
}

/*
 * End - FILE* based callback implementations
 */

/*
 * Null initialized csvreader initializer
 */
csvreader _csvreader_init(csvdialect dialect) {
  ZF_LOGI("internal csvreader initializer");
  csvreader reader = NULL;

  if ((reader = malloc(sizeof *reader)) == NULL) {
    ZF_LOGD("csvreader could not be allocated");
    return NULL;
  }

  reader->parser_state = START_RECORD;

  if (dialect == NULL) {
    ZF_LOGD("dialect supplied was NULL, initializing default dialect");
    reader->dialect = csvdialect_init();
  } else {
    ZF_LOGD("dialect supplied was NOT NULL, deep copying dialect");
    reader->dialect = csvdialect_copy(dialect);
  }
  reader->streamdata  = NULL;
  reader->getnextchar = NULL;
  reader->appendchar  = NULL;
  reader->savefield   = NULL;
  reader->saverecord  = NULL;
  reader->closer      = NULL;

  return reader;
}

/*
 * Begin of 'csv/read.h' implementations
 */

/* bool controls 'should continue' (true) or should break switch (false) */
bool parse_start_record(csvreader reader, csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  if (value == '\0') {
    ZF_LOGD("Empty record indicated");

    /* indicates empty record */
    return false;
  } else if ((value == '\n') || (value == '\r')) {
    ZF_LOGD("\\r or \\n encountered at beginning of record, discarding");
    reader->parser_state = EAT_CRNL;
    ZF_LOGD("setting parser state to EAT_CRNL");
    return false;
  }

  /* normal character, handle as start field */
  reader->parser_state = START_FIELD;
  ZF_LOGD("setting parser state to START_FIELD");
  return true;
}

void parse_start_field(csvreader reader, csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  if ((value == '\0') || (value == '\n') || (value == '\r')) {
    ZF_LOGD("encountered boundary character of \\0, \\r or \\n, saving field");
    (*reader->savefield)(reader->streamdata);

    if (value == '\0') {
      reader->parser_state = START_RECORD;
      ZF_LOGD("setting parser state to START_RECORD");
    } else {
      reader->parser_state = EAT_CRNL;
      ZF_LOGD("setting parser state to EAT_CRNL");
    }
  } else if ((value == csvdialect_get_quotechar(reader->dialect)) &&
             (QUOTE_STYLE_NONE == csvdialect_get_quotestyle(reader->dialect))) {
    reader->parser_state = IN_QUOTED_FIELD;
    ZF_LOGD("setting parser state to IN_QUOTED_FIELD");
  } else if (value == csvdialect_get_escapechar(reader->dialect)) {
    reader->parser_state = ESCAPED_CHAR;
    ZF_LOGD("setting parser state to ESCAPED_CHAR");
  } else if ((value == ' ') &&
             csvdialect_get_skipinitialspace(reader->dialect)) {
    // no change
    return;
  } else if (value == csvdialect_get_delimiter(reader->dialect)) {
    /* end of field, so therefore empty/null field */
    ZF_LOGD("encountered delimiter, saving field - no change in parser state");
    (*reader->savefield)(reader->streamdata);
  } else {
    (*reader->appendchar)(reader->streamdata, value);
    reader->parser_state = IN_FIELD;
    ZF_LOGD("appending character to field");
    ZF_LOGD("setting parser state to IN_FIELD");
  }
}

void parse_escaped_char(csvreader reader, csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  if ((value == '\n') || (value == '\r')) {
    (*reader->appendchar)(reader->streamdata, value);
    reader->parser_state = AFTER_ESCAPED_CRNL;
    ZF_LOGD("setting parser state to AFTER_ESCAPED_CRNL");
    return;
  }

  if (value == '\0') value = '\n';

  (*reader->appendchar)(reader->streamdata, value);
  reader->parser_state = IN_FIELD;
  ZF_LOGD("setting parser state to IN_FIELD");
}

void parse_in_field(csvreader reader, csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  /* in unquoted field */
  if ((value == '\n') || (value == '\r') || (value == '\0')) {
    (*reader->savefield)(reader->streamdata);

    if (value == '\0') {
      reader->parser_state = START_RECORD;
      ZF_LOGD("setting parser state to START_RECORD");
    } else {
      reader->parser_state = EAT_CRNL;
      ZF_LOGD("setting parser state to EAT_CRNL");
    }
  } else if (value == csvdialect_get_escapechar(reader->dialect)) {
    reader->parser_state = ESCAPED_CHAR;
    ZF_LOGD("setting parser state to ESCAPED_CHAR");
  } else if (value == csvdialect_get_delimiter(reader->dialect)) {
    (*reader->savefield)(reader->streamdata);
    reader->parser_state = START_FIELD;
    ZF_LOGD("setting parser state to START_FIELD");
  } else {
    (*reader->appendchar)(reader->streamdata, value);
  }
}

void parse_in_quoted_field(csvreader reader, csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  if (value == '\0') { /* no-op */
  } else if (value == csvdialect_get_escapechar(reader->dialect)) {
    reader->parser_state = ESCAPE_IN_QUOTED_FIELD;
    ZF_LOGD("setting parser state to ESCAPE_IN_QUOTED_FIELD");
  } else if ((value == csvdialect_get_quotechar(reader->dialect)) &&
             (QUOTE_STYLE_NONE != csvdialect_get_quotestyle(reader->dialect))) {
    if (csvdialect_get_doublequote(reader->dialect)) {
      reader->parser_state = ESCAPE_IN_QUOTED_FIELD;
      ZF_LOGD("setting parser state to ESCAPE_IN_QUOTED_FIELD");
    } else {
      reader->parser_state = IN_FIELD;
      ZF_LOGD("setting parser state to IN_FIELD");
    }
  } else {
    (*reader->appendchar)(reader->streamdata, value);
  }
}

void parse_quote_in_quoted_field(csvreader                reader,
                                 csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  if ((csvdialect_get_quotestyle(reader->dialect) != QUOTE_STYLE_NONE) &&
      (value == csvdialect_get_quotechar(reader->dialect))) {
    /* save "" as " */
    (*reader->appendchar)(reader->streamdata, value);
    reader->parser_state = IN_QUOTED_FIELD;
    ZF_LOGD("setting parser state to IN_QUOTED_FIELD");
  } else if (value == csvdialect_get_delimiter(reader->dialect)) {
    (*reader->savefield)(reader->streamdata);
    reader->parser_state = START_FIELD;
    ZF_LOGD("setting parser state to START_FIELD");
  } else if ((value == '\0') || (value == '\r') || (value == '\n')) {
    (*reader->savefield)(reader->streamdata);
    reader->parser_state = START_FIELD;
    ZF_LOGD("setting parser state to START_FIELD");
  } else {
    (*reader->savefield)(reader->streamdata);
    reader->parser_state = (value == '\0') ? START_RECORD : EAT_CRNL;

    if (value == '\0') {
      reader->parser_state = START_RECORD;
      ZF_LOGD("setting parser state to START_RECORD");
    } else {
      reader->parser_state = EAT_CRNL;
      ZF_LOGD("setting parser state to EAT_CRNL");
    }
  }
}

void parse_value(csvreader reader, csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  switch (reader->parser_state) {
    case START_RECORD:

      if (!parse_start_record(reader, value)) break;

      /* else, fall through */

    case START_FIELD: parse_start_field(reader, value); break;

    case ESCAPED_CHAR: parse_escaped_char(reader, value); break;

    case AFTER_ESCAPED_CRNL:

      if (value == '\0') break;

      /* else, fallthrough */

    case IN_FIELD: parse_in_field(reader, value); break;

    case IN_QUOTED_FIELD:

      /* in a quoted field */
      parse_in_quoted_field(reader, value);
      break;

    case ESCAPE_IN_QUOTED_FIELD:

      if (value == '\0') value = '\n';
      (*reader->appendchar)(reader->streamdata, value);
      reader->parser_state = IN_QUOTED_FIELD;
      break;

    case QUOTE_IN_QUOTED_FIELD:
      parse_quote_in_quoted_field(reader, value);
      break;

    case EAT_CRNL:

      if (value != '\0') {
        reader->parser_state = START_RECORD;
      }
      break;
  }
}

/**
 * @endcond
 */
