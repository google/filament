#ifndef DRACO_IO_STDIO_FILE_READER_H_
#define DRACO_IO_STDIO_FILE_READER_H_

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "draco/io/file_reader_interface.h"

namespace draco {

class StdioFileReader : public FileReaderInterface {
 public:
  // Creates and returns a StdioFileReader that reads from |file_name|.
  // Returns nullptr when the file does not exist or cannot be read.
  static std::unique_ptr<FileReaderInterface> Open(
      const std::string &file_name);

  StdioFileReader() = delete;
  StdioFileReader(const StdioFileReader &) = delete;
  StdioFileReader &operator=(const StdioFileReader &) = delete;

  StdioFileReader(StdioFileReader &&) = default;
  StdioFileReader &operator=(StdioFileReader &&) = default;

  // Closes |file_|.
  ~StdioFileReader() override;

  // Reads the entire contents of the input file into |buffer| and returns true.
  bool ReadFileToBuffer(std::vector<char> *buffer) override;
  bool ReadFileToBuffer(std::vector<uint8_t> *buffer) override;

  // Returns the size of the file.
  size_t GetFileSize() override;

 private:
  StdioFileReader(FILE *file) : file_(file) {}

  FILE *file_ = nullptr;
  static bool registered_in_factory_;
};

}  // namespace draco

#endif  // DRACO_IO_STDIO_FILE_READER_H_
