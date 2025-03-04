// Copyright (c) 2024 Google Inc.
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

#include "io.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#if defined(SPIRV_WINDOWS)
#include <fcntl.h>
#include <io.h>

#define SET_STDIN_TO_BINARY_MODE() _setmode(_fileno(stdin), O_BINARY);
#define SET_STDIN_TO_TEXT_MODE() _setmode(_fileno(stdin), O_TEXT);
#define SET_STDOUT_TO_BINARY_MODE() _setmode(_fileno(stdout), O_BINARY);
#define SET_STDOUT_TO_TEXT_MODE() _setmode(_fileno(stdout), O_TEXT);
#define SET_STDOUT_MODE(mode) _setmode(_fileno(stdout), mode);
#else
#define SET_STDIN_TO_BINARY_MODE()
#define SET_STDIN_TO_TEXT_MODE()
#define SET_STDOUT_TO_BINARY_MODE() 0
#define SET_STDOUT_TO_TEXT_MODE() 0
#define SET_STDOUT_MODE(mode)
#endif

namespace {
// Appends the contents of the |file| to |data|, assuming each element in the
// file is of type |T|.
template <typename T>
void ReadFile(FILE* file, std::vector<T>* data) {
  if (file == nullptr) return;

  const int buf_size = 4096 / sizeof(T);
  T buf[buf_size];
  while (size_t len = fread(buf, sizeof(T), buf_size, file)) {
    data->insert(data->end(), buf, buf + len);
  }
}

// Returns true if |file| has encountered an error opening the file or reading
// from it. If there was an error, writes an error message to standard error.
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
  }
  return true;
}

// Ensure the file contained an exact number of elements, whose size is given in
// |alignment|.
bool WasFileSizeAligned(const char* filename, size_t read_size,
                        size_t alignment) {
  assert(alignment != 1);
  if ((read_size % alignment) != 0) {
    fprintf(stderr,
            "error: file size should be a multiple of %zd; file '%s' corrupt\n",
            alignment, filename);
    return false;
  }
  return true;
}

// Different formats the hex is expected to be in.
enum class HexMode {
  // 0x07230203, ...
  Words,
  // 0x07, 0x23, 0x02, 0x03, ...
  BytesBigEndian,
  // 0x03, 0x02, 0x23, 0x07, ...
  BytesLittleEndian,
  // 07 23 02 03 ...
  StreamBigEndian,
  // 03 02 23 07 ...
  StreamLittleEndian,
};

// Whether a character should be skipped as whitespace / separator /
// end-of-file.
bool IsSpace(char c) { return isspace(c) || c == ',' || c == '\0'; }

bool IsHexStream(const std::vector<char>& stream) {
  for (char c : stream) {
    if (IsSpace(c)) {
      continue;
    }

    // Every possible case of a SPIR-V hex stream starts with either '0' or 'x'
    // (see |HexMode| values).  Make a decision upon inspecting the first
    // non-space character.
    return c == '0' || c == 'x' || c == 'X';
  }

  return false;
}

bool MatchIgnoreCase(const char* token, const char* expect, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    if (tolower(token[i]) != tolower(expect[i])) {
      return false;
    }
  }

  return true;
}

// Helper class to tokenize a hex stream
class HexTokenizer {
 public:
  HexTokenizer(const char* filename, const std::vector<char>& stream,
               std::vector<uint32_t>* data)
      : filename_(filename), stream_(stream), data_(data) {
    DetermineMode();
  }

  bool Parse() {
    while (current_ < stream_.size() && !encountered_error_) {
      data_->push_back(GetNextWord());

      // Make sure trailing space does not lead to parse error by skipping it
      // and exiting the loop.
      SkipSpace();
    }

    return !encountered_error_;
  }

 private:
  void ParseError(const char* reason) {
    if (!encountered_error_) {
      fprintf(stderr,
              "error: hex stream parse error at character %zu: %s in '%s'\n",
              current_, reason, filename_);
      encountered_error_ = true;
    }
  }

  // Skip whitespace until the next non-whitespace non-comma character.
  void SkipSpace() {
    while (current_ < stream_.size()) {
      char c = stream_[current_];
      if (!IsSpace(c)) {
        return;
      }

      ++current_;
    }
  }

  // Skip the 0x or x at the beginning of a hex value.
  void Skip0x() {
    // The first character must be 0 or x.
    const char first = Next();
    if (first != '0' && first != 'x' && first != 'X') {
      ParseError("expected 0x or x");
    } else if (first == '0') {
      const char second = Next();
      if (second != 'x' && second != 'X') {
        ParseError("expected 0x");
      }
    }
  }

