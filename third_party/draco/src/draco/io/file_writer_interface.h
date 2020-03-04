#ifndef DRACO_IO_FILE_WRITER_INTERFACE_H_
#define DRACO_IO_FILE_WRITER_INTERFACE_H_

#include <cstddef>

namespace draco {

class FileWriterInterface {
 public:
  FileWriterInterface() = default;
  FileWriterInterface(const FileWriterInterface &) = delete;
  FileWriterInterface &operator=(const FileWriterInterface &) = delete;

  FileWriterInterface(FileWriterInterface &&) = default;
  FileWriterInterface &operator=(FileWriterInterface &&) = default;

  // Closes the file.
  virtual ~FileWriterInterface() = default;

  // Writes |size| bytes from |buffer| to file.
  virtual bool Write(const char *buffer, size_t size) = 0;
};

}  // namespace draco

#endif  // DRACO_IO_FILE_WRITER_INTERFACE_H_
