#include <stdlib.h>

#include "csv.h"

struct csv_field {
	char  * data;
};

csvfield csvfield_init(void) {
	return NULL;
}

csvfield csvfield_buffer_init(size_t buffer) {
	return NULL;
}

void csvfield_close(csvfield *field) {
	if (*field == NULL) return;
	if ( (*field)->data != NULL )
		free( (*field)->data );
	free(*field);
	*field = NULL;
}

size_t csvfield_available(const csvfield field) {
	return 0;
}

size_t csvfield_length(const csvfield field) {
	return 0;
}

size_t csvfield_capacity(const csvfield field) {
	return 0;
}

int csvfield_cmp(const csvfield lhs,
                 const csvfield rhs) {
	return 0;
}

csvreturn csvfield_set(csvfield field,
                       int			char_) {
	return csvreturn_init();
}

csvreturn csvfield_copy(csvfield				*dest,
                        const csvfield	source) {
	return csvreturn_init();
}

csvreturn csvfield_clear(csvfield field) {
	return csvreturn_init();
}

csvreturn csvfield_reserve(csvfield field,
                           size_t		newsize) {
	return csvreturn_init();
}

csvreturn csvfield_shrink_to_fit(csvfield field) {
	return csvreturn_init();
}

csvreturn csvfield_append(csvfield	field,
                          int				char_) {
	return csvreturn_init();
}
