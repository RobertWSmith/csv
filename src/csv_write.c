#include <stdio.h>
#include <stdlib.h>

#include "csv.h"

struct csv_writer {
  csvdialect             dialect;
  csvstream_type         streamdata;
  csvstream_close        closer;
  csvstream_getnextfield getnextfield;
  csvstream_writestring  writestring;
};

csvwriter _csvwriter_init(csvdialect dialect) {
  csvwriter writer = NULL;

  if ((writer = malloc(sizeof *writer)) == NULL) {
    // couldn't initialize
    return NULL;
  }

  writer->dialect = dialect;

  if (dialect == NULL) {
    writer->dialect = csvdialect_init();
  }
  writer->streamdata   = NULL;
  writer->closer       = NULL;
  writer->getnextfield = NULL;
  writer->writestring  = NULL;

  return writer;
}

typedef struct csv_file_writer {
  const char *filepath;
  FILE       *file;
  size_t      field_position;
} *csvfilewriter;

csvfilewriter _csvfilewriter_init(void) {
  csvfilewriter fw = NULL;

  if ((fw = malloc(sizeof *fw)) == NULL) {
    return NULL;
  }
  fw->filepath       = NULL;
  fw->file           = NULL;
  fw->field_position = 0;
  return fw;
}

CSV_CHAR_TYPE csv_filepath_getnextfield(csvstream_type streamdata,
                                        csvrecord_type record,
                                        size_t         record_length,
                                        csvfield_type *field,
                                        size_t        *field_length) {
  if ((streamdata == NULL) || (record == NULL)) {
    *field        = NULL;
    *field_length = 0;
    return CSV_CHAR8;
  }

  csvfilewriter fw = (csvfilewriter)streamdata;

  if ((fw->field_position + 1) >= record_length) {
    // TODO: need to figure out what I could use as a end of record indicator
    fw->field_position = 0;
    *field             = NULL;
    *field_length      = 0;
    return CSV_CHAR8;
  }

  *field        = record[fw->field_position++];
  *field_length = strlen(*field);
  return CSV_CHAR8;
}

void csv_filepath_writestring(csvstream_type      streamdata,
                              CSV_CHAR_TYPE       char_type,
                              const csvfield_type field,
                              size_t              field_length) {
  if ((streamdata == NULL) || (field == NULL)) return;

  int char_, rc;

  csvfilewriter fw = (csvfilewriter)streamdata;
  char *value      = (char *)field;

  for (size_t i = 0; i < field_length; ++i) {
    char_ = (int)value[i];
    rc    = fputc(char_, fw->file);

    if (rc != char_) {
      if (ferror(fw->file)) {
        perror("fputc()\n");
        fprintf(stderr,
                "Encountered error in file %s at line #%d\n",
                __FILE__,
                __LINE__);
        exit(EXIT_FAILURE);
      }
    }
  }
}

void csv_write_filepath_closer(csvstream_type streamdata) {
  if (streamdata == NULL) return;

  csvfilewriter fw = (csvfilewriter)streamdata;

  if (fw->file != NULL) {
    fclose(fw->file);
  }
  free(fw);
}

csvwriter csvwriter_init(csvdialect  dialect,
                         const char *filepath) {
  csvwriter writer = NULL;
  csvfilewriter fw = NULL;
  FILE *f          = NULL;

  if ((f = fopen(filepath, "wb")) == NULL) {
    // short circuit if the file couldn't be opened
    return NULL;
  }

  if ((fw = _csvfilewriter_init()) == NULL) {
    // finalize if memory cannot be allocated
    fclose(f);
    return NULL;
  }

  fw->filepath = filepath;
  fw->file     = f;

  writer =
    csvwriter_advanced_init(dialect,
                            &csv_filepath_getnextfield,
                            &csv_filepath_writestring,
                            (csvstream_type)fw);

  writer = csvwriter_set_closer(writer, &csv_write_filepath_closer);

  if (writer == NULL) {
    fclose(f);
    free(fw);
    return NULL;
  }
  return writer;
}

void csvwriter_close(csvwriter *writer) {
  csvwriter w = *writer;

  if (w == NULL) return;

  if (w->dialect != NULL) csvdialect_close(&(w->dialect));

  if (w->file != NULL) fclose(w->file);

  free(w);
  *writer = NULL;
}

csvwriter csvwriter_advanced_init(csvdialect             dialect,
                                  csvstream_getnextfield getnextfield,
                                  csvstream_writestring  writestring,
                                  csvstream_type         streamdata) {
  csvwriter writer = NULL;

  if ((writer = _csvwriter_init(dialect)) == NULL) {
    return NULL;
  }

  writer->getnextfield = getnextfield;
  writer->writestring  = writestring;
  writer->streamdata   = streamdata;

  return writer;
}

csvwriter csvwriter_set_closer(csvwriter writer, csvstream_close closer) {
  if (writer == NULL) return NULL;

  writer->closer = closer;

  return writer;
}

csvreturn csvwriter_next_record(csvwriter            writer,
                                const csvrecord_type record,
                                size_t               record_length) {
  return csvreturn_init(false);
}
