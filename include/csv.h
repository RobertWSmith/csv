/**
 * @file csv.h
 * @author Robert Smith
 * @date 2018-06-18
 * @brief Publicly accessible import header for CSV I/O library
 *
 * Import this header to ensure the entire CSV I/O library API is available
 * to your project.
 */

#ifndef CSV_H_
#define CSV_H_

/* library-wide definitions, version information, return codes, etc. */
#include "csv/definitions.h"
#include "csv/version.h"

/* I/O stream API */
#include "csv/stream.h"

/* configuration API for read & write */
#include "csv/dialect.h"

/* read & write API, respectively */
#include "csv/read.h"
#include "csv/write.h"

#endif /* CSV_H_ */
