#ifndef DRACO_IO_FILE_READER_FACTORY_H_
#define DRACO_IO_FILE_READER_FACTORY_H_

#include <memory>
#include <string>

#include "draco/io/file_reader_interface.h"

namespace draco {

class FileReaderFactory {
 public:
  using OpenFunction =
      std::unique_ptr<FileReaderInterface> (*)(const std::string &file_name);

  FileReaderFactory() = delete;
  FileReaderFactory(const FileReaderFactory &) = delete;
  FileReaderFactory &operator=(const FileReaderFactory &) = delete;
  ~FileReaderFactory() = default;

  // Registers the OpenFunction for a FileReaderInterface and returns true when
  // registration succeeds.
  static bool RegisterReader(OpenFunction open_function);

  // Passes |file_name| to each OpenFunction until one succeeds. Returns nullptr
  // when no reader is found for |file_name|. Otherwise a FileReaderInterface is
  // returned.
  static std::unique_ptr<FileReaderInterface> OpenReader(
      const std::string &file_name);
};

}  // namespace draco

#endif  // DRACO_IO_FILE_READER_FACTORY_H_
