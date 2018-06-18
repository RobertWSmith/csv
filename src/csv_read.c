#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "csv.h"

struct csv_reader {
	csvdialect						dialect;
	csvstream_type				streamdata;
	csvstream_close				closer;
	csvstream_getnext			getnext;
	csvstream_savefield		savefield;
	csvrecord							record_buffer;
};

/*
 * FILE* based callback implementations
 */
typedef struct csv_file_reader {
  /* if providex, might be null but useful for debug info */
	const char	* filepath;
  /* needed for the input stream */
	FILE				* file;

	char				* buffer;
	size_t			capacity;
	size_t			size;
} * csvfilereader;

/* TODO: finish implementing FILE object related callbacks
 */

typedef char32_t (*csvstream_getnext)(csvstream_type streamdata) csvstream_getnext;

char32_t csv_file_getnext(csvstream_type streamdata) {
	return 0;
}

void csv_file_savefield(csvstream_type	streamdata,
                        void						** field,
                        size_t					* length) {
	*length = 0;
	*field = NULL;
}

csvreader csvreader_init(csvdialect dialect,
                         const char *filepath) {
	csvfilereader fr = NULL;
	csvreader			reader = NULL;

	if ( (fr = malloc(sizeof *fr) ) == NULL )
		return NULL;

	fr->filepath = filepath;

	if ( (fr->file = fopen(fr->filepath, "rb") ) == NULL ) {
		free(fr);
		return NULL;
	}
  /* 256 chosen as a default because this is generally the max
   * witdth of a SQL database VARCHAR field.
   */
	fr->size = 0;
	fr->capacity = 256;
	if ( (fr->field = malloc(sizeof *fr->field * fr->capacity) ) == NULL ) {
		fr->field[0] = 0;
		fclose(fr->file);
		free(fr);
		return NULL;
	}

	reader = csvreader_advanced_init(dialect, &csv_file_getnext, &csv_file_savefield, fr);
	if (reader == NULL) {
		free(fr->field);
		fclose(fr->file);
		free(fr);
		return NULL;
	}
	return reader;
}

void csvreader_close(csvreader *reader) {
	if (*reader == NULL) return;

	if ( (*reader)->dialect != NULL ) csvdialect_close(&( (*reader)->dialect ) );

	if ( (*reader)->file != NULL ) fclose( (*reader)->file );

	if ()

		free(*reader);
	*reader = NULL;
}

csvreturn csvreader_next_record(csvreader reader,
                                csvrecord *record) {
	return csvreturn_init();
}