  // Consume the next character.
  char Next() { return current_ < stream_.size() ? stream_[current_++] : '\0'; }

  // Determine how to read the hex stream based on the first token.
  void DetermineMode() {
    SkipSpace();

    // Read 11 bytes, that is the size of the biggest token (10) + one more.
    char first_token[11];
    for (uint32_t i = 0; i < 11; ++i) {
      first_token[i] = Next();
    }

    // Table of how to match the first token with a mode.
    struct {
      const char* expect;
      bool must_have_delimiter;
      HexMode mode;
    } parse_info[] = {
        {"0x07230203", true, HexMode::Words},
        {"0x7230203", true, HexMode::Words},
        {"x07230203", true, HexMode::Words},
        {"x7230203", true, HexMode::Words},

        {"0x07", true, HexMode::BytesBigEndian},
        {"0x7", true, HexMode::BytesBigEndian},
        {"x07", true, HexMode::BytesBigEndian},
        {"x7", true, HexMode::BytesBigEndian},

        {"0x03", true, HexMode::BytesLittleEndian},
        {"0x3", true, HexMode::BytesLittleEndian},
        {"x03", true, HexMode::BytesLittleEndian},
        {"x3", true, HexMode::BytesLittleEndian},

        {"07", false, HexMode::StreamBigEndian},
        {"03", false, HexMode::StreamLittleEndian},
    };

    // Check to see if any of the possible first tokens are matched.  If not,
    // this is not a recognized hex stream.
    encountered_error_ = true;
    for (const auto& info : parse_info) {
      const size_t expect_len = strlen(info.expect);
      const bool matches_expect =
          MatchIgnoreCase(first_token, info.expect, expect_len);
      const bool satisfies_delimeter =
          !info.must_have_delimiter || IsSpace(first_token[expect_len]);
      if (matches_expect && satisfies_delimeter) {
        mode_ = info.mode;
        encountered_error_ = false;
        break;
      }
    }

    if (encountered_error_) {
      fprintf(stderr,
              "error: hex format detected, but pattern '%.11s' is not "
              "recognized '%s'\n",
              first_token, filename_);
    }

    // Reset the position to restart parsing with the determined mode.
    current_ = 0;
  }

  // Consume up to |max_len| characters and put them in |token_chars|.  A
  // delimiter is expected. The resulting string is NUL-terminated.
  void NextN(char token_chars[9], size_t max_len) {
    assert(max_len < 9);

    for (size_t i = 0; i <= max_len; ++i) {
      char c = Next();
      if (IsSpace(c)) {
        token_chars[i] = '\0';
        return;
      }

      token_chars[i] = c;
      if (!isxdigit(c)) {
        ParseError("encountered non-hex character");
      }
    }

    // If space is not reached before the maximum number of characters where
    // consumed, that's an error.
    ParseError("expected delimiter (space or comma)");
    token_chars[max_len] = '\0';
  }

  // Consume one hex digit.
  char NextHexDigit() {
    char c = Next();
    if (!isxdigit(c)) {
      ParseError("encountered non-hex character");
    }
    return c;
  }

  // Extract a token out of the stream.  It could be either a word or a byte,
  // based on |mode_|.
  uint32_t GetNextToken() {
    SkipSpace();

    // The longest token can be 8 chars (for |HexMode::Words|), add one for
    // '\0'.
    char token_chars[9];

    switch (mode_) {
      case HexMode::Words:
      case HexMode::BytesBigEndian:
      case HexMode::BytesLittleEndian:
        // Start with 0x, followed by up to 8 (for Word) or 2 (for Byte*)
        // digits.
        Skip0x();
        NextN(token_chars, mode_ == HexMode::Words ? 8 : 2);
        break;
      case HexMode::StreamBigEndian:
      case HexMode::StreamLittleEndian:
        // Always expected to see two consecutive hex digits.
        token_chars[0] = NextHexDigit();
        token_chars[1] = NextHexDigit();
        token_chars[2] = '\0';
        break;
    }

    if (encountered_error_) {
      return 0;
    }

    // Parse the hex value that was just read.
    return static_cast<uint32_t>(strtol(token_chars, nullptr, 16));
  }

