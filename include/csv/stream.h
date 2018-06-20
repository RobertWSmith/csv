/**
 * @file csv/stream.h
 * @author Robert Smith
 * @date 2018-06-18
 * @brief CSV stream callback API
 */

#ifndef CSV_STREAM_H_
#define CSV_STREAM_H_

#include <uchar.h>

typedef enum CSV_CHAR_TYPE {
  CSV_CHAR8,
  CSV_CHAR16,
  CSV_CHAR32,
} CSV_CHAR_TYPE;


typedef enum CSV_STREAM_SIGNAL {
  CSV_GOOD,
  CSV_EOF,
  CSV_ERROR,
} CSV_STREAM_SIGNAL;


/**
 * @brief CSV Stream data container for callbacks
 *
 * This type represents an input or output datastream and is passed to the
 * advanced CSV Reader and Writer initializer.
 *
 * @see csvreader_advanced_init
 * @see csvwriter_advanced_init
 * @see csvfield_type
 * @see csvrecord_type
 */
typedef void *csvstream_type;

/**
 * @brief CSV field type, represents a container for a unique row-column value
 *
 * This type is a string analog, and is defined as a generic pointer to enable
 * various string widths and/or custom structs.
 *
 * A call to @c csvstream_savefield should add a null character to the end of
 * the current CSV field, set the current field at the end of the CSV Record and
 * set the CSV field buffer to the beginning.
 *
 * @see csvstream_savefield
 */
typedef void *csvfield_type;

/**
 * @brief CSV Record type, represents a container of CSV Fields
 *
 * This type should be considered as most closely aligned with a single line of
 * a CSV, or a single record from a SQL database query. This might be defined
 * as simply as @c csvfield_type* or could be a custom struct.
 */
typedef void *csvrecord_type;


/* reader and writer, optional shutdown method called within the closer */
typedef void (*csvstream_close)(csvstream_type streamdata);

/* reader only, get next character from stream */
typedef CSV_STREAM_SIGNAL (*csvstream_getnextchar)(csvstream_type streamdata,
                                                   char32_t      *value);

/* reader only, append character to existing field buffer */
typedef void (*csvstream_appendfield)(csvstream_type streamdata,
                                      char32_t       value);

/* push field back into record */
typedef void (*csvstream_savefield)(csvstream_type streamdata);

/* finalize record, and return it by reference */
typedef CSV_CHAR_TYPE (*csvstream_saverecord)(csvstream_type  streamdata,
                                              csvrecord_type *fields,
                                              size_t         *length);

/*
 * return the next field for the writer by reference, return the char type
 * this is the value prior to processing the dialect rules
 */
typedef CSV_CHAR_TYPE (*csvstream_getnextfield)(csvstream_type streamdata,
                                                csvrecord_type record,
                                                size_t         record_length,
                                                csvfield_type *field,
                                                size_t        *field_length);

/*
 * write a string to the output stream, when end of record is identified the
 * string will be the line terminator sequence
 */
typedef void (*csvstream_writestring)(csvstream_type streamdata,
                                      CSV_CHAR_TYPE  char_type,
                                      csvfield_type  field,
                                      size_t         length);

#endif /* CSV_STREAM_H_ */
