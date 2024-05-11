#ifndef DRACO_IO_STDIO_FILE_WRITER_H_
#define DRACO_IO_STDIO_FILE_WRITER_H_

#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>

#include "draco/io/file_writer_interface.h"

namespace draco {

class StdioFileWriter : public FileWriterInterface {
 public:
  // Creates and returns a StdioFileWriter that writes to |file_name|.
  // Returns nullptr when |file_name| cannot be opened for writing.
  static std::unique_ptr<FileWriterInterface> Open(
      const std::string &file_name);

  StdioFileWriter() = delete;
  StdioFileWriter(const StdioFileWriter &) = delete;
  StdioFileWriter &operator=(const StdioFileWriter &) = delete;

  StdioFileWriter(StdioFileWriter &&) = default;
  StdioFileWriter &operator=(StdioFileWriter &&) = default;

  // Closes |file_|.
  ~StdioFileWriter() override;

  // Writes |size| bytes to |file_| from |buffer|. Returns true for success.
  bool Write(const char *buffer, size_t size) override;

 private:
  StdioFileWriter(FILE *file) : file_(file) {}

  FILE *file_ = nullptr;
  static bool registered_in_factory_;
};

}  // namespace draco

#endif  // DRACO_IO_STDIO_FILE_WRITER_H_
