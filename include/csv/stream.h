#ifndef CSV_STREAM_H_
#define CSV_STREAM_H_

#include <uchar.h>

typedef enum CSV_CHAR_TYPE {
	CSV_CHAR8 = 0,
	CSV_CHAR16,
	CSV_CHAR32
} CSV_CHAR_TYPE;

/**
 * @brief CSV Stream data container for callbacks
 */
typedef void* csvstream_type;

/**
 * @brief Get next character point from CSV Stream buffer
 *
 * @param[in,out] streamdata  reference to data returned by @c csvstream_open
 *                            and potentially modified by this function. This
 *                            function is responsible for efficiently managing
 *                            the buffer.
 *
 * @return                    next character in the stream
 *
 * @see csvreader_advanced_init
 * @see csvstream_type
 * @see csvstream_open
 * @see csvstream_savefield
 * @see csvstream_close
 */
typedef char32_t (*csvstream_getnext)(csvstream_type streamdata) csvstream_getnext;

/**
 * @brief Finalize field and reset buffer for next field
 *
 * @param[in,out] streamdata  CSV datastructure
 * @param[out]    field       string output data, char type determined by
 *                            callback
 * @param[out]    length      length of @p field
 *
 * @see csvreader_advanced_init
 * @see csvstream_type
 * @see csvstream_open
 * @see csvstream_getnext
 * @see csvstream_close
 */
typedef void (*csvstream_savefield)(csvstream_type streamdata, void** field, size_t* length) csvstream_savefield;

/**
 * @brief Put next character point from CSV Stream buffer
 *
 * @param[in,out] streamdata  reference to data returned by @c csvstream_open
 *                            and potentially modified by this function. This
 *                            function is responsible for efficiently managing
 *                            the buffer.
 * @param[in]     value       Character to write to the stream.
 *
 * @return                    next character in the stream
 *
 * @see csvreader_advanced_init
 * @see csvstream_type
 * @see csvstream_open
 * @see csvstream_close
 */
typedef void (*csvstream_putnext)(csvstream_type streamdata, char32_t value) csvstream_putnext;

/**
 * @brief Get next character point from CSV Stream buffer
 *
 * @param[in,out] streamdata  reference to data returned by @c csvstream_open
 *                            and potentially modified by this function. This
 *                            function is responsible for efficiently managing
 *                            the buffer.
 *
 * @return                    next character in the stream
 *
 * @see csvreader_advanced_init
 * @see csvstream_type
 * @see csvstream_open
 * @see csvstream_close
 */
typedef char32_t (*csvstream_getnext)(csvstream_type streamdata) csvstream_getnext;

/**
 * @brief Close the CSV stream
 *
 * @param[in] streamdata    Data returned from @c csvstream_open, implements the
 *                          proper resource closing procedure.
 *
 * @see csvreader_advanced_init
 * @see csvstream_type
 * @see csvstream_open
 * @see csvstream_getnext
 */
typedef void (*csvstream_close)(csvstream_type streamdata) csvstream_close;


#endif  /* CSV_STREAM_H_ */
