/**
 * @file csv/dialect.h
 * @author Robert Smith
 * @date 2018-06-18
 * @brief CSV library configuration object for both reading and writing
 */

#ifndef CSV_DIALECT_H_
#define CSV_DIALECT_H_

#include <stddef.h>
#include <stdbool.h>

#include "definitions.h"

/**
 * @brief CSV Quoting Style
 *
 * Defines the CSV input or output quoting style.
 * @c QUOTE_STYLE_MINIMAL indicates that a field is quoted if it includes any
 *                        character defined as a delimiter, line terminator,
 *                        quoting character, escape character, null character,
 *                        or other special character such as unprintable
 *                        characters
 * @c QUOTE_STYLE_NONE    indicates no fields are quoted, if any of the special
 *                        characters defined by @c QUOTE_STYLE_MINIMAL are
 *                        present in a field, the value set as the @c escapechar
 *                        is printed prior to the special character. If
 *                        @c escapechar is not set prior to passing the
 *                        @c csvdialect to @c csvreader_init a @c csvreturn
 *                        indicating an invalid dialect state will be returned.
 * @c QUOTE_STYLE_ALL     indicates all fields are surrounded by the
 *                        @c quotechar value. If the @c quotechar value is
 *                        present in a field, the value of @c doublequote is
 *                        used to determine if this character will be doubled
 *                        or if the value of @c escapechar should be expected
 *                        prior to the @c quotechar in order for the field to
 *                        be valid.
 */
typedef enum QUOTE_STYLE {
  QUOTE_STYLE_MINIMAL = 0,
  QUOTE_STYLE_NONE,
  QUOTE_STYLE_ALL
} QUOTE_STYLE;

/**
 * @brief CSV Dialect type which configures CSV reader and writer objects
 */
typedef struct csv_dialect *csvdialect;

/**
 * @brief CSV Dialect default initializer
 *
 * Please see setter function documentation for information on the fields and
 * their default values.
 *
 * @return  Fully initialized @csvdialect using the defaults documented in the
 *          relevant setter functions, or NULL if an error occurred.
 *
 * @see csvdialect_close
 * @see csvdialect_set_delimiter
 * @see csvdialect_set_doublequote
 * @see csvdialect_set_escapechar
 * @see csvdialect_set_lineterminator
 * @see csvdialect_set_quotechar
 * @see csvdialect_set_quotestyle
 * @see csvdialect_set_skipinitialspace
 */
csvdialect csvdialect_init(void);

/**
 * @brief CSV Dialect destructor
 *
 * After the function call has completed @p dialect will point to @c NULL. It is
 * safe to call this function with a previously destroyed dialect as it does
 * not try to free null pointers.
 *
 * @param[in,out]  dialect  Reference to @c csvdialect.
 *
 * @see csvdialect_init
 */
void       csvdialect_close(csvdialect *dialect);

/**
 * @brief Set CSV Dialect delimiter character
 *
 *
 * The @c dialect field defaults to ',' (comma character). Calling this function
 * allows you to apply any single character as the configured delimiter and
 * directly controls the character the @c csvreader looks for to parse a CSV or
 * the @c csvwriter uses to join fields from a record into a line.
 *
 * @param[in]  dialect    CSV Dialect type
 * @param[in]  delimiter  delimiter character for configuration
 *
 * @return                CSV Return type to determine if the operation was
 *                        successful
 *
 * @see csvdialect_init
 * @see csvdialect_close
 */
csvreturn  csvdialect_set_delimiter(csvdialect dialect,
                                    char       delimiter);

/**
 * @brief Set CSV Dialect quote character escape configuration
 *
 * Default value is @c true.
 *
 * @c true which indicates that two consecutive quoting characters in a field
 * represent a single quoting character that is part of the text of the field.
 * @c false indicates that a @c quotechar which should be considered part of
 * the text of a field only if preceded by @c escapechar. If @c false, the CSV
 * Dialect is in an invalid state if @escapechar is set to
 * @c CSV_UNDEFINED_CHAR.
 *
 * Example 1) Assuming CSV Dialect is set to the defaults.
 * Input line:
 *  1,2,"abc,123",Mr. T ""Pities the fool""
 *
 * Output by field:
 *  * 1
 *  * 2
 *  * abc,123
 *  * Mr. T "Pities the fool"
 *
 * Example 2) Assuming CSV Dialect is set to the defaults, save for the
 * following:
 * * @c doublequote is set to false
 * * @c escapechar is set to '\\'
 *
 * Input line:
 *  1,2,"abc,123",Mr. T \"Pities the fool\"
 *
 * Output by field:
 *  * 1
 *  * 2
 *  * abc,123
 *  * Mr. T "Pities the fool"
 *
 * @param[in]  dialect      CSV Dialect type
 * @param[in]  doublequote  boolean to configure
 *
 * @return                  CSV Return type to determine if the operation was
 *                          successful
 *
 * @see csvdialect_init
 * @see csvdialect_close
 */
