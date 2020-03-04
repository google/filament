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
#ifndef DRACO_IO_FILE_UTILS_H_
#define DRACO_IO_FILE_UTILS_H_

#include <string>
#include <vector>

namespace draco {

// Splits full path to a file into a folder path + file name.
// |out_folder_path| will contain the path to the folder containing the file
// excluding the final slash. If no folder is specified in the |full_path|, then
// |out_folder_path| is set to "."
// Returns false on error.
bool SplitPath(const std::string &full_path, std::string *out_folder_path,
               std::string *out_file_name);

// Replaces file extension in |in_file_name| with |new_extension|.
// If |in_file_name| does not have any extension, the extension is appended.
std::string ReplaceFileExtension(const std::string &in_file_name,
                                 const std::string &new_extension);

// Returns the file extension in lowercase if present, else "". Extension is
// defined as the string after the last '.' character. If the file starts with
// '.' (e.g. Linux hidden files), the first delimiter is ignored.
std::string LowercaseFileExtension(const std::string &filename);

// Given a path of the input file |input_file_relative_path| relative to the
// parent directory of |sibling_file_full_path|, this function returns full path
// to the input file. If |sibling_file_full_path| has no directory, the relative
// path itself |input_file_relative_path| is returned. A common use case is for
// the |input_file_relative_path| to be just a file name. See usage examples in
// the unit test.
std::string GetFullPath(const std::string &input_file_relative_path,
                        const std::string &sibling_file_full_path);

// Convenience method. Uses draco::FileReaderFactory internally. Reads contents
// of file referenced by |file_name| into |buffer| and returns true upon
// success.
bool ReadFileToBuffer(const std::string &file_name, std::vector<char> *buffer);
bool ReadFileToBuffer(const std::string &file_name,
                      std::vector<uint8_t> *buffer);

// Convenience method. Uses draco::FileWriterFactory internally. Writes contents
// of |buffer| to file referred to by |file_name|. File is overwritten if it
// exists. Returns true after successful write.
bool WriteBufferToFile(const char *buffer, size_t buffer_size,
                       const std::string &file_name);
bool WriteBufferToFile(const unsigned char *buffer, size_t buffer_size,
                       const std::string &file_name);
bool WriteBufferToFile(const void *buffer, size_t buffer_size,
                       const std::string &file_name);

// Convenience method. Uses draco::FileReaderFactory internally. Returns size of
// file referenced by |file_name|. Returns 0 when referenced file is empty or
// does not exist.
size_t GetFileSize(const std::string &file_name);

}  // namespace draco

#endif  // DRACO_IO_FILE_UTILS_H_
