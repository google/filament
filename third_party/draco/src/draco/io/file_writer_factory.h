#ifndef DRACO_IO_FILE_WRITER_FACTORY_H_
#define DRACO_IO_FILE_WRITER_FACTORY_H_

#include <memory>
#include <string>

#include "draco/io/file_writer_interface.h"

namespace draco {

class FileWriterFactory {
 public:
  using OpenFunction =
      std::unique_ptr<FileWriterInterface> (*)(const std::string &file_name);

  FileWriterFactory() = delete;
  FileWriterFactory(const FileWriterFactory &) = delete;
  FileWriterFactory &operator=(const FileWriterFactory &) = delete;
  ~FileWriterFactory() = default;

  // Registers the OpenFunction for a FileWriterInterface and returns true when
  // registration succeeds.
  static bool RegisterWriter(OpenFunction open_function);

  // Passes |file_name| to each OpenFunction until one succeeds. Returns nullptr
  // when no writer is found for |file_name|. Otherwise a FileWriterInterface is
  // returned.
  static std::unique_ptr<FileWriterInterface> OpenWriter(
      const std::string &file_name);
};

}  // namespace draco

#endif  // DRACO_IO_FILE_WRITER_FACTORY_H_
