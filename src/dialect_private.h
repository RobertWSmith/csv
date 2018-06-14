/**
 * @file dialect_private.h
 * @author Robert Smith
 * @date 2018-06-18
 * @brief Private API for CSV Dialect. No guarantee of stability.
 */
#ifndef CSV_DIALECT_PRIVATE_H_
#define CSV_DIALECT_PRIVATE_H_

#include <stdbool.h>
#include <stddef.h>

#include "csv/dialect.h"

/**
 * Deep copy of CSV Dialect
 *
 * @param[in]  dialect  CSV Dialect type to copy
 *
 * @return              Deep copy of @p dialect values, note: this must be freed
 *                      by the requestor.
 *
 * @see csvdialect_init
 * @see csvdialect_close
 */
csvdialect csvdialect_copy(csvdialect dialect);


/**
 * @brief Ensure CSV Dialect is in a valid state
 *
 * @param[in]  dialect  CSV Dialect type
 *
 * @return              CSV Return type to determine if the operation was
 *                      successful
 */
csvreturn csvdialect_validate(csvdialect dialect);


/**
 * @brief Get CSV Dialect delimiter
 *
 * @param[in]  dialect  CSV Dialect to query
 *
 * @return              Either a character representing the delimiter, or
 *                      CSV_UNDEFINED_CHAR.
 *
 * @see csvdialect_set_delimiter
 */
char  csvdialect_get_delimiter(csvdialect dialect);

/**
 * @brief Get CSV Dialect double quoting configuration
 *
 * @param[in] dialect     CSV Dialect type
 *
 * @return                double quoting configuration
 *
 * @see csvdialect_set_doublequote
 */
bool  csvdialect_get_doublequote(csvdialect dialect);

/**
 * @brief Get CSV Dialect Escape character
 *
 * @param  dialect[in]  CSV Dialect type
 *
 * @return              escape character
 *
 * @see csvdialect_set_escapechar
 */
char  csvdialect_get_escapechar(csvdialect dialect);

/**
 * @brief Get CSV Dialect Line Terminator string
 *
 * @param[in]   dialect CSV Dialect type
 * @param[out]  length  Reference to pass the length of the output string
 *
 * @return              CSV Lineterminator string
 *
 * @see csvdialect_set_lineterminator
 */
char* csvdialect_get_lineterminator(csvdialect dialect,
                                    size_t    *length);

/**
 * @brief Get CSV Dialect Quoting Character
 *
 * @param[in]  dialect  CSV Dialect type
 *
 * @return              CSV quoting character
 *
 * @see csvdialect_set_quotechar
 */
char        csvdialect_get_quotechar(csvdialect dialect);

/**
 * @brief Get CSV Dialect Quoting Style
 *
 * @param[in]  dialect  CSV Dialect type
 *
 * @return              CSV quoting style configuration
 *
 * @see csvdialect_set_quotestyle
 */
QUOTE_STYLE csvdialect_get_quotestyle(csvdialect dialect);

/**
 * @brief Get CSV Dialect Skip Initial Space configuration
 *
 * @param[in]  dialect  CSV Dialect type
 *
 * @return              skip initial space configuration
 *
 * @see csvdialect_set_skipinitialspace
 */
bool        csvdialect_get_skipinitialspace(csvdialect dialect);

#endif /* CSV_DIALECT_PRIVATE_H_ */
