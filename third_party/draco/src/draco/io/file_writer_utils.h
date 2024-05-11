// Copyright 2020 The Draco Authors.
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
#ifndef DRACO_IO_FILE_WRITER_UTILS_H_
#define DRACO_IO_FILE_WRITER_UTILS_H_

#include <string>

namespace draco {

// Splits full path to a file into a folder path + file name.
// |out_folder_path| will contain the path to the folder containing the file
// excluding the final slash. If no folder is specified in the |full_path|, then
// |out_folder_path| is set to "."
void SplitPathPrivate(const std::string &full_path,
                      std::string *out_folder_path, std::string *out_file_name);

// Checks is |path| exists and if it is a directory.
bool DirectoryExists(const std::string &path);

// Checks if the path for file is valid. If not this function will try and
// create the path. Returns false on error.
bool CheckAndCreatePathForFile(const std::string &filename);

}  // namespace draco

#endif  // DRACO_IO_FILE_WRITER_UTILS_H_
