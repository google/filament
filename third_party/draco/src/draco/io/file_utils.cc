// Copyright 2018 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "draco/io/file_utils.h"

#include "draco/io/file_reader_factory.h"
#include "draco/io/file_reader_interface.h"
#include "draco/io/file_writer_factory.h"
#include "draco/io/file_writer_interface.h"
#include "draco/io/file_writer_utils.h"
#include "draco/io/parser_utils.h"

namespace draco {

void SplitPath(const std::string &full_path, std::string *out_folder_path,
               std::string *out_file_name) {
  SplitPathPrivate(full_path, out_folder_path, out_file_name);
}

std::string ReplaceFileExtension(const std::string &in_file_name,
                                 const std::string &new_extension) {
  const auto pos = in_file_name.find_last_of(".");
  if (pos == std::string::npos) {
    // No extension found.
    return in_file_name + "." + new_extension;
  }
  return in_file_name.substr(0, pos + 1) + new_extension;
}

std::string LowercaseFileExtension(const std::string &filename) {
  const size_t pos = filename.find_last_of('.');
  if (pos == 0 || pos == std::string::npos || pos == filename.length() - 1) {
    return "";
  }
  return parser::ToLower(filename.substr(pos + 1));
}

std::string GetFullPath(const std::string &input_file_relative_path,
                        const std::string &sibling_file_full_path) {
  const auto pos = sibling_file_full_path.find_last_of("/\\");
  std::string input_file_full_path;
  if (pos != std::string::npos) {
    input_file_full_path = sibling_file_full_path.substr(0, pos + 1);
  }
  input_file_full_path += input_file_relative_path;
  return input_file_full_path;
}

bool ReadFileToBuffer(const std::string &file_name, std::vector<char> *buffer) {
  std::unique_ptr<FileReaderInterface> file_reader =
      FileReaderFactory::OpenReader(file_name);
  if (file_reader == nullptr) {
    return false;
  }
  return file_reader->ReadFileToBuffer(buffer);
}

bool ReadFileToBuffer(const std::string &file_name,
                      std::vector<uint8_t> *buffer) {
  std::unique_ptr<FileReaderInterface> file_reader =
      FileReaderFactory::OpenReader(file_name);
  if (file_reader == nullptr) {
    return false;
  }
  return file_reader->ReadFileToBuffer(buffer);
}

bool WriteBufferToFile(const char *buffer, size_t buffer_size,
                       const std::string &file_name) {
  std::unique_ptr<FileWriterInterface> file_writer =
      FileWriterFactory::OpenWriter(file_name);
  if (file_writer == nullptr) {
    return false;
  }
  return file_writer->Write(buffer, buffer_size);
}

bool WriteBufferToFile(const unsigned char *buffer, size_t buffer_size,
                       const std::string &file_name) {
  return WriteBufferToFile(reinterpret_cast<const char *>(buffer), buffer_size,
                           file_name);
}

bool WriteBufferToFile(const void *buffer, size_t buffer_size,
                       const std::string &file_name) {
  return WriteBufferToFile(reinterpret_cast<const char *>(buffer), buffer_size,
                           file_name);
}

size_t GetFileSize(const std::string &file_name) {
  std::unique_ptr<FileReaderInterface> file_reader =
      FileReaderFactory::OpenReader(file_name);
  if (file_reader == nullptr) {
    return 0;
  }
  return file_reader->GetFileSize();
}

}  // namespace draco
