#include <stdio.h>
#include <stdlib.h>

#include "csv.h"

struct csv_writer {
	csvdialect	dialect;
	FILE				* file;
};

csvwriter csvwriter_init(csvdialect dialect,
                         const char *filepath) {
	return NULL;
}

void csvwriter_close(csvwriter *writer) {
	if (*writer == NULL) return;
	if ( (*writer)->dialect != NULL )
		csvdialect_close(&( (*writer)->dialect ) );
	if ( (*writer)->file != NULL )
		fclose( (*writer)->file );
	free(*writer);
	*writer = NULL;
}

csvreturn csvwriter_next_record(csvwriter				writer,
                                const csvrecord record) {
	return csvreturn_init();
}
