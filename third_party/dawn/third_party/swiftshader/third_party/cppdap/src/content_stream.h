// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef dap_content_stream_h
#define dap_content_stream_h

#include <deque>
#include <memory>
#include <string>

#include <stdint.h>

namespace dap {

// Forward declarations
class Reader;
class Writer;

class ContentReader {
 public:
  ContentReader() = default;
  ContentReader(const std::shared_ptr<Reader>&);
  ContentReader& operator=(ContentReader&&) noexcept;

  bool isOpen();
  void close();
  std::string read();

 private:
  bool scan(const uint8_t* seq, size_t len);
  bool scan(const char* str);
  bool match(const uint8_t* seq, size_t len);
  bool match(const char* str);
  char matchAny(const char* chars);
  bool buffer(size_t bytes);

  std::shared_ptr<Reader> reader;
  std::deque<uint8_t> buf;
};

class ContentWriter {
 public:
  ContentWriter() = default;
  ContentWriter(const std::shared_ptr<Writer>&);
  ContentWriter& operator=(ContentWriter&&) noexcept;

  bool isOpen();
  void close();
  bool write(const std::string&) const;

 private:
  std::shared_ptr<Writer> writer;
};

}  // namespace dap

#endif  // dap_content_stream_h
