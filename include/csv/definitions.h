/**
 * @file csv/definitions.h
 * @author Robert Smith
 * @date 2018-06-18
 * @brief Definitions for return codes and validation macros
 */
#ifndef CSV_DEFINITIONS_H_
#define CSV_DEFINITIONS_H_

#include <stdbool.h>
#include <limits.h>

/**
 * @brief Defines a @c csvdialect char parameter which has not been configured.
 *
 * If this value is set as a value for a @c csvdialect attribute, this defines
 * a character field which has no configuration and therefore is turned off.
 */
#define CSV_UNDEFINED_CHAR CHAR_MAX

/**
 * @brief CSV Dialect Lineterminator maximum length
 *
 * This value can be set at compile time, but defaults to 4 characters
 */
#ifndef CSV_LINETERMINATOR_MAX
# define CSV_LINETERMINATOR_MAX 4
#endif /* CSV_LINETERMINATOR_MAX */

/**
 * @brief CSV Return type
 *
 * Represents a bitmask for API operations. Guaranteed to be less than or equal
 * to the size of a @c void*
 */
typedef struct csv_return csvreturn;

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
bool csv_success(csvreturn retcode);

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
bool csv_failure(csvreturn retcode);

#endif /* CSV_DEFINITIONS_H_ */
