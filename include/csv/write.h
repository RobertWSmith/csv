#ifndef CSV_WRITE_H_
#define CSV_WRITE_H_

#include <stdio.h>

#include "definitions.h"
#include "dialect.h"
#include "stream.h"

/**
 * @brief CSV Writer datastructure
 *
 * Manages all the information required to format and write to a CSV file.
 */
typedef struct csv_writer *csvwriter;


/**
 * @brief CSV Writer initializer
 *
 * Create a new CSV Writer.
 *
 * @p dialect is deep-copied upon initialization. If @p dialect is @c NULL, the
 * initializer will use the default @c csvdialect returned by
 * @c csvdialect_init. If the @c csvdialect is in an invalid state, the
 * @c csvwriter returned will be @c NULL.
 *
 * If @p filepath cannot be opened with @c fopen the return value for
 * @c csvwriter_init will be @c NULL.
 *
 * If any other issue not noted is encountered during the call to
 * @c csvwriter_init the @c csvwriter returned will be @c NULL.
 *
 * @param[in]  dialect  CSV dialect type.
 * @param[in]  filepath Filepath to input CSV
 *
 * @return              Fully initialized CSV Writer, or @c NULL on error
 *
 * @see csvdialect_init
 * @see csvwriter_file_init
 * @see csvwriter_close
 * @see csvwriter_next_record
 */
csvwriter csvwriter_init(csvdialect  dialect,
                         const char *filepath);

/**
 * @brief CSV Writer initializer from File object
 *
 * @param  dialect CSV Dialect type
 * @param  fileobj File object, if @c NULL a @c NULL writer will be returned
 *
 * @return         Filly initialized CSV Writer, or @c NULL on error
 *
 * @see csvdialect_init
 * @see csvwriter_init
 * @see csvwriter_close
 * @see csvwriter_next_record
 */
csvwriter csvwriter_file_init(csvdialect dialect,
                              FILE      *fileobj);

/**
 * [csvwriter_advanced_init description]
 *
 * @param  dialect      [description]
 * @param  getnextfield [description]
 * @param  writestring  [description]
 * @param  streamdata   [description]
 *
 * @return              [description]
 */
csvwriter csvwriter_advanced_init(csvdialect             dialect,
                                  csvstream_getnextfield getnextfield,
                                  csvstream_writestring  writestring,
                                  csvstream_type         streamdata);

/**
 * [csvwriter_set_closer description]
 *
 * @param  writer [description]
 * @param  closer [description]
 *
 * @return        [description]
 */
csvwriter csvwriter_set_closer(csvwriter       writer,
                               csvstream_close closer)

/*
 * maybe there isn't an 'advanced' API?
 *
 * csvwrite csvwriter_advanced_init(csvdialect dialect);
 */

/**
 * @brief CSV Writer destructor
 *
 * Cleans up the @p writer and sets @p writer to @c NULL after the function
 * returns. Safe to pass @c NULL or previously closed @c csvwriter as it does
 * not attempt to free @c NULL pointers.
 *
 * @param[in,out]  writer   CSV writer type
 *
 * @see csvwriter_init
 */
void      csvwriter_close(csvwriter *writer);

/**
 * @brief Set next CSV Record to CSV Writer's file
 *
 * @param[in]   writer        CSV Writer type
 * @param[in]   record        CSV Record type
 * @param[in]   record_length Number of fields stored in @p record
 *
 * @return              CSV Return type to determine if the operation was
 *                      successful
 */
csvreturn csvwriter_next_record(csvwriter            writer,
                                const csvrecord_type record,
                                size_t               record_length);


#endif /* CSV_WRITE_H_ */