csvreturn csvdialect_set_doublequote(csvdialect dialect,
                                     bool       doublequote);

/**
 * @brief Set CSV Dialect escape character
 *
 * Defaults setting is CSV_UNDEFINED_CHAR.
 *
 * If the default value is not changed, the CSV reader assumes no values need to
 * be escaped as all special characters are inside quoted fields. @c doublequote
 * must be @c true if the @escapechar remains with its default value.
 *
 * This character controls how special characters are represented inside a field
 * especially, if the field is not surrounded by quoting characters.
 *
 * @param[in]  dialect    CSV Dialect type
 * @param[in]  escapechar Escape character for configuration
 *
 * @return                CSV Return type to determine if the operation was
 *                        successful
 *
 * @see csvdialect_init
 * @see csvdialect_close
 * @see csvdialect_set_doublequote
 */
csvreturn csvdialect_set_escapechar(csvdialect dialect,
                                    char       escapechar);

/**
 * @brief Set CSV Dialect Lineterminator sequence
 *
 * Default setting is "\n"
 *
 * TODO: make this default equal to the platform newline sequence.
 *
 * Please note, for reading CSV files, this setting is not used. All unescaped
 * '\r' and '\n' characters are considered newlines. This value should only be
 * considered for writing CSVs where the newline character matters for a
 * system which will be consuming the CSV output in the future.
 *
 * Also note that the @p lineterminator length is capped at either the value
 * passed by @p length, the value configured by @c CSV_LINETERMINATOR_MAX
 * at compile time or the first null character encountered in the string.
 * Whichever value is shortest. If the string ends up having a length of zero,
 * the CSV Dialect will be in an invalid state.
 *
 * Finally, if this value is set it is deep-copied at function call time,
 * therefore any changes to the string will not affect the CSV dialect.
 *
 * @param[in]  dialect        CSV Dialect type
 * @param[in]  lineterminator Lineterminator string to apply
 * @param[in]  length         Length of the @p lineterminator string. If set to
 *                            zero the value will be populated with a call to
 *                            @c strlen.
 *
 * @return                    CSV Return type to determine if the operation was
 *                            successful
 *
 * @see csvdialect_init
 * @see csvdialect_close
 * @see CSV_LINETERMINATOR_MAX
 */
csvreturn csvdialect_set_lineterminator(csvdialect  dialect,
                                        const char *lineterminator,
                                        size_t      length);

/**
 * @brief Set CSV Dialect Quoting Character
 *
 * Default setting is '"' (doublequote character)
 *
 * Controls the character which represents a quoted field.
 *
 * @param[in]  dialect   CSV Dialect type
 * @param[in]  quotechar Quoting character for the CSV file
 *
 * @return              CSV Return type to determine if the operation was
 *                      successful
 *
 * @see csvdialect_init
 * @see csvdialect_close
 */
csvreturn csvdialect_set_quotechar(csvdialect dialect,
                                   char       quotechar);

/**
 * @brief Set CSV Dialect Quoting Style
 *
 * Default setting is @c QUOTE_STYLE_MINIMAL.
 *
 * Controls the CSV Quoting style configuration.
 *
 * @param[in]  dialect    CSV Dialect type
 * @param[in]  quotestyle Quoting style configuration
 *
 * @return                CSV Return type to determine if the operation was
 *                        successful
 *
 * @see csvdialect_init
 * @see csvdialect_close
 * @see QUOTE_STYLE
 */
csvreturn csvdialect_set_quotestyle(csvdialect  dialect,
                                    QUOTE_STYLE quotestyle);

/**
 * @brief Set CSV Skip Initial Space configuration
 *
 * Default setting is @c false
 *
 * If @c true, the CSV reader will discard any whitespace encountered between
 * the @c delimiter character and beginning of the next field. If @c false
 * all characters after the @c delimiter is encountered (respecting the quoting
 * configuration) is contained as part of the next field.
 *
 * @param[in]  dialect          CSV Dialect type
 * @param[in]  skipinitialspace boolean configuration to control if initial
 *                              spaces are dropped or kept as part of a field.
 *
 * @return                      CSV Return type to determine if the operation
 *                              was successful
 *
 * @see csvdialect_init
 * @see csvdialect_close
 */
csvreturn csvdialect_set_skipinitialspace(csvdialect dialect,
                                          bool       skipinitialspace);


#endif /* CSV_DIALECT_H_ */
