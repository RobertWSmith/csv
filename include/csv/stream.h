/**
 * @file csv/stream.h
 * @author Robert Smith
 * @date 2018-06-18
 * @brief CSV stream callback API
 */

#ifndef CSV_STREAM_H_
#define CSV_STREAM_H_

#include <stddef.h>

#include "definitions.h"
#include "version.h"

/**
 * @brief CSV Stream data container for callbacks
 *
 * This type represents an input or output datastream and is passed to the
 * advanced CSV Reader and Writer initializer.
 *
 * @see csvreader_advanced_init
 * @see csvwriter_advanced_init
 */
typedef void *csvstream_type;

/* reader and writer, optional shutdown method called within the closer */
typedef void (*csvstream_close)(csvstream_type streamdata);

/* reader only, get next character from stream */
typedef CSV_STREAM_SIGNAL (*csvstream_getnextchar)(
    csvstream_type streamdata, csv_comparison_char_type *value);

/* reader only, append character to existing field buffer */
typedef void (*csvstream_appendfield)(csvstream_type           streamdata,
                                      csv_comparison_char_type value);

/* push field back into record */
typedef void (*csvstream_savefield)(csvstream_type streamdata);

/* finalize record, and return it by reference */
typedef void (*csvstream_saverecord)(csvstream_type streamdata,
                                     char ***       fields,
                                     size_t *       length);

/*
 * Sets the provided record as active
 */
typedef void (*csvstream_setrecord)(csvstream_type streamdata,
                                    const char **  record,
                                    size_t         length);

/*
 * return the next field for the writer by reference, return the char type
 * this is the value prior to processing the dialect rules
 */
typedef CSV_STREAM_SIGNAL (*csvstream_setnextfield)(csvstream_type streamdata,
                                                    size_t *       length);

/*
 * write a char to the output stream, when end of record is identified the
 * string will be the line terminator sequence
 */
typedef void (*csvstream_writechar)(csvstream_type           streamdata,
                                    csv_comparison_char_type value);

/*
 * needed for 'QUOTE_STYLE_MINIMAL' to allow the iterator to rest to
 * the beginning of the field
 */
typedef void (*csvstream_resetfield)(csvstream_type streamdata);

#endif /* CSV_STREAM_H_ */
