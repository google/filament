// Copyright (c) 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TOOLS_IO_H_
#define TOOLS_IO_H_

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

// Sets the contents of the file named |filename| in |data|, assuming each
// element in the file is of type |uint32_t|. The file is opened as a binary
// file. If |filename| is nullptr or "-", reads from the standard input, but
// reopened as a binary file. If any error occurs, writes error messages to
// standard error and returns false.
//
// If the given input is detected to be in ascii hex, it is converted to binary
// automatically.  In that case, the shape of the input data is determined based
// on the representation of the magic number:
//
//  * "[0]x[0]7230203": Every following "0x..." represents a word.
//  * "[0]x[0]7[,] [0]x23...": Every following "0x..." represents a byte, stored
//    in big-endian order
//  * "[0]x[0]3[,] [0]x[0]2...": Every following "0x..." represents a byte,
//    stored in little-endian order
//  * "07[, ]23...": Every following "XY" represents a byte, stored in
//    big-endian order
//  * "03[, ]02...": Every following "XY" represents a byte, stored in
//    little-endian order
bool ReadBinaryFile(const char* filename, std::vector<uint32_t>* data);

// The hex->binary logic of |ReadBinaryFile| applied to a pre-loaded stream of
// bytes.  Used by tests to avoid having to call |ReadBinaryFile| with temp
// files.  Returns false in case of parse errors.
bool ConvertHexToBinary(const std::vector<char>& stream,
                        std::vector<uint32_t>* data);

// Sets the contents of the file named |filename| in |data|, assuming each
// element in the file is of type |char|. The file is opened as a text file.  If
// |filename| is nullptr or "-", reads from the standard input, but reopened as
// a text file. If any error occurs, writes error messages to standard error and
// returns false.
bool ReadTextFile(const char* filename, std::vector<char>* data);

// Writes the given |data| into the file named as |filename| using the given
// |mode|, assuming |data| is an array of |count| elements of type |T|. If
// |filename| is nullptr or "-", writes to standard output. If any error occurs,
// returns false and outputs error message to standard error.
template <typename T>
bool WriteFile(const char* filename, const char* mode, const T* data,
               size_t count);

#endif  // TOOLS_IO_H_
