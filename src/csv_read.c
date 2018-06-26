/**
 * Significantly inspired by Python's '_csv.c' implementation.
 *
 * https://github.com/python/cpython/blob/master/Modules/_csv.c
 */

#include <stddef.h>
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
#include "csv/stream.h"
#include "csv/read.h"
#include "dialect_private.h"

typedef enum CSV_READER_PARSER_STATE {
  START_RECORD,
  START_FIELD,
  ESCAPED_CHAR,
  IN_FIELD,
  IN_QUOTED_FIELD,
  ESCAPE_IN_QUOTED_FIELD,
  QUOTE_IN_QUOTED_FIELD,
  EAT_CRNL,
  AFTER_ESCAPED_CRNL,
} CSV_READER_PARSER_STATE;

struct csv_reader {
  csvdialect              dialect;
  csvstream_type          streamdata;
  csvstream_getnextchar   getnextchar;
  csvstream_appendfield   appendchar;
  csvstream_savefield     savefield;
  csvstream_saverecord    saverecord;
  csvstream_close         closer;
  CSV_READER_PARSER_STATE parser_state;
};

/*
 * private implementation forward declarations.
 */
typedef struct csv_file_reader *csvfilereader;

csvfilereader     csvfilereader_init(void);
csvfilereader     csv_filepath_open(char const *filepath);
csvfilereader     csv_file_open(FILE *fileobj);
CSV_STREAM_SIGNAL csv_file_getnextchar(csvstream_type            streamdata,
                                       csv_comparison_char_type *value);
void              csv_file_appendchar(csvstream_type           streamdata,
                                      csv_comparison_char_type value);
void              csv_file_savefield(csvstream_type streamdata);
CSV_CHAR_TYPE     csv_file_saverecord(csvstream_type  streamdata,
                                      csvrecord_type *fields,
                                      size_t         *length);
void              csv_read_filepath_close(csvstream_type streamdata);
void              csv_read_file_close(csvstream_type streamdata);
csvreader         _csvreader_init(csvdialect dialect);
inline void       parse_value(csvreader                reader,
                              csv_comparison_char_type value);

/*
 * end of private forward declarations
 */

/*
 * API implementations
 */
csvreader csvreader_init(csvdialect  dialect,
                         const char *filepath) {
  ZF_LOGI("`csvreader_init` called dialect: `%p`, filepath: `%s`.",
          dialect,
          filepath);
  csvreader reader = NULL;
  csvfilereader fr = NULL;

  if ((fr = csv_filepath_open(filepath)) == NULL) {
    ZF_LOGI("`csvfilereader` could not be allocated.");
    return NULL;
  }

  /* broke out of if statement because I find that visually difficult to read */
  reader = csvreader_advanced_init(dialect,
                                   &csv_file_getnextchar,
                                   &csv_file_appendchar,
                                   &csv_file_savefield,
                                   &csv_file_saverecord,
                                   (csvstream_type)fr);

  if (reader != NULL) {
    reader = csvreader_set_closer(reader, &csv_read_filepath_close);
  }

  /* final validation */
  if (reader == NULL) {
    ZF_LOGI("`csvreader` could not be allocated.");
    csv_read_filepath_close((csvstream_type)fr);
    return NULL;
  }

  ZF_LOGD("`csvreader` successfully allocated at: `%p`.", reader);
  return reader;
}

csvreader csvreader_file_init(csvdialect dialect,
                              FILE      *fileobj) {
  ZF_LOGI("`csvreader_file_init` called dialect: `%p`, fileobj: `%p`.",
          dialect,
          fileobj);
  csvreader reader = NULL;
  csvfilereader fr = NULL;

  if ((fr = csv_file_open(fileobj)) == NULL) {
    ZF_LOGI("`csvfilereader` could not be allocated.");
    return NULL;
  }

  /* broke out of if statement because I find that visually difficult to read */
  reader = csvreader_advanced_init(dialect,
                                   &csv_file_getnextchar,
                                   &csv_file_appendchar,
                                   &csv_file_savefield,
                                   &csv_file_saverecord,
                                   (csvstream_type)fr);

  if (reader != NULL) {
    reader = csvreader_set_closer(reader, &csv_read_file_close);
  }

  /* final validation */
  if (reader == NULL) {
    ZF_LOGI("`csvreader` could not be allocated.");
    csv_read_file_close((csvstream_type)fr);
    return NULL;
  }

  ZF_LOGD("`csvreader` successfully allocated at: `%p`.", reader);
  return reader;
}

