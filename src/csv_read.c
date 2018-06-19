/**
 * Significantly inspired by Python's '_csv.c' implementation.
 *
 * https://github.com/python/cpython/blob/master/Modules/_csv.c
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "csv.h"
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
	AFTER_ESCAPED_CRNL
} CSV_READER_PARSER_STATE;

struct csv_reader {
	csvdialect								dialect;
	csvstream_type						streamdata;
	csvstream_getnext					getnext;
	csvstream_appendchar			appendchar;
	csvstream_savefield				savefield;
	csvstream_saverecord			saverecord;
	csvstream_close						closer;
	CSV_READER_PARSER_STATE		parser_state;
};

/*
 * Begin - FILE* based callback implementations
 */
typedef struct csv_file_reader {
  /* if providex, might be null but useful for debug info */
	char const  *filepath;

  /* needed for the input stream */
	FILE		*file;

	char		**record;
	char		*field;
	size_t	capacity_f;
	size_t	size_f;
	size_t	capacity_r;
	size_t	size_r;
} *csvfilereader;

csvfilereader csvfilereader_init(void) {
	csvfilereader fr = NULL;

	if ( (fr = malloc(sizeof *fr) ) == NULL ) return NULL;

	fr->filepath = NULL;
	fr->file = NULL;

  /* 256 chosen as a default because this is generally the max
   * witdth of a SQL database VARCHAR field.
   */
	fr->size_f = 0;
	fr->capacity_f = 256;

	if ( (fr->field = malloc(sizeof *fr->field * fr->capacity_f) ) == NULL ) {
    /* couldn't allocate string */
		free(fr);
		return NULL;
	}

  /* 8 is arbitrary, subject to redesign on performance analysis */
	fr->size_r = 0;
	fr->capacity_r = 8;

	if ( (fr->record = malloc(sizeof *fr->record * fr->capacity_r) ) == NULL ) {
    /* couldn't allocate record array */
		free( fr->field);
		free( fr);
		return NULL;
	}
	return fr;
}

/*
 * make a file reader from a filepath, shares common logic and implementation
 * with the FILE* initializer.
 */
csvfilereader csv_filepath_open(char const *filepath) {
	if (filepath == NULL) return NULL;

	csvfilereader fr = NULL;
	FILE					*fileobj = NULL;

	if ( (fileobj = fopen(filepath, "rb") ) == NULL )
		return NULL;

	if ( (fr = csvfilereader_init() ) == NULL ) {
		fclose(fileobj);
		return NULL;
	}

	fr->filepath = filepath;
	fr->file = fileobj;
	return fr;
}

/*
 * make a file reader from a supplied FILE*
 */
csvfilereader csv_file_open(FILE *fileobj) {
	if (fileobj == NULL) return NULL;

	csvfilereader fr = NULL;

	if ( (fr = csvfilereader_init() ) == NULL )
		return NULL;

	fr->filepath = NULL;
	fr->file = fileobj;
	return fr;
}

CSV_STREAM_SIGNAL csv_file_getnext(csvstream_type streamdata,
                                   char32_t				*value) {
	if (streamdata == NULL) {
		*value = CSV_UNDEFINED_CHAR;
		return CSV_ERROR;
	}

	csvfilereader fr = (csvfilereader)streamdata;
	int						c = fgetc(fr->file);

	if (feof(fr->file) ) {
		*value = 0;
		return CSV_EOF;
	}

	if (ferror(fr->file) ) {
		perror("Error detected while reading CSV.");
		*value = CSV_UNDEFINED_CHAR;
		return CSV_ERROR;
	}

	*value = (char32_t)c;
	return CSV_GOOD;
}

void csv_file_appendchar(csvstream_type streamdata,
                         char32_t				value) {
	if (streamdata == NULL) return;

	csvfilereader fr = (csvfilereader)streamdata;

  /* expand field, if neccessary. hopefully efficiently, needs validation */
	if ( (fr->size_f + 1) > fr->capacity_f ) {
		if (fr->capacity_f > 4096)
			fr->capacity_f += 1024;
		else
			fr->capacity_f *= 2;
		fr->field = realloc(fr->field, (fr->capacity_f + 1) );
	}

	fr->field[fr->size_f++] = (char)value;
}

