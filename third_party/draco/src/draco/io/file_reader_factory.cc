#include "draco/io/file_reader_factory.h"

#include <vector>

namespace draco {
namespace {

#define FILEREADER_LOG_ERROR(error_string)                             \
  do {                                                                 \
    fprintf(stderr, "%s:%d (%s): %s.\n", __FILE__, __LINE__, __func__, \
            error_string);                                             \
  } while (false)

std::vector<FileReaderFactory::OpenFunction> *GetFileReaderOpenFunctions() {
  static auto open_functions =
      new (std::nothrow) std::vector<FileReaderFactory::OpenFunction>();
  return open_functions;
}

}  // namespace

bool FileReaderFactory::RegisterReader(OpenFunction open_function) {
  if (open_function == nullptr) {
    return false;
  }
  auto open_functions = GetFileReaderOpenFunctions();
  const size_t num_readers = open_functions->size();
  open_functions->push_back(open_function);
  return open_functions->size() == num_readers + 1;
}

std::unique_ptr<FileReaderInterface> FileReaderFactory::OpenReader(
    const std::string &file_name) {
  for (auto open_function : *GetFileReaderOpenFunctions()) {
    auto reader = open_function(file_name);
    if (reader == nullptr) {
      continue;
    }
    return reader;
  }
  FILEREADER_LOG_ERROR("No file reader able to open input");
  return nullptr;
}

}  // namespace draco
