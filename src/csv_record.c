#include <stdlib.h>

#include "csv.h"

struct csv_record {
	csvfield  * data;
};

csvrecord csvrecord_init(void) {
	return NULL;
}

csvrecord csvrecord_buffer_init(size_t buffer) {
	return NULL;
}

void csvrecord_close(csvrecord *record) {
	if (*record == NULL) return;

	size_t i, length;
	if ( (*record)->data != NULL ) {
		length = csvrecord_length(*record);
		for (i = 0; i < length; ++i) {
			csvfield_close(&( (*record)->data[i] ) );
		}
		free( (*record)->data );
	}
	free(*record);
	*record = NULL;
}

size_t csvrecord_available(const csvrecord record) {
	return 0;
}

size_t csvrecord_length(const csvrecord record) {
	return 0;
}

size_t csvrecord_capacity(const csvrecord record) {
	return 0;
}

int csvrecord_cmp(const csvrecord lhs,
                  const csvrecord rhs) {
	return 0;
}

csvreturn csvrecord_set(csvrecord record,
                        int				char_) {
	return csvreturn_init();
}

csvreturn csvrecord_copy(csvrecord				*dest,
                         const csvrecord	source) {
	return csvreturn_init();
}

csvreturn csvrecord_clear(csvrecord record) {
	return csvreturn_init();
}

csvreturn csvrecord_reserve(csvrecord record,
                            size_t		newsize) {
	return csvreturn_init();
}

csvreturn csvrecord_shrink_to_fit(csvrecord record) {
	return csvreturn_init();
}

csvreturn csvrecord_append(csvrecord	record,
                           csvfield		field) {
	return csvreturn_init();
}
