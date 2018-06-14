#ifndef CSV_WRITE_H_
#define CSV_WRITE_H_

#include "definitions.h"
#include "dialect.h"
#include "record.h"

#include "dialect_private.h"

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
 * @return              Fully initialized CSV Writer, or NULL on error
 *
 * @see csvdialect_init
 * @see csvwriter_close
 * @see csvwriter_next_record
 */
csvwriter csvwriter_init(csvdialect  dialect,
                         const char *filepath);

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
 * @param[in]   writer  CSV Writer type
 * @param[out]  record  CSV Record type
 *
 * @return              CSV Return type to determine if the operation was
 *                      successful
 */
csvreturn csvwriter_next_record(csvwriter       writer,
                                const csvrecord record);


#endif /* CSV_WRITE_H_ */