csvreader csvreader_advanced_init(csvdialect            dialect,
                                  csvstream_getnextchar getnextchar,
                                  csvstream_appendfield appendchar,
                                  csvstream_savefield   savefield,
                                  csvstream_saverecord  saverecord,
                                  csvstream_type        streamdata) {
  ZF_LOGI("`csvreader_advanced_init` called.");

  /* validation step */
  /* other than dialect, all arguments must be non-null */
  if ((getnextchar == NULL) || (appendchar == NULL) || (savefield == NULL) ||
      (saverecord == NULL) || (streamdata == NULL)) {
    ZF_LOGI(
      "`csvreader_advance_init` failed due to unexpected NULL.\ngetnextchar: `%s`\nappendchar:  `%s`\nsavefield:   `%s`\nsaverecord:  `%s`\nstreamdata:  `%s`\n",
      (getnextchar == NULL) ? "NULL" : "NOT NULL",
      (appendchar == NULL) ? "NULL" : "NOT NULL",
      (savefield == NULL) ? "NULL" : "NOT NULL",
      (saverecord == NULL) ? "NULL" : "NOT NULL",
      (streamdata == NULL) ? "NULL" : "NOT NULL"
      );
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

csvreader csvreader_set_closer(csvreader       reader,
                               csvstream_close closer) {
  if (reader == NULL) {
    ZF_LOGI("`csvreader_set_closer` called with NULL `reader`.");
    return NULL;
  }

  reader->closer = closer;
  ZF_LOGD("`csvreader_set_closer` succeeded.");
  return reader;
}

void csvreader_close(csvreader *reader) {
  /* optimization opportunity after test success */
  ZF_LOGI("`csvreader_close` called.");
  csvreader r = *reader;


  if (r == NULL) {
    ZF_LOGI("`reader` referenced a NULL pointer, exiting function early.");
    return;
  }

  /* optimization opportunity after test success */
  csvdialect d;
  d = r->dialect;
  csvdialect_close(&d);

  /* optimization opportunity after test success */
  csvstream_close closer;
  closer = r->closer;

  if (closer != NULL) {
    ZF_LOGI(
      "Calling the `csvstream_close` function supplied to close the stream data.");
    (*closer)(r->streamdata);
  }

  ZF_LOGD("Freeing the `csvreader`.");
  free(r);
  *reader = NULL;
}

csvreturn csvreader_next_record(csvreader       reader,
                                CSV_CHAR_TYPE  *char_type,
                                csvrecord_type *record,
                                size_t         *record_length) {
  ZF_LOGI("`csvreader_next_record` called.");
  csv_comparison_char_type value = 0;
  CSV_STREAM_SIGNAL signal       = CSV_GOOD;
  csvreturn rc;

  ZF_LOGD("Beginning `getnextchar` loop.");

  /* TODO: look into why this loop is breaking on the first character... */
  do {
    signal = (*reader->getnextchar)(reader->streamdata, &value);
    ZF_LOGD("signal returned: `%d`, character returned: `%c`.",
            signal,
            (char)value);

    if (value == '\0') {
      ZF_LOGI("line contains NULL byte");
      rc          = csvreturn_init(false);
      rc.io_error = 1;
      return rc;
    }

    if (signal == CSV_GOOD) {
      ZF_LOGD("Signal indicates good value returned, beginning to parse.");
      parse_value(reader, value);
    }
    else {
      ZF_LOGD("Signal indicates EOF or Error, ending loop.");
      break;
    }
  } while (reader->parser_state != START_RECORD);

  size_t len = 0;
  *char_type     = (*reader->saverecord)(reader->streamdata, record, &len);
  *record_length = len;

  if (signal == CSV_EOF) {
    ZF_LOGI("CSV Reader found EOF reached.");
    rc        = csvreturn_init(true);
    rc.io_eof = 1;
    return rc;
  }
  else if (signal != CSV_GOOD) {
    ZF_LOGI("CSV Reader found IO error state encountered.");
    rc          = csvreturn_init(false);
    rc.io_error = 1;
    return rc;
  }
  ZF_LOGI("CSV Reader end of record, IO state is good.");
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
  char  *field;
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
  ZF_LOGI("`csvfilereader_init` called.");

  csvfilereader fr = NULL;

  if ((fr = malloc(sizeof *fr)) == NULL) {
    ZF_LOGD("`csvfilereader` could not be allocated.");
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
    ZF_LOGD("`csvfilereader->field` could not be allocated with a size of `%lu`.",
            (long unsigned int)fr->capacity_f);
    free(fr);
    return NULL;
  }

  /* 8 is arbitrary, subject to redesign on performance analysis */
  fr->size_r     = 0;
  fr->capacity_r = 8;

  if ((fr->record = malloc(sizeof *fr->record * fr->capacity_r)) == NULL) {
    ZF_LOGD(
      "`csvfilereader->record` could not be allocated with a size of `%lu`.",
      (long unsigned int)fr->capacity_r);
    free(fr->field);
    free(fr);
    return NULL;
  }
  ZF_LOGD("`csvfilereader` successfully allocated at `%p`.", fr);
  return fr;
}

/*
 * make a file reader from a filepath, shares common logic and implementation
 * with the FILE* initializer.
 */
csvfilereader csv_filepath_open(char const *filepath) {
  ZF_LOGI("`csv_filepath_open` called with filepath: `%s`.", filepath);

  if (filepath == NULL) {
    ZF_LOGD("`csvfilereader` filepath cannot be a NULL string, returning NULL.");
    return NULL;
  }

  csvfilereader fr = NULL;
  FILE *fileobj    = NULL;

  if ((fileobj = fopen(filepath, "rb")) == NULL) {
    ZF_LOGD("`csvfilereader` Filepath could not be opened with a call to `fopen`.");
    return NULL;
  }

  if ((fr = csvfilereader_init()) == NULL) {
    ZF_LOGD("`csvfilereader` could not be allocated.");
    fclose(fileobj);
    return NULL;
  }

  fr->filepath = filepath;
  fr->file     = fileobj;
  ZF_LOGD("`csvfilereader` successfully allocated.");
  return fr;
}

/*
 * make a file reader from a supplied FILE*
 */
csvfilereader csv_file_open(FILE *fileobj) {
  ZF_LOGI("`csv_file_open` called with `fileobj`: `%p`.", fileobj);

  if (fileobj == NULL) {
    ZF_LOGD("`csvfilereader` `fileobj` cannot be NULL.");
    return NULL;
  }

  csvfilereader fr = NULL;

  if ((fr = csvfilereader_init()) == NULL) {
    ZF_LOGD("`csvfilereader` could not be allocated.");
    return NULL;
  }

  fr->filepath = NULL;
  fr->file     = fileobj;
  ZF_LOGD("`csvfilereader` successfully allocated.");
  return fr;
}

CSV_STREAM_SIGNAL csv_file_getnextchar(csvstream_type            streamdata,
                                       csv_comparison_char_type *value) {
  ZF_LOGI("`csv_file_getnextchar` called.");
  int c = 0;

  if (streamdata == NULL) {
    ZF_LOGD(
      "`csvstream_type` provided was NULL, and must point to a valid memory address.");
    *value = CSV_UNDEFINED_CHAR;
    return CSV_ERROR;
  }

  csvfilereader fr = (csvfilereader)streamdata;

  if (fr->file == NULL) {
    ZF_LOGD(
      "`csvstream_type` `file` provided was NULL, and must point to a valid memory address for a `FILE*`.");
    *value = 0;
    return CSV_ERROR;
  }

  ZF_LOGD("`csv_file_getnextchar` prior to calling `fgetc(fr->file)`.");

  if ((c = fgetc(fr->file)) != EOF) {
    *value = (csv_comparison_char_type)c;
    ZF_LOGD("`csv_file_getnextchar` comlpeted with `%c`.", (char)(*value));
    return CSV_GOOD;
  }
  ZF_LOGD("`csv_file_getnextchar` after calling `fgetc(fr->file)`.");

  if (feof(fr->file)) {
    ZF_LOGD("End of file indicator encountered.");
    *value = 0;
    return CSV_EOF;
  }

  if (ferror(fr->file)) {
    ZF_LOGD("IO Error Encountered.");
    perror("Error detected while reading CSV.");
    *value = CSV_UNDEFINED_CHAR;
    return CSV_ERROR;
  }
  return CSV_GOOD;
}

void csv_file_appendchar(csvstream_type           streamdata,
                         csv_comparison_char_type value) {
  ZF_LOGI("`csv_file_appendchar` called with value argument `%c`.", (char)value);

  if (streamdata == NULL) {
    ZF_LOGD("`csvstream_type` provided was NULL, bad value.");
    return;
  }

  csvfilereader fr = (csvfilereader)streamdata;

  /* expand field, if neccessary. hopefully efficiently, needs validation */
  if ((fr->size_f + 1) >= fr->capacity_f) {
    ZF_LOGD(
      "`csvfilereader` field size required exceeds capacity, calling `realloc` to expand.");

    if (fr->capacity_f > 4096) {
      ZF_LOGD("`csvfilereader` field is greater than 4KiB, expanding by 1KiB.");
      fr->capacity_f += 1024;
    }
    else {
      ZF_LOGD(
        "`csvfilereader` field is less than 4KiB, doubling current capacity.");
      fr->capacity_f *= 2;
    }
    fr->field = realloc(fr->field, (fr->capacity_f + 1));
    ZF_LOGI("`csvfilereader` field reallocated to new size of: `%lu`.",
            (long unsigned int)fr->capacity_f);

    memset((fr->field + fr->size_f), 0, ((fr->capacity_f + 1) - fr->size_f));
  }

  fr->field[fr->size_f++] = (char)value;
  ZF_LOGD("Appending character to field: `%c` at position: `%lu`.",
          (char)value,
          (long unsigned int)fr->size_f);
}

void csv_file_savefield(csvstream_type streamdata) {
  ZF_LOGI("`csv_file_savefield` called");

  if (streamdata == NULL) {
    ZF_LOGD("`csvstream_type` provided was NULL, bad value.");
    return;
  }

  csvfilereader fr = (csvfilereader)streamdata;
  char *temp       = NULL;

  /* grow field, if neccessary */
  if ((fr->size_r + 1) >= fr->capacity_r) {
    ZF_LOGD(
      "`csvfilereader` record size required exceeds capacity, calling `realloc` to expand.");

    if (fr->capacity_r > 128) {
      ZF_LOGD(
        "`csvfilereader` record capacity greater than 128, expanding by 128.");
      fr->capacity_r += 128;
    }
    else {
      ZF_LOGD("`csvfilereader` record capacity less than 128, doubling capacity.");
      fr->capacity_r *= 2;
    }
    fr->record = realloc(fr->record, fr->capacity_r);
    ZF_LOGI("`csvfilereader` record reallocated to new size of: `%lu`.",
            (long unsigned int)fr->capacity_r);
  }

  if ((temp = calloc(fr->size_f + 1, sizeof *fr->field)) == NULL) {
    ZF_LOGD("`csvfilereader` record field could not be allocated.");
    return;
  }

  /* copy up to the size of the current field */
  ZF_LOGD("Beginning to copy field to temp.");
  memcpy(temp, fr->field, fr->size_f);
  ZF_LOGD("Completed copying field to temp, value: `%s`.", temp);
  fr->record[fr->size_r] = temp;
  fr->size_r            += 1;

  /* set field back to the beginning of the field */
  fr->size_f = 0;
  memset(fr->field, 0, fr->capacity_f);
  ZF_LOGD("Reset field buffer to filled with \'\\0\' characters.");
}

CSV_CHAR_TYPE csv_file_saverecord(csvstream_type  streamdata,
                                  csvrecord_type *fields,
                                  size_t         *length) {
  ZF_LOGI("`csv_file_saverecord` called.");

  if (streamdata == NULL) {
    ZF_LOGD("`csv_file_saverecord` streamdata is NULL.");
    *fields = NULL;
    *length = 0;
    return CSV_CHAR;
  }

  csvfilereader fr = (csvfilereader)streamdata;

  /* allocate string array to pass the pointer list to caller */
  char **record = NULL;
  *length = fr->size_r;
  ZF_LOGD("`csv_file_saverecord` record length `%lu`.",
          (long unsigned int)(*length));

  if ((record = malloc(sizeof *record * fr->size_r)) == NULL) {
    ZF_LOGD("`csv_file_saverecord` record could not be allocated.");
    *fields = NULL;
    *length = 0;
    return CSV_CHAR;
  }

  ZF_LOGD("`csv_file_saverecord` copying records to output.");

  for (size_t i = 0; i < fr->size_r; ++i) {
    record[i]     = fr->record[i];
    fr->record[i] = NULL;
    ZF_LOGV("`csv_file_saverecord` field: `%lu` value: `%s`.",
            (long unsigned int)i,
            record[i]);
  }
  *fields = (csvrecord_type)record;

  /* reset internal field and record index */
  fr->size_f = 0;
  fr->size_r = 0;

  /*
   * this enum gives the caller a clue of the appropriate char type for casting
   * in general, a custom set of callbacks won't vary in the character types
   * this enum is provided to allow a consistent API between character widths
   */
  return CSV_CHAR;
}

void csv_read_filepath_close(csvstream_type streamdata) {
  ZF_LOGI("streamdata is %s.", streamdata == NULL ? "NULL" : "NOT NULL");

  if (streamdata != NULL) {
    csvfilereader fr = (csvfilereader)streamdata;

    /* fr->filepath is allocated externally, not freed here because it could be
     *  a string literal */
    if (fr->file != NULL) {
      ZF_LOGD("file is not null, closing.");
      fclose(fr->file);
    }

    if (fr->field != NULL) {
      ZF_LOGD("field is not null, freeing.");
      free(fr->field);
    }

    if (fr->record != NULL) {
      ZF_LOGD("record is not null, freeing.");

      for (size_t i = 0; i < fr->size_r; ++i) {
        if (fr->record[i] != NULL) {
          ZF_LOGD("csvfilereader->record[%lu] is not null, freeing.",
                  (long unsigned int)i);
          free(fr->record[i]);
        }
      }
      free(fr->record);
    }

    free(fr);
  }
}

void csv_read_file_close(csvstream_type streamdata) {
  ZF_LOGI("streamdata is %s.", streamdata == NULL ? "NULL" : "NOT NULL");

  if (streamdata != NULL) {
    csvfilereader fr = (csvfilereader)streamdata;

    /* fr->filepath is allocated externally, not freed here because it could be
     *  a string literal */

    /* fr->file is allocated externally, therefore not freed here */

    if (fr->field != NULL) {
      ZF_LOGD("field is not null, freeing.");
      free(fr->field);
    }

    if (fr->record != NULL) {
      ZF_LOGD("record is not null, freeing.");

      for (size_t i = 0; i < fr->size_r; ++i) {
        if (fr->record[i] != NULL) {
          ZF_LOGD("csvfilereader->record[%lu] is not null, freeing.",
                  (long unsigned int)i);
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
  }
  else {
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
inline bool parse_start_record(csvreader                reader,
                               csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  if (value == '\0') {
    ZF_LOGD("Empty record indicated");

    /* indicates empty record */
    return false;
  }
  else if ((value == '\n') || (value == '\r')) {
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

inline void parse_start_field(csvreader                reader,
                              csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  if ((value == '\0') || (value == '\n') || (value == '\r')) {
    ZF_LOGD("encountered boundary character of \\0, \\r or \\n, saving field");
    (*reader->savefield)(reader->streamdata);

    if (value == '\0') {
      reader->parser_state = START_RECORD;
      ZF_LOGD("setting parser state to START_RECORD");
    }
    else {
      reader->parser_state = EAT_CRNL;
      ZF_LOGD("setting parser state to EAT_CRNL");
    }
  }
  else if ((value == csvdialect_get_quotechar(reader->dialect)) &&
           (QUOTE_STYLE_NONE == csvdialect_get_quotestyle(reader->dialect))) {
    reader->parser_state = IN_QUOTED_FIELD;
    ZF_LOGD("setting parser state to IN_QUOTED_FIELD");
  }
  else if (value == csvdialect_get_escapechar(reader->dialect)) {
    reader->parser_state = ESCAPED_CHAR;
    ZF_LOGD("setting parser state to ESCAPED_CHAR");
  }
  else if ((value == ' ') && csvdialect_get_skipinitialspace(reader->dialect)) {
    // no change
    return;
  }
  else if (value == csvdialect_get_delimiter(reader->dialect)) {
    /* end of field, so therefore empty/null field */
    ZF_LOGD("encountered delimiter, saving field - no change in parser state");
    (*reader->savefield)(reader->streamdata);
  }
  else {
    (*reader->appendchar)(reader->streamdata, value);
    reader->parser_state = IN_FIELD;
    ZF_LOGD("appending character to field");
    ZF_LOGD("setting parser state to IN_FIELD");
  }
}

inline void parse_escaped_char(csvreader                reader,
                               csv_comparison_char_type value) {
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

inline void parse_in_field(csvreader                reader,
                           csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  /* in unquoted field */
  if ((value == '\n') || (value == '\r') || (value == '\0')) {
    (*reader->savefield)(reader->streamdata);

    if (value == '\0') {
      reader->parser_state = START_RECORD;
      ZF_LOGD("setting parser state to START_RECORD");
    }
    else {
      reader->parser_state = EAT_CRNL;
      ZF_LOGD("setting parser state to EAT_CRNL");
    }
  }
  else if (value == csvdialect_get_escapechar(reader->dialect)) {
    reader->parser_state = ESCAPED_CHAR;
    ZF_LOGD("setting parser state to ESCAPED_CHAR");
  }
  else if (value == csvdialect_get_delimiter(reader->dialect)) {
    (*reader->savefield)(reader->streamdata);
    reader->parser_state = START_FIELD;
    ZF_LOGD("setting parser state to START_FIELD");
  }
  else {
    (*reader->appendchar)(reader->streamdata, value);
  }
}

inline void parse_in_quoted_field(csvreader                reader,
                                  csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  if (value == '\0') { /* no-op */ }
  else if (value == csvdialect_get_escapechar(reader->dialect)) {
    reader->parser_state = ESCAPE_IN_QUOTED_FIELD;
    ZF_LOGD("setting parser state to ESCAPE_IN_QUOTED_FIELD");
  }
  else if ((value == csvdialect_get_quotechar(reader->dialect)) &&
           (QUOTE_STYLE_NONE != csvdialect_get_quotestyle(reader->dialect))) {
    if (csvdialect_get_doublequote(reader->dialect)) {
      reader->parser_state = ESCAPE_IN_QUOTED_FIELD;
      ZF_LOGD("setting parser state to ESCAPE_IN_QUOTED_FIELD");
    }
    else {
      reader->parser_state = IN_FIELD;
      ZF_LOGD("setting parser state to IN_FIELD");
    }
  }
  else {
    (*reader->appendchar)(reader->streamdata, value);
  }
}

inline void parse_quote_in_quoted_field(csvreader                reader,
                                        csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  if ((csvdialect_get_quotestyle(reader->dialect) != QUOTE_STYLE_NONE) &&
      (value == csvdialect_get_quotechar(reader->dialect))) {
    /* save "" as " */
    (*reader->appendchar)(reader->streamdata, value);
    reader->parser_state = IN_QUOTED_FIELD;
    ZF_LOGD("setting parser state to IN_QUOTED_FIELD");
  }
  else if (value == csvdialect_get_delimiter(reader->dialect)) {
    (*reader->savefield)(reader->streamdata);
    reader->parser_state = START_FIELD;
    ZF_LOGD("setting parser state to START_FIELD");
  }
  else if ((value == '\0') || (value == '\r') || (value == '\n')) {
    (*reader->savefield)(reader->streamdata);
    reader->parser_state = START_FIELD;
    ZF_LOGD("setting parser state to START_FIELD");
  } else {
    (*reader->savefield)(reader->streamdata);
    reader->parser_state = (value == '\0') ? START_RECORD : EAT_CRNL;

    if (value == '\0') {
      reader->parser_state = START_RECORD;
      ZF_LOGD("setting parser state to START_RECORD");
    }
    else {
      reader->parser_state = EAT_CRNL;
      ZF_LOGD("setting parser state to EAT_CRNL");
    }
  }
}

inline void parse_value(csvreader                reader,
                        csv_comparison_char_type value) {
  ZF_LOGD("input value: %c", (char)value);

  switch (reader->parser_state) {
  case START_RECORD:

    if (!parse_start_record(reader, value)) break;

  /* else, fall through */

  case START_FIELD:
    parse_start_field(reader, value);
    break;

  case ESCAPED_CHAR:
    parse_escaped_char(reader, value);
    break;

  case AFTER_ESCAPED_CRNL:

    if (value == '\0') break;

  /* else, fallthrough */

  case IN_FIELD:
    parse_in_field(reader, value);
    break;

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

    if (value == '\0') reader->parser_state = START_RECORD;
    break;

  default:
    break;
  }
}