void csv_file_savefield(csvstream_type streamdata) {
	if (streamdata == NULL) return;

	csvfilereader fr = (csvfilereader)streamdata;

  /* grow field, if neccessary */
	if ( (fr->size_r + 1) > fr->capacity_r ) {
		if (fr->capacity_r > 128)
			fr->capacity_r += 128;
		else
			fr->capacity_r *= 2;
		fr->record = realloc(fr->record, fr->capacity_r);
	}

	if ( (fr->record[fr->size_r++] =
					calloc(fr->size_f + 1, sizeof *fr->field) ) == NULL ) {
    /* raise some sort of exception... */
		fr->record[fr->size_r++] = NULL;
		return;
	}

  /* copy up to the size of the current field */
	memcpy(fr->record[fr->size_r], fr->field, fr->size_f);

  /* set field back to the beginning of the field */
	fr->size_f = 0;
}

CSV_CHAR_TYPE csv_file_saverecord(csvstream_type	streamdata,
                                  csvrecord_type	*fields,
                                  size_t					*length) {
	if (streamdata == NULL) {
		*fields = NULL;
		*length = 0;
		return CSV_CHAR8;
	}

	csvfilereader fr = (csvfilereader)streamdata;

  /* allocate string array to pass the pointer list to caller */
	char **record = NULL;

	if ( (record = malloc(sizeof *record * fr->size_r) ) == NULL ) {
		*fields = NULL;
		*length = 0;
		return CSV_CHAR8;
	}

	for (size_t i = 0; i < fr->size_r; ++i) {
		record[i] = fr->record[i];
	}

  /* reset internal field and record index */
	fr->size_f = 0;
	fr->size_r = 0;

  /* set outputs, after this the caller owns the memory for the field strings */
  /* a memory leak will ensue if the caller does not free the memory when */
  /* completed with it. */
	*length = fr->size_r;
	*fields = (csvrecord_type)record;

  /* this enum gives the caller a clue of the appropriate char type for casting */
  /* in general, a custom set of callbacks won't vary in the character types */
  /* this enum is provided to allow a consistent API between character widths */
	return CSV_CHAR8;
}

void csv_filepath_close(csvstream_type streamdata) {
	if (streamdata != NULL) {
		csvfilereader fr = (csvfilereader)streamdata;

    /* fr->filepath is allocated externally, not freed here because it could be
     *  a string literal */
		if (fr->file != NULL)
			fclose(fr->file);

		if (fr->field != NULL)
			free(fr->field);

		if (fr->record != NULL) {
			for (size_t i = 0; i < fr->size_r; ++i) {
				free(fr->record[i]);
			}
			free(fr->record);
		}

		free(fr);
	}
}

