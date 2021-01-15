#include "draco/io/stdio_file_writer.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

#include "draco/draco_features.h"
#include "draco/io/file_writer_factory.h"
#include "draco/io/file_writer_utils.h"

namespace draco {

#define FILEWRITER_LOG_ERROR(error_string)                             \
  do {                                                                 \
    fprintf(stderr, "%s:%d (%s): %s.\n", __FILE__, __LINE__, __func__, \
            error_string);                                             \
  } while (false)

bool StdioFileWriter::registered_in_factory_ =
    FileWriterFactory::RegisterWriter(StdioFileWriter::Open);

StdioFileWriter::~StdioFileWriter() { fclose(file_); }

std::unique_ptr<FileWriterInterface> StdioFileWriter::Open(
    const std::string &file_name) {
  if (file_name.empty()) {
    return nullptr;
  }
  if (!CheckAndCreatePathForFile(file_name)) {
    return nullptr;
  }

  FILE *raw_file_ptr = fopen(file_name.c_str(), "wb");
  if (raw_file_ptr == nullptr) {
    return nullptr;
  }

  std::unique_ptr<StdioFileWriter> file(new (std::nothrow)
                                            StdioFileWriter(raw_file_ptr));
  if (file == nullptr) {
    FILEWRITER_LOG_ERROR("Out of memory");
    fclose(raw_file_ptr);
    return nullptr;
  }

#ifndef DRACO_OLD_GCC
  return file;
#else
  return std::move(file);
#endif
}

bool StdioFileWriter::Write(const char *buffer, size_t size) {
  return fwrite(buffer, 1, size, file_) == size;
}

}  // namespace draco
