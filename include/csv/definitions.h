/**
 * @file csv/definitions.h
 * @author Robert Smith
 * @date 2018-06-18
 * @brief Definitions for return codes and validation macros
 */
#ifndef CSV_DEFINITIONS_H_
#define CSV_DEFINITIONS_H_

#include <limits.h>
#include <stdbool.h>

typedef unsigned char csvchar_type;


/**
 * @brief Defines a @c csvdialect char parameter which has not been configured.
 *
 * If this value is set as a value for a @c csvdialect attribute, this defines
 * a character field which has no configuration and therefore is turned off.
 */
#define CSV_UNDEFINED_CHAR ( (csvchar_type)UCHAR_MAX )

/**
 * @brief Unknown string length
 *
 * If this value is returned, the length of the string is unknown or the string
 * is undefined.
 */
#define CSV_UNDEFINED_STRING_LENGTH ( (size_t)SIZE_MAX )

/**
 * @brief CSV Return type
 *
 * Represents a bitmask for API operations. Guaranteed to be less than or equal
 * to the size of a @c void*
 *
 * Note: lineterminator config errors don't matter for reader
 */
typedef struct csv_return {
	unsigned long		succeeded							: 1;
	unsigned long		truncated							: 1;
	unsigned long		dialect_null					: 1;
	unsigned long		delimiter_error				: 1;
	unsigned long		quoteescape_error			: 1;
	unsigned long		lineterminator_error	: 1;
} csvreturn;

/**
 * Make a CSV Return value initialized with all fields to default zeroes.
 *
 * @return  CSV Return type
 */
inline csvreturn csvreturn_init(void) {
	csvreturn rc = { 0 };

	return rc;
}

/**
 * Validate call to CSV API was performed successfully
 *
 * @param  retcode  CSV Return type to analyze
 *
 * @return          @c true indicates successful operation, @c false indicates
 *                  failure.
 *
 * @see csv_failure
 */
inline bool csv_success(csvreturn retcode) {
	return retcode.succeeded;
}

/**
 * Validate call to CSV API failed
 *
 * @param  retcode  CSV Return type to analyze
 *
 * @return          @c false indicates successful operation, @c true indicates
 *                  failure.
 *
 * @see csv_success
 */
inline bool csv_failure(csvreturn retcode) {
	return !( (bool)retcode.succeeded );
}

#endif  /* CSV_DEFINITIONS_H_ */