void csv_file_close(csvstream_type streamdata) {
	if (streamdata != NULL) {
		csvfilereader fr = (csvfilereader)streamdata;

    /* fr->filepath is allocated externally, not freed here because it could be
     *  a string literal */

    /* fr->file is allocated externally, therefore not freed here */

		if (fr->field != NULL)
			free(fr->field);
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
	csvreader reader = NULL;

	if ( (reader = malloc(sizeof *reader) ) == NULL )
		return NULL;

	reader->parser_state = START_RECORD;
	reader->dialect = (dialect == NULL) ? csvdialect_init() : dialect;
	reader->streamdata = NULL;
	reader->getnext = NULL;
	reader->appendchar = NULL;
	reader->savefield = NULL;
	reader->saverecord = NULL;
	reader->closer = NULL;

	return reader;
}

/*
 * Begin of 'csv/read.h' implementations
 */
csvreader csvreader_init(csvdialect dialect,
                         const char *filepath) {
	csvreader			reader = NULL;
	csvfilereader fr = NULL;

	if ( (fr = csv_filepath_open(filepath) ) == NULL )
    /* couldn't allocate the IO struct */
		return NULL;

  /* broke out of if statement because I find that visually difficult to read */
	reader = csvreader_advanced_init(dialect,
		&csv_file_getnext,
		&csv_file_appendchar,
		&csv_file_savefield,
		&csv_file_saverecord,
		(csvstream_type)fr);

	if (reader != NULL)
		reader = csvreader_set_closer(reader, &csv_filepath_close);

  /* final validation */
	if (reader == NULL) {
		csv_filepath_close( (csvstream_type)fr );
		return NULL;
	}
	return reader;
}

csvreader csvreader_file_init(csvdialect	dialect,
                              FILE				*fileobj) {
	csvreader			reader = NULL;
	csvfilereader fr = NULL;

	if ( (fr = csv_file_open(fileobj) ) == NULL )
    /* couldn't allocate the IO struct */
		return NULL;

  /* broke out of if statement because I find that visually difficult to read */
	reader = csvreader_advanced_init(dialect,
		&csv_file_getnext,
		&csv_file_appendchar,
		&csv_file_savefield,
		&csv_file_saverecord,
		(csvstream_type)fr);

	if (reader != NULL)
		reader = csvreader_set_closer(reader, &csv_file_close);

  /* final validation */
	if (reader == NULL) {
		csv_file_close( (csvstream_type)fr );
		return NULL;
	}
	return reader;
}

csvreader csvreader_advanced_init(csvdialect						dialect,
                                  csvstream_getnext			getnext,
                                  csvstream_appendchar	appendchar,
                                  csvstream_savefield		savefield,
                                  csvstream_saverecord	saverecord,
                                  csvstream_type				streamdata) {
  /* validation step */
  /* other than dialect, all arguments must be non-null */
	if ( (getnext == NULL) || (appendchar == NULL) || (savefield == NULL) ||
	     (saverecord == NULL) || (streamdata == NULL) )
		return NULL;

	csvreader reader = _csvreader_init(dialect);

	reader->streamdata = streamdata;
	reader->getnext = getnext;
	reader->appendchar = appendchar;
	reader->savefield = savefield;
	reader->saverecord = saverecord;

	return reader;
}

csvreader csvreader_set_closer(csvreader				reader,
                               csvstream_close	closer) {
	if (reader == NULL)
		return NULL;
	reader->closer = closer;
	return reader;
}

void csvreader_close(csvreader *reader) {
  /* optimization opportunity after test success */
	csvreader r = *reader;

	if (r == NULL) return;

  /* optimization opportunity after test success */
	csvdialect d;
	d = r->dialect;
	csvdialect_close(&d);

  /* optimization opportunity after test success */
	csvstream_close closer;
	closer = r->closer;

	if (closer != NULL)
		(*closer)(r->streamdata);

	free(r);
	*reader = NULL;
}

/*
 * TODO: core parsing logic -- needs to be before 'csvreader_next_record'
 */

/* bool controls 'should continue' (true) or should break switch (false) */
inline bool parse_start_record(csvreader	reader,
                               char32_t		value) {
	if (value == '\0') {
    /* indicates empty record */
		return false;
	}
	else if ( (value == '\n') || (value == '\r') ) {
		reader->parser_state = EAT_CRNL;
		return false;
	}

  /* normal character, handle as start field */
	reader->parser_state = START_FIELD;
	return true;
}

inline void parse_start_field(csvreader reader,
                              char32_t	value) {
	if ( (value == '\0') || (value == '\n') || (value == '\r') ) {
		(*reader->savefield)(reader->streamdata);
		reader->parser_state = value == '\0' ? START_RECORD : EAT_CRNL;
	}
	else if ( (value == csvdialect_get_quotechar(reader->dialect) ) &&
	          (QUOTE_STYLE_NONE == csvdialect_get_quotestyle(reader->dialect) ) ) {
		reader->parser_state = IN_QUOTED_FIELD;
	}
	else if (value == csvdialect_get_escapechar(reader->dialect) ) {
		reader->parser_state = ESCAPED_CHAR;
	}
	else if ( (value == ' ') && csvdialect_get_skipinitialspace(reader->dialect) ) {}
	else if (value == csvdialect_get_delimiter(reader->dialect) ) {
    /* end of field, so therefore empty/null field */
		(*reader->savefield)(reader->streamdata);
	}
	else {
		(*reader->appendchar)(reader->streamdata, value);
		reader->parser_state = IN_FIELD;
	}
}

inline void parse_escaped_char(csvreader	reader,
                               char32_t		value) {
	if ( (value == '\n') || (value == '\r') ) {
		(*reader->appendchar)(reader->streamdata, value);
		reader->parser_state = AFTER_ESCAPED_CRNL;
		return;
	}

	if (value == '\0')
		value = '\n';

	(*reader->appendchar)(reader->streamdata, value);
	reader->parser_state = IN_FIELD;
}

inline void parse_in_field(csvreader	reader,
                           char32_t		value) {
  /* in unquoted field */
	if ( (value == '\n') || (value == '\r') || (value == '\0') ) {
		(*reader->savefield)(reader->streamdata);
		reader->parser_state = value == '\0' ? START_RECORD : EAT_CRNL;
	}
	else if (value == csvdialect_get_escapechar(reader->dialect) ) {
		reader->parser_state = ESCAPED_CHAR;
	}
	else if (value == csvdialect_get_delimiter(reader->dialect) ) {
		(*reader->savefield)(reader->streamdata);
		reader->parser_state = START_FIELD;
	}
	else {
		(*reader->appendchar)(reader->streamdata, value);
	}
}

inline void parse_in_quoted_field(csvreader reader,
                                  char32_t	value) {
	if (value == '\0') { /* no-op */ }
	else if (value == csvdialect_get_escapechar(reader->dialect) ) {
		reader->parser_state = ESCAPE_IN_QUOTED_FIELD;
	}
	else if ( (value == csvdialect_get_quotechar(reader->dialect) ) &&
	          (QUOTE_STYLE_NONE != csvdialect_get_quotestyle(reader->dialect) ) ) {
		if (csvdialect_get_doublequote(reader->dialect) )
			reader->parser_state = QUOTE_IN_QUOTED_FIELD;
		else
			reader->parser_state = IN_FIELD;
	}
	else {
		(*reader->appendchar)(reader->streamdata, value);
	}
}

inline void parse_quote_in_quoted_field(csvreader reader,
                                        char32_t	value) {
	if ( (csvdialect_get_quotestyle(reader->dialect) != QUOTE_STYLE_NONE) &&
	     (value == csvdialect_get_quotechar(reader->dialect) ) ) {
    /* save "" as " */
		(*reader->appendchar)(reader->streamdata, value);
		reader->parser_state = IN_QUOTED_FIELD;
	}
	else if (value == csvdialect_get_delimiter(reader->dialect) ) {
		(*reader->savefield)(reader->streamdata);
		reader->parser_state = START_FIELD;
	}
	else if ( (value == '\0') || (value == '\r') || (value == '\n') ) {
		(*reader->savefield)(reader->streamdata);
		reader->parser_state = START_FIELD;
	} else {
		(*reader->savefield)(reader->streamdata);
		reader->parser_state = (value == '\0') ? START_RECORD : EAT_CRNL;
	}
}

inline void parse_value(csvreader reader,
                        char32_t	value) {
	switch (reader->parser_state) {
	case START_RECORD:

		if (!parse_start_record(reader, value) ) break;

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

		if (value == '\0')
			reader->parser_state = START_RECORD;
		break;

	default:
		fprintf(stderr, "Undefined CSV Reader Parser State: %d",
			reader->parser_state);
		break;
	}
}

csvreturn csvreader_next_record(csvreader				reader,
                                CSV_CHAR_TYPE		*char_type,
                                csvrecord_type	*record,
                                size_t					*record_length) {
	char32_t					value = 0;
	CSV_STREAM_SIGNAL signal = CSV_GOOD;

	do {
		if ( (signal = (*reader->getnext)(reader->streamdata, &value) ) == CSV_GOOD )
			parse_value(reader, value);
		else
			break;
	} while (reader->parser_state != START_RECORD);

	*char_type =
		(*reader->saverecord)(reader->streamdata, record, record_length);

	if (signal == CSV_GOOD) {
		return csvreturn_init(true);
	}
	else if (signal == CSV_EOF) {
		csvreturn rc = csvreturn_init(true);
		rc.csv_eof = 1;
		return rc;
	}
	else {
		csvreturn rc = csvreturn_init(false);
		rc.csv_io_error = 1;
		return rc;
	}
}
