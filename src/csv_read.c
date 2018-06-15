#include <stdio.h>
#include <stdlib.h>

#include "csv.h"

struct csv_reader {
	csvdialect	dialect;
	FILE				* file;
};

csvreader csvreader_init(csvdialect dialect,
                         const char *filepath) {
	return NULL;
}

void csvreader_close(csvreader *reader) {
	if (*reader == NULL) return;
	if ( (*reader)->dialect != NULL )
		csvdialect_close(&( (*reader)->dialect ) );
	if ( (*reader)->file != NULL )
		fclose( (*reader)->file );
	free(*reader);
	*reader = NULL;
}

csvreturn csvreader_next_record(csvreader reader,
                                csvrecord *record) {
	return csvreturn_init();
}
