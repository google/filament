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

#include "content_stream.h"

#include "dap/io.h"

#include <string.h>   // strlen
#include <algorithm>  // std::min

namespace dap {

////////////////////////////////////////////////////////////////////////////////
// ContentReader
////////////////////////////////////////////////////////////////////////////////
ContentReader::ContentReader(const std::shared_ptr<Reader>& reader)
    : reader(reader) {}

ContentReader& ContentReader::operator=(ContentReader&& rhs) noexcept {
  buf = std::move(rhs.buf);
  reader = std::move(rhs.reader);
  return *this;
}

bool ContentReader::isOpen() {
  return reader ? reader->isOpen() : false;
}

void ContentReader::close() {
  if (reader) {
    reader->close();
  }
}

std::string ContentReader::read() {
  // Find Content-Length header prefix
  if (!scan("Content-Length:")) {
    return "";
  }
  // Skip whitespace and tabs
  while (matchAny(" \t")) {
  }
  // Parse length
  size_t len = 0;
  while (true) {
    auto c = matchAny("0123456789");
    if (c == 0) {
      break;
    }
    len *= 10;
    len += size_t(c) - size_t('0');
  }
  if (len == 0) {
    return "";
  }
  // Expect \r\n\r\n
  if (!match("\r\n\r\n")) {
    return "";
  }
  // Read message
  if (!buffer(len)) {
    return "";
  }
  std::string out;
  out.reserve(len);
  for (size_t i = 0; i < len; i++) {
    out.push_back(static_cast<char>(buf.front()));
    buf.pop_front();
  }
  return out;
}

bool ContentReader::scan(const uint8_t* seq, size_t len) {
  while (buffer(len)) {
    if (match(seq, len)) {
      return true;
    }
    buf.pop_front();
  }
  return false;
}

bool ContentReader::scan(const char* str) {
  auto len = strlen(str);
  return scan(reinterpret_cast<const uint8_t*>(str), len);
}

bool ContentReader::match(const uint8_t* seq, size_t len) {
  if (!buffer(len)) {
    return false;
  }
  auto it = buf.begin();
  for (size_t i = 0; i < len; i++, it++) {
    if (*it != seq[i]) {
      return false;
    }
  }
  for (size_t i = 0; i < len; i++) {
    buf.pop_front();
  }
  return true;
}

bool ContentReader::match(const char* str) {
  auto len = strlen(str);
  return match(reinterpret_cast<const uint8_t*>(str), len);
}

char ContentReader::matchAny(const char* chars) {
  if (!buffer(1)) {
    return false;
  }
  int c = buf.front();
  if (auto p = strchr(chars, c)) {
    buf.pop_front();
    return *p;
  }
  return 0;
}

bool ContentReader::buffer(size_t bytes) {
  if (bytes < buf.size()) {
    return true;
  }
  bytes -= buf.size();
  while (bytes > 0) {
    uint8_t chunk[256];
    auto numWant = std::min(sizeof(chunk), bytes);
    auto numGot = reader->read(chunk, numWant);
    if (numGot == 0) {
      return false;
    }
    for (size_t i = 0; i < numGot; i++) {
      buf.push_back(chunk[i]);
    }
    bytes -= numGot;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// ContentWriter
////////////////////////////////////////////////////////////////////////////////
ContentWriter::ContentWriter(const std::shared_ptr<Writer>& rhs)
    : writer(rhs) {}

ContentWriter& ContentWriter::operator=(ContentWriter&& rhs) noexcept {
  writer = std::move(rhs.writer);
  return *this;
}

bool ContentWriter::isOpen() {
  return writer ? writer->isOpen() : false;
}

void ContentWriter::close() {
  if (writer) {
    writer->close();
  }
}

bool ContentWriter::write(const std::string& msg) const {
  auto header =
      std::string("Content-Length: ") + std::to_string(msg.size()) + "\r\n\r\n";
  return writer->write(header.data(), header.size()) &&
         writer->write(msg.data(), msg.size());
}

}  // namespace dap