  // Construct a word out of tokens
  uint32_t GetNextWord() {
    if (mode_ == HexMode::Words) {
      return GetNextToken();
    }

    uint32_t tokens[4] = {
        GetNextToken(),
        GetNextToken(),
        GetNextToken(),
        GetNextToken(),
    };

    switch (mode_) {
      case HexMode::BytesBigEndian:
      case HexMode::StreamBigEndian:
        return tokens[0] << 24 | tokens[1] << 16 | tokens[2] << 8 | tokens[3];
      case HexMode::BytesLittleEndian:
      case HexMode::StreamLittleEndian:
        return tokens[3] << 24 | tokens[2] << 16 | tokens[1] << 8 | tokens[0];
      default:
        assert(false);
        return 0;
    }
  }

  const char* filename_;
  const std::vector<char>& stream_;
  std::vector<uint32_t>* data_;

  HexMode mode_ = HexMode::Words;
  size_t current_ = 0;
  bool encountered_error_ = false;
};
}  // namespace

bool ReadBinaryFile(const char* filename, std::vector<uint32_t>* data) {
  assert(data->empty());

  const bool use_file = filename && strcmp("-", filename);
  FILE* fp = nullptr;
  if (use_file) {
    fp = fopen(filename, "rb");
  } else {
    SET_STDIN_TO_BINARY_MODE();
    fp = stdin;
  }

  // Read into a char vector first.  If this is a hex stream, it needs to be
  // processed as such.
  std::vector<char> data_raw;
  ReadFile(fp, &data_raw);
  bool succeeded = WasFileCorrectlyRead(fp, filename);
  if (use_file && fp) fclose(fp);

  if (!succeeded) {
    return false;
  }

  if (IsHexStream(data_raw)) {
    // If a hex stream, parse it and fill |data|.
    HexTokenizer tokenizer(filename, data_raw, data);
    succeeded = tokenizer.Parse();
  } else {
    // If not a hex stream, convert it to uint32_t via memcpy.
    succeeded = WasFileSizeAligned(filename, data_raw.size(), sizeof(uint32_t));
    if (succeeded) {
      data->resize(data_raw.size() / sizeof(uint32_t), 0);
      memcpy(data->data(), data_raw.data(), data_raw.size());
    }
  }

  return succeeded;
}

bool ConvertHexToBinary(const std::vector<char>& stream,
                        std::vector<uint32_t>* data) {
  HexTokenizer tokenizer("<input string>", stream, data);
  return tokenizer.Parse();
}

bool ReadTextFile(const char* filename, std::vector<char>* data) {
  assert(data->empty());

  const bool use_file = filename && strcmp("-", filename);
  FILE* fp = nullptr;
  if (use_file) {
    fp = fopen(filename, "r");
  } else {
    SET_STDIN_TO_TEXT_MODE();
    fp = stdin;
  }

  ReadFile(fp, data);
  bool succeeded = WasFileCorrectlyRead(fp, filename);
  if (use_file && fp) fclose(fp);
  return succeeded;
}

namespace {
// A class to create and manage a file for outputting data.
class OutputFile {
 public:
  // Opens |filename| in the given mode.  If |filename| is nullptr, the empty
  // string or "-", stdout will be set to the given mode.
  OutputFile(const char* filename, const char* mode) : old_mode_(0) {
    const bool use_stdout =
        !filename || (filename[0] == '-' && filename[1] == '\0');
    if (use_stdout) {
      if (strchr(mode, 'b')) {
        old_mode_ = SET_STDOUT_TO_BINARY_MODE();
      } else {
        old_mode_ = SET_STDOUT_TO_TEXT_MODE();
      }
      fp_ = stdout;
    } else {
      fp_ = fopen(filename, mode);
    }
  }

  ~OutputFile() {
    if (fp_ == stdout) {
      fflush(stdout);
      SET_STDOUT_MODE(old_mode_);
    } else if (fp_ != nullptr) {
      fclose(fp_);
    }
  }

  // Returns a file handle to the file.
  FILE* GetFileHandle() const { return fp_; }

 private:
  FILE* fp_;
  int old_mode_;
};
}  // namespace

template <typename T>
bool WriteFile(const char* filename, const char* mode, const T* data,
               size_t count) {
  OutputFile file(filename, mode);
  FILE* fp = file.GetFileHandle();
  if (fp == nullptr) {
    fprintf(stderr, "error: could not open file '%s'\n", filename);
    return false;
  }

  size_t written = fwrite(data, sizeof(T), count, fp);
  if (count != written) {
    fprintf(stderr, "error: could not write to file '%s'\n", filename);
    return false;
  }

  return true;
}

template bool WriteFile<uint32_t>(const char* filename, const char* mode,
                                  const uint32_t* data, size_t count);
template bool WriteFile<char>(const char* filename, const char* mode,
                              const char* data, size_t count);
