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
#include <stdint.h>
#include <uchar.h>

/**
 * @brief Defines a @c csvdialect char parameter which has not been configured.
 *
 * If this value is set as a value for a @c csvdialect attribute, this defines
 * a character field which has no configuration and therefore is turned off.
 */
#define CSV_UNDEFINED_CHAR ((char32_t)INT_LEAST32_MAX)

/**
 * @brief Unknown string length
 *
 * If this value is returned, the length of the string is unknown or the string
 * is undefined.
 */
#define CSV_UNDEFINED_STRING_LENGTH ((size_t)SIZE_MAX)

/**
 * @brief CSV Return type
 *
 * Represents a bitmask for API operations. Guaranteed to be less than or equal
 * to the size of a @c void*
 *
 * Note: lineterminator config errors don't matter for reader
 */
typedef struct csv_return {
  uint64_t succeeded            : 1;
  uint64_t csv_eof              : 1;
  uint64_t csv_io_error         : 1;
  uint64_t truncated            : 1;
  uint64_t dialect_null         : 1;
  uint64_t delimiter_error      : 1;
  uint64_t quoteescape_error    : 1;
  uint64_t lineterminator_error : 1;
} csvreturn;

/**
 * Make a CSV Return value initialized with all fields to default zeroes.
 *
 * @return  CSV Return type
 */
inline csvreturn csvreturn_init(bool succeeded) {
  csvreturn rc = {
    .succeeded            = succeeded,
    .csv_eof              = 0,
    .csv_io_error         = 0,
    .truncated            = 0,
    .dialect_null         = 0,
    .delimiter_error      = 0,
    .quoteescape_error    = 0,
    .lineterminator_error = 0,
  };

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
  return !retcode.succeeded;
}

/**
 * @brief Verify if EOF signal was returned
 *
 * @param  retcode CSV Return type
 *
 * @return         boolean, true indicates EOF returned, false indicates no EOF
 */
inline bool csv_eof(csvreturn retcode) {
  return retcode.csv_eof;
}

#endif /* CSV_DEFINITIONS_H_ */
