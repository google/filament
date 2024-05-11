#include "draco/io/file_writer_factory.h"

#include <vector>

namespace draco {
namespace {

#define FILEWRITER_LOG_ERROR(error_string)                             \
  do {                                                                 \
    fprintf(stderr, "%s:%d (%s): %s.\n", __FILE__, __LINE__, __func__, \
            error_string);                                             \
  } while (false)

std::vector<FileWriterFactory::OpenFunction> *GetFileWriterOpenFunctions() {
  static auto open_functions =
      new (std::nothrow) std::vector<FileWriterFactory::OpenFunction>();
  return open_functions;
}

}  // namespace

bool FileWriterFactory::RegisterWriter(OpenFunction open_function) {
  if (open_function == nullptr) {
    return false;
  }
  auto open_functions = GetFileWriterOpenFunctions();
  const size_t num_writers = open_functions->size();
  open_functions->push_back(open_function);
  return open_functions->size() == num_writers + 1;
}

std::unique_ptr<FileWriterInterface> FileWriterFactory::OpenWriter(
    const std::string &file_name) {
  for (auto open_function : *GetFileWriterOpenFunctions()) {
    auto writer = open_function(file_name);
    if (writer == nullptr) {
      continue;
    }
    return writer;
  }
  FILEWRITER_LOG_ERROR("No file writer able to open output");
  return nullptr;
}

}  // namespace draco
