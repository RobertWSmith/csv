/**
 * @file csv/read.h
 * @author Robert Smith
 * @date 2018-06-18
 * @brief CSV Reader API
 */

#ifndef CSV_READ_H_
#define CSV_READ_H_

#include <stddef.h>
#include <stdio.h>

#include "definitions.h"
#include "dialect.h"
#include "stream.h"
#include "version.h"

/**
 * @brief CSV Reader datastructure
 *
 * Manages all the information required to read from and parse a CSV file.
 */
typedef struct csv_reader *csvreader;

/**
 * @brief CSV Reader initializer from filepath
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
 * @see csvreader_file_init
 * @see csvreader_close
 * @see csvreader_next_record
 */
csvreader csvreader_init(csvdialect dialect, const char *filepath);

/**
 * @brief CSV Reader initializer from file object
 *
 * Create a new CSV Reader from an existing file object.
 *
 * @param[in]  dialect  CSV dialect type.
 * @param[in]  fileobj  File object for input stream
 *
 * @return              Fully initialized CSV Reader, or NULL on error
 *
 * @see csvdialect_init
 * @see csvdialect_advanced_init
 * @see csvreader_close
 * @see csvreader_next_record
 */
csvreader csvreader_file_init(csvdialect dialect, FILE *fileobj);

/**
 * @brief Advanced Initializer for CSV Reader
 *
 * Allows customization of data sources and decoding prior to CSV parser
 * application.
 *
 * @param[in]  dialect        preconfigured CSV Dialect type
 * @param[in]  getnextchar    Function pointer which returns the next character
 *                            in the stream buffer.
 * @param[in]  appendfield    Function pointer which appends character to
 *                            current field buffer.
 * @param[in]  savefield      Function pointer which pushes the current field to
 *                            the end of the record buffer and resets the field
 *                            buffer to the beginning.
 * @param[in]  saverecord     Function pointer which completes a record and
 *                            returns it by reference.
 * @param[in]  streamdata     Datastructure which is passed to @p stream_getnext
 *
 * @return                    initialized CSV Reader
 *
 * @see csvreader_advanced_init
 * @see csvstream_type
 * @see csvstream_open
 * @see csvstream_getnext
 * @see csvstream_close
 */
csvreader csvreader_advanced_init(csvdialect            dialect,
                                  csvstream_getnextchar getnextchar,
                                  csvstream_appendfield appendfield,
                                  csvstream_savefield   savefield,
                                  csvstream_saverecord  saverecord,
                                  csvstream_type        streamdata);

/**
 * @brief Set CSV stream closer
 *
 * @param[in]  reader        CSV reader type
 * @param[in]  closer        Function pointer which accepts the @c streamdata
 *                           and uses this to properly close the stream. This
 *                           function is called at the very beginning of the
 *                           @c csvreader_close function.
 *
 * @return                   initialized CSV Reader
 *
 * @see csvreader_advanced_init
 * @see csvstream_type
 * @see csvstream_close
 */
csvreader csvreader_set_closer(csvreader reader, csvstream_close closer);

/**
 * @brief CSV Reader destructor
 *
 * Cleans up the @p reader and sets @p reader to @c NULL after the function
 * returns. Safe to pass @c NULL or previously closed @c csvreader as it does
 * not attempt to free @c NULL pointers.
 *
 * If a closing function pointer was passed to @c csvreader_set_closer then this
 * will be called prior to freeing the CSV reader resources.
 *
 * @param[in,out]  reader   CSV reader type
 *
 * @see csvreader_init
 * @see csvreader_advanced_init
 * @see csvreader_set_closer
 */
void csvreader_close(csvreader *reader);

/**
 * @brief Get next CSV Record from CSV Reader's file
 *
 * @param[in]   reader        CSV Reader type
 * @param[out]  record        Reference to a CSV Record type, if @c NULL a new
 *                            @c csvreader is allocated.
 * @param[out]  record_length Number of fields stored in @p record
 *
 * @return                    CSV Return type to determine if the operation was
 *                            successful
 */
csvreturn csvreader_next_record(csvreader reader,
                                char ***  record,
                                size_t *  record_length);

#endif /* CSV_READ_H_ */
