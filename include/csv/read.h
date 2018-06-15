/**
 * @file csv/read.h
 * @author Robert Smith
 * @date 2018-06-18
 * @brief CSV Reader API
 */

#ifndef CSV_READ_H_
#define CSV_READ_H_

#include "definitions.h"
#include "dialect.h"
#include "record.h"

/**
 * @brief CSV Reader datastructure
 *
 * Manages all the information required to read from and parse a CSV file.
 */
typedef struct csv_reader *csvreader;

/**
 * @brief CSV Reader initializer
 *
 * Create a new CSV Reader.
 *
 * @p dialect is deep-copied upon initialization. If @p dialect is @c NULL, the
 * initializer will use the default @c csvdialect returned by
 * @c csvdialect_init. If the @c csvdialect is in an invalid state, the
 * @c csvreader returned will be @c NULL.
 *
 * If @p filepath cannot be opened with @c fopen the return value for
 * @c csvreader_init will be @c NULL.
 *
 * If any other issue not noted is encountered during the call to
 * @c csvreader_init the @c csvreader returned will be @c NULL.
 *
 * @param[in]  dialect  CSV dialect type.
 * @param[in]  filepath Filepath to input CSV
 *
 * @return              Fully initialized CSV Reader, or NULL on error
 *
 * @see csvdialect_init
 * @see csvreader_close
 * @see csvreader_next_record
 */
csvreader csvreader_init(csvdialect dialect,
                         const char *filepath);

/**
 * @brief CSV Reader destructor
 *
 * Cleans up the @p reader and sets @p reader to @c NULL after the function
 * returns. Safe to pass @c NULL or previously closed @c csvreader as it does
 * not attempt to free @c NULL pointers.
 *
 * @param[in,out]  reader   CSV reader type
 *
 * @see csvreader_init
 */
void csvreader_close(csvreader *reader);

/**
 * @brief Get next CSV Record from CSV Reader's file
 *
 * @param[in]   reader  CSV Reader type
 * @param[out]  record  Reference to a CSV Record type, if @c NULL a new
 *                      @c csvreader is allocated.
 *
 * @return              CSV Return type to determine if the operation was
 *                      successful
 */
csvreturn csvreader_next_record(csvreader reader,
                                csvrecord *record);

#endif  /* CSV_READ_H_ */
