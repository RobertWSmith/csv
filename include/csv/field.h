/**
 * @file csv/field.h
 * @author Robert Smith
 * @date 2018-06-18
 * @brief CSV field object
 */

#ifndef CSV_FIELD_H_
#define CSV_FIELD_H_

#include <stddef.h>

#include "definitions.h"

/**
 * @brief CSV Field object
 *
 * Acts as an I/O buffer and represents a single value in a CSV Record / Row
 */
typedef struct csv_field *csvfield;

/**
 * @brief CSV Field default initializer
 *
 * @return  Initialized CSV Field, with zero buffer
 *
 * @see csvfield_buffer_init
 * @see csvfield_close
 */
csvfield csvfield_init(void);

/**
 * @brief CSV Field buffered initializer
 *
 * @param[in]  buffer   Size of initial CSV Field buffer
 *
 * @return              Initialized CSV Field
 *
 * @see csvfield_init
 * @see csvfield_close
 */
csvfield csvfield_buffer_init(size_t buffer);

/**
 * @brief CSV Field destructor
 *
 * @param  field[in,out]  Reference to a CSV Field type, @p field will be set to
 *                        @c NULL after the function returns.
 *
 * @see csvfield_init
 * @see csvfield_buffer_init
 */
void     csvfield_close(csvfield *field);

/**
 * @brief Amount of unused space remaining in the CSV Field buffer
 *
 * @param[in]  field  CSV Field type
 *
 * @return            Length of available buffer
 *
 * @see csvfield_capacity
 * @see csvfield_length
 */
size_t   csvfield_available(const csvfield field);

/**
 * @brief Amount of used space in a CSV Field buffer
 *
 * @param[in]  field  CSV Field type
 *
 * @return            Length of consumed buffer
 *
 * @see csvfield_available
 * @see csvfield_capacity
 */
size_t   csvfield_length(const csvfield field);

/**
 * @brief Amount of space allocated in CSV Field buffer
 *
 * @param[in]  field  CSV Field type
 *
 * @return            Allocated buffer length
 *
 * @see csvfield_available
 * @see csvfield_length
 */
size_t   csvfield_capacity(const csvfield field);

/**
 * @brief Comparison between two CSV Fields
 *
 * @param[in]  lhs CSV Field type
 * @param[in]  rhs CSV Field type
 *
 * @return      <0 : the first character which doesn't match is lower in @p lhs
 *               0 : the contents of both fields are equal
 *              >0 : the first character which doesn't match is higher in @p lhs
 */
int      csvfield_cmp(const csvfield lhs,
                      const csvfield rhs);

/**
 * @brief Fill a CSV Field buffer with a given character
 *
 * @param[in]  field  CSV Field type
 * @param[in]  char_  value to fill in @p field buffer
 *
 * @return            CSV Return type to determine if the operation was
 *                    successful
 */
csvreturn csvfield_set(csvfield field,
                       int      char_);

/**
 * @brief Copy the contents of one CSV Field to another
 *
 * @param[out]  dest   CSV Field type where the data is to be stored
 * @param[in]  source Source CSV Field type
 *
 * @return        CSV Return type to determine if the operation was successful
 */
csvreturn csvfield_copy(csvfield      *dest,
                        const csvfield source);

/**
 * @brief Clear values in CSV Field
 *
 * @param[in]  field CSV Field type
 *
 * @return        CSV Return type to determine if the operation was successful
 */
csvreturn csvfield_clear(csvfield field);

/**
 * @brief Modify CSV Field buffer to ensure it is at least @p newsize
 *
 * @param[in]  field    CSV Field type
 * @param[in]  newsize  Minimum size of the buffer after function returns
 *
 * @return              CSV Return type to determine if the operation was
 *                      successful
 */
csvreturn csvfield_reserve(csvfield field,
                           size_t   newsize);

/**
 * @brief Shrink CSV Field allocated buffer to be equal to consumed size
 *
 * @param[in]  field  CSV Field type
 *
 * @return            CSV Return type to determine if the operation was
 *                    successful
 */
csvreturn csvfield_shrink_to_fit(csvfield field);

/**
 * @brief Append character to CSV Field
 *
 * @param[in] field   CSV Field type
 * @param[in] char_   Character to append
 *
 * @return            CSV Return type to determine if the operation was
 *                    successful
 */
csvreturn csvfield_append(csvfield field,
                          int      char_);

#endif /* CSV_FIELD_H_ */
