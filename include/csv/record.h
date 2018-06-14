/**
 * @file csv/record.h
 * @author Robert Smith
 * @date 2018-06-18
 * @brief CSV record object
 */

#ifndef CSV_RECORD_H_
#define CSV_RECORD_H_

#include "definitions.h"
#include "field.h"

/**
 * @brief CSV Record object
 *
 * Maintains an array of CSV Records
 */
typedef struct csv_record *csvrecord;


/**
 * @brief CSV Record default initializer
 *
 * @return  Initialized CSV Record, with zero buffer
 *
 * @see csvrecord_buffer_init
 * @see csvrecord_close
 */
csvrecord csvrecord_init(void);

/**
 * @brief CSV Record buffered initializer
 *
 * @param[in]  buffer   Size of initial CSV Record buffer
 *
 * @return              Initialized CSV Record
 *
 * @see csvrecord_init
 * @see csvrecord_close
 */
csvrecord csvrecord_buffer_init(size_t buffer);

/**
 * @brief CSV Record destructor
 *
 * @param  record[in,out]   Reference to a CSV Record type, @p record will be
 *                          set to @c NULL after the function returns.
 *
 * @see csvrecord_init
 * @see csvrecord_buffer_init
 */
void      csvrecord_close(csvrecord *record);

/**
 * @brief Amount of unused space remaining in the CSV Record buffer
 *
 * @param[in]  record  CSV Record type
 *
 * @return            Length of available buffer
 *
 * @see csvrecord_capacity
 * @see csvrecord_length
 */
size_t    csvrecord_available(const csvrecord record);

/**
 * @brief Amount of used space in a CSV Record buffer
 *
 * @param[in]  record  CSV Record type
 *
 * @return            Length of consumed buffer
 *
 * @see csvrecord_available
 * @see csvrecord_capacity
 */
size_t    csvrecord_length(const csvrecord record);

/**
 * @brief Amount of space allocated in CSV Record buffer
 *
 * @param[in]  record  CSV Record type
 *
 * @return            Allocated buffer length
 *
 * @see csvrecord_available
 * @see csvrecord_length
 */
size_t    csvrecord_capacity(const csvrecord record);

/**
 * @brief Comparison between two CSV Records
 *
 * @param[in]  lhs CSV Record type
 * @param[in]  rhs CSV Record type
 *
 * @return      <0 : the first character which doesn't match is lower in @p lhs
 *               0 : the contents of both records are equal
 *              >0 : the first character which doesn't match is higher in @p lhs
 */
int       csvrecord_cmp(const csvrecord lhs,
                        const csvrecord rhs);

/**
 * @brief Fill a CSV Record buffer with a given character
 *
 * @param[in]  record  CSV Record type
 * @param[in]  char_  value to fill in @p record buffer
 *
 * @return            CSV Return type to determine if the operation was
 *                    successful
 */
csvreturn csvrecord_set(csvrecord record,
                        int       char_);

/**
 * @brief Copy the contents of one CSV Record to another
 *
 * @param[out]  dest   CSV Record type where the data is to be stored
 * @param[in]  source Source CSV Record type
 *
 * @return        CSV Return type to determine if the operation was successful
 */
csvreturn csvrecord_copy(csvrecord      *dest,
                         const csvrecord source);

/**
 * @brief Clear values in CSV Record
 *
 * @param[in]  record CSV Record type
 *
 * @return        CSV Return type to determine if the operation was successful
 */
csvreturn csvrecord_clear(csvrecord record);

/**
 * @brief Modify CSV Record buffer to ensure it is at least @p newsize
 *
 * @param[in]  record    CSV Record type
 * @param[in]  newsize  Minimum size of the buffer after function returns
 *
 * @return              CSV Return type to determine if the operation was
 *                      successful
 */
csvreturn csvrecord_reserve(csvrecord record,
                            size_t    newsize);

/**
 * @brief Shrink CSV Record allocated buffer to be equal to consumed size
 *
 * @param[in]  record  CSV Record type
 *
 * @return            CSV Return type to determine if the operation was
 *                    successful
 */
csvreturn csvrecord_shrink_to_fit(csvrecord record);

/**
 * @brief Append CSV Field to CSV Record
 *
 * @param[in] record  CSV Record type
 * @param[in] field   CSV Field to append
 *
 * @return            CSV Return type to determine if the operation was
 *                    successful
 */
csvreturn csvrecord_append(csvrecord record,
                           csvfield  field);

#endif /* CSV_RECORD_H_ */
