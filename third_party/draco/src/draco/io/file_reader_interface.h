#ifndef DRACO_IO_FILE_READER_INTERFACE_H_
#define DRACO_IO_FILE_READER_INTERFACE_H_

#include <cstddef>
#include <cstdint>
#include <vector>

namespace draco {

class FileReaderInterface {
 public:
  FileReaderInterface() = default;
  FileReaderInterface(const FileReaderInterface &) = delete;
  FileReaderInterface &operator=(const FileReaderInterface &) = delete;

  FileReaderInterface(FileReaderInterface &&) = default;
  FileReaderInterface &operator=(FileReaderInterface &&) = default;

  // Closes the file.
  virtual ~FileReaderInterface() = default;

  // Reads the entire contents of the input file into |buffer| and returns true.
  virtual bool ReadFileToBuffer(std::vector<char> *buffer) = 0;
  virtual bool ReadFileToBuffer(std::vector<uint8_t> *buffer) = 0;

  // Returns the size of the file.
  virtual size_t GetFileSize() = 0;
};

}  // namespace draco

#endif  // DRACO_IO_FILE_READER_INTERFACE_H_
