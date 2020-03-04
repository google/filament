#include "draco/io/stdio_file_reader.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#endif

#include "draco/io/file_reader_factory.h"

namespace draco {

#define FILEREADER_LOG_ERROR(error_string)                             \
  do {                                                                 \
    fprintf(stderr, "%s:%d (%s): %s.\n", __FILE__, __LINE__, __func__, \
            error_string);                                             \
  } while (false)

bool StdioFileReader::registered_in_factory_ =
    FileReaderFactory::RegisterReader(StdioFileReader::Open);

StdioFileReader::~StdioFileReader() { fclose(file_); }

std::unique_ptr<FileReaderInterface> StdioFileReader::Open(
    const std::string &file_name) {
  if (file_name.empty()) {
    return nullptr;
  }

  FILE *raw_file_ptr = fopen(file_name.c_str(), "rb");

  if (raw_file_ptr == nullptr) {
    return nullptr;
  }

  std::unique_ptr<FileReaderInterface> file(new (std::nothrow)
                                                StdioFileReader(raw_file_ptr));
  if (file == nullptr) {
    FILEREADER_LOG_ERROR("Out of memory");
    fclose(raw_file_ptr);
    return nullptr;
  }

  return file;
}

bool StdioFileReader::ReadFileToBuffer(std::vector<char> *buffer) {
  if (buffer == nullptr) {
    return false;
  }
  buffer->clear();

  const size_t file_size = GetFileSize();
  if (file_size == 0) {
    FILEREADER_LOG_ERROR("Unable to obtain file size or file empty");
    return false;
  }

  buffer->resize(file_size);
  return fread(buffer->data(), 1, file_size, file_) == file_size;
}

bool StdioFileReader::ReadFileToBuffer(std::vector<uint8_t> *buffer) {
  if (buffer == nullptr) {
    return false;
  }
  buffer->clear();

  const size_t file_size = GetFileSize();
  if (file_size == 0) {
    FILEREADER_LOG_ERROR("Unable to obtain file size or file empty");
    return false;
  }

  buffer->resize(file_size);
  return fread(buffer->data(), 1, file_size, file_) == file_size;
}

size_t StdioFileReader::GetFileSize() {
  if (fseek(file_, SEEK_SET, SEEK_END) != 0) {
    FILEREADER_LOG_ERROR("Seek to EoF failed");
    return false;
  }

  const size_t file_size = static_cast<size_t>(ftell(file_));
  rewind(file_);

  return file_size;
}

}  // namespace draco
