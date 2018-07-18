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

#include "version.h"

/**
 * @brief CSV Linterminator definitions
 *
 * Definitions which help provide lineterminators of various styles and
 * character encodings. All begin with the style @c _CSV_LINETERMINATOR_*
 * followed by the line terminator style, which are currently defined as
 * @c CRNL ("\r\n"), @c CR ("\r") and @c NL ("\n"). Finally, the suffix
 * determines the character encoding, which is currently defined as
 * @c CHAR (@c char), @c WCHAR (@c wchar_t), @c CHARU8 (@c char, with UTF-8
 * encoding), @c CHAR16 (@c char16_t), @c CHAR32 (@c char32_t).
 *
 * There is also an attempt to determine the system default line terminator,
 * which is performed by first checking if @c _WIN32 or @c _WIN64 is defined.
 * If so, the @c CRNL variants are defined as the default. Next, there is a
 * check for legacy Macintosh OS 9 definitions, if so the @c CR variant is
 * assigned as the default. Finally, any other system is assumed to be a Unix
 * variant and the @c NL variant is assigned as the system default.
 *
 * Notes:
 *    borrowed from https://stackoverflow.com/a/6864861/2788895
 */
#if defined(_WIN32) || defined(WIN32) /* begin os detection */
#define CSV_LINETERMINATOR_SYSTEM_DEFAULT "\r\n"
#elif defined(macintosh) /* OS 9 - very old */
#define CSV_LINETERMINATOR_SYSTEM_DEFAULT "\r"
#else /* *nix case */
#define CSV_LINETERMINATOR_SYSTEM_DEFAULT "\n"
#endif /* end os detection */

/**
 * @brief CSV Stream Signal, controls parser behavior
 */
typedef enum CSV_STREAM_SIGNAL {
  CSV_GOOD,         /**< Next value available */
  CSV_EOF,          /**< End of File */
  CSV_EOR,          /**< End of Record */
  CSV_END_OF_FIELD, /**< End of field */
  CSV_ERROR,        /**< Some IO Error encountered */
} CSV_STREAM_SIGNAL;

/**
 * @brief CSV Stream Signal enumeration string representation
 *
 * Primarily used for logging.
 *
 * @param  state Current CSV Stream Signal
 *
 * @return       string representation of current CSV Stream Signal
 */
static inline const char* stream_signal(CSV_STREAM_SIGNAL csv_stream_signal) {
  switch (csv_stream_signal) {
    case CSV_GOOD: return "CSV_GOOD";

    case CSV_EOF: return "CSV_EOF";

    case CSV_EOR: return "CSV_EOR";

    case CSV_END_OF_FIELD: return "CSV_END_OF_FIELD";

    case CSV_ERROR: return "CSV_ERROR";
  }
}

/**
 * @brief Enum for callbacks to get or return when passing @c csvfield_type ot
 * @c csvreturn_type
 *
 * @c CSV_CHAR indicates the @c csvfield_type ot @c csvreturn_type is composed
 * of @c char
 */
typedef enum CSV_CHAR_TYPE {
  CSV_CHAR,      /**< indicates the @c csvfield_type ot @c csvreturn_type is
                    composed of @c char */
  CSV_WCHAR,     /**< indicates the @c csvfield_type ot @c csvreturn_type is
                    composed of @c wchar_t */
  CSV_UCHAR8,    /**< indicates the @c csvfield_type ot @c csvreturn_type is
                    composed of @c char encoded as UTF-8 */
  CSV_UCHAR16,   /**< indicates the @c csvfield_type ot @c csvreturn_type is
                    composed of @c char16_t */
  CSV_UCHAR32,   /**< indicates the @c csvfield_type ot @c csvreturn_type is
                    composed of @c char32_t */
  CSV_UNDEFINED, /**< indicates an error was encountered */
} CSV_CHAR_TYPE;

/**
 * @brief type which allows comparisons between characters of up to 32 bits
 *
 * This type is guaranteed to be at least 64-bits wide and signed, so a larger
 * number of values can be stored than a traditional 4-bit character to allow
 * for sentinal values like End of File, Record or Field to be defined without
 * clashing with defined values of signed or unsigned characters
 */
typedef int_fast64_t csv_comparison_char_type;

/**
 * @brief Defines a @c csvdialect char parameter which has not been configured.
 *
 * If this value is set as a value for a @c csvdialect attribute, this defines
 * a character field which has no configuration and therefore is turned off.
 */
#define CSV_UNDEFINED_CHAR ((csv_comparison_char_type)INT_FAST64_MIN)

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
 * Notes:
 *  * guaranteed to fit inside the same space allocated for a 64-bit pointer
 *  * lineterminator config errors don't matter for reader
 *  * this bitfield structure form was chosen as opposed to bitmasks because
 *    the author believe this leads to simpler analysis and code.
 */
typedef struct csv_return {
  uint64_t succeeded : 1;    /**< boolean to determine if an operation
                                was successful, if @c true then no other
                                fields need be checked. */
  uint64_t io_good : 1;      /**< indicates file being processed in a good
                                state. For @c FILE* targets, this indicates that
                                the end of file is not reached and no bad bit is
                                set. For other sources, this is
                                implementation-defined. @c false does not
                                indicate any state if the returning function
                                does not interact with any IO */
  uint64_t io_eof : 1;       /**< indicates EOF was reached,
                                 implementation-dependent, set by
                                 callbacks for reader and writer */
  uint64_t io_error : 1;     /**< indicates an IO error was encountered,
                                implementation-dependent, set by
                                callbacks for reader and writer */
  uint64_t truncated : 1;    /**< indicates a field was truncated for
                                either reading or writing */
  uint64_t dialect_null : 1; /**< indicates a CSV Dialect was passed as
                                null when one is required */
  uint64_t quoteescape_error : 1;
  uint64_t delimiter_error : 1;
} csvreturn;

/**
 * @brief CSV Return Initializer
 *
 * This initializer's return should be used wherever state information is needed
 * by the caller. There is space available for a significant number of states
 * without exceeding the size of a pointer.
 *
 * In general, the @c succeeded field is the only one which needs to be checked.
 * Other fields should be checked at runtime to determine if the state indicates
 * processing should be considered complete like @c csv_eof, or if there was a
 * fatal error like @c csv_io_error.
 *
 * @param  succeeded controls the @c succeeded field's initial state
 *
 * @return           CSV Return struct
 *
 * @see csvreturn
 */
#define csvreturn_init(succeeded) \
  ((csvreturn){((int)succeeded) == 0 ? 0 : 1, 0, 0, 0, 0, 0, 0, 0})

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
#define csv_success(retcode) ((bool)retcode.succeeded)

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
#define csv_failure(retcode) (!((bool)retcode.succeeded))

#endif /* CSV_DEFINITIONS_H_ */
