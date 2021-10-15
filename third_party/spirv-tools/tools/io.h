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

#if defined(SPIRV_WINDOWS)
#include <fcntl.h>
#include <io.h>

#define SET_STDIN_TO_BINARY_MODE() _setmode(_fileno(stdin), O_BINARY);
#define SET_STDIN_TO_TEXT_MODE() _setmode(_fileno(stdin), O_TEXT);
#else
#define SET_STDIN_TO_BINARY_MODE()
#define SET_STDIN_TO_TEXT_MODE()
#endif

// Appends the contents of the |file| to |data|, assuming each element in the
// file is of type |T|.
template <typename T>
void ReadFile(FILE* file, std::vector<T>* data) {
  if (file == nullptr) return;

  const int buf_size = 1024;
  T buf[buf_size];
  while (size_t len = fread(buf, sizeof(T), buf_size, file)) {
    data->insert(data->end(), buf, buf + len);
  }
}

// Returns true if |file| has encountered an error opening the file or reading
// the file as a series of element of type |T|. If there was an error, writes an
// error message to standard error.
template <class T>
bool WasFileCorrectlyRead(FILE* file, const char* filename) {
  if (file == nullptr) {
    fprintf(stderr, "error: file does not exist '%s'\n", filename);
    return false;
  }

  if (ftell(file) == -1L) {
    if (ferror(file)) {
      fprintf(stderr, "error: error reading file '%s'\n", filename);
      return false;
    }
  } else {
    if (sizeof(T) != 1 && (ftell(file) % sizeof(T))) {
      fprintf(
          stderr,
          "error: file size should be a multiple of %zd; file '%s' corrupt\n",
          sizeof(T), filename);
      return false;
    }
  }
  return true;
}

// Appends the contents of the file named |filename| to |data|, assuming
// each element in the file is of type |T|. The file is opened as a binary file
// If |filename| is nullptr or "-", reads from the standard input, but
// reopened as a binary file. If any error occurs, writes error messages to
// standard error and returns false.
template <typename T>
bool ReadBinaryFile(const char* filename, std::vector<T>* data) {
  const bool use_file = filename && strcmp("-", filename);
  FILE* fp = nullptr;
  if (use_file) {
    fp = fopen(filename, "rb");
  } else {
    SET_STDIN_TO_BINARY_MODE();
    fp = stdin;
  }

  ReadFile(fp, data);
  bool succeeded = WasFileCorrectlyRead<T>(fp, filename);
  if (use_file) fclose(fp);
  return succeeded;
}

// Appends the contents of the file named |filename| to |data|, assuming
// each element in the file is of type |T|. The file is opened as a text file
// If |filename| is nullptr or "-", reads from the standard input, but
// reopened as a text file. If any error occurs, writes error messages to
// standard error and returns false.
template <typename T>
bool ReadTextFile(const char* filename, std::vector<T>* data) {
  const bool use_file = filename && strcmp("-", filename);
  FILE* fp = nullptr;
  if (use_file) {
    fp = fopen(filename, "r");
  } else {
    SET_STDIN_TO_TEXT_MODE();
    fp = stdin;
  }

  ReadFile(fp, data);
  bool succeeded = WasFileCorrectlyRead<T>(fp, filename);
  if (use_file) fclose(fp);
  return succeeded;
}

// Writes the given |data| into the file named as |filename| using the given
// |mode|, assuming |data| is an array of |count| elements of type |T|. If
// |filename| is nullptr or "-", writes to standard output. If any error occurs,
// returns false and outputs error message to standard error.
template <typename T>
bool WriteFile(const char* filename, const char* mode, const T* data,
               size_t count) {
  const bool use_stdout =
      !filename || (filename[0] == '-' && filename[1] == '\0');
  if (FILE* fp = (use_stdout ? stdout : fopen(filename, mode))) {
    size_t written = fwrite(data, sizeof(T), count, fp);
    if (count != written) {
      fprintf(stderr, "error: could not write to file '%s'\n", filename);
      if (!use_stdout) fclose(fp);
      return false;
    }
    if (!use_stdout) fclose(fp);
  } else {
    fprintf(stderr, "error: could not open file '%s'\n", filename);
    return false;
  }
  return true;
}

#endif  // TOOLS_IO_H_
