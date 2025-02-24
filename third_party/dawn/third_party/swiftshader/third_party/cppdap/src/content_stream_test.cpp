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

#include "string_buffer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <memory>

namespace {

// SingleByteReader wraps a dap::Reader to only provide a single byte for each
// read() call, regardless of the number of bytes actually requested.
class SingleByteReader : public dap::Reader {
 public:
  SingleByteReader(std::unique_ptr<dap::Reader>&& inner)
      : inner(std::move(inner)) {}

  bool isOpen() override { return inner->isOpen(); }
  void close() override { return inner->close(); }
  size_t read(void* buffer, size_t) override { return inner->read(buffer, 1); };

 private:
  std::unique_ptr<dap::Reader> inner;
};

}  // namespace

TEST(ContentStreamTest, Write) {
  auto sb = dap::StringBuffer::create();
  auto ptr = sb.get();
  dap::ContentWriter cw(std::move(sb));
  cw.write("Content payload number one");
  cw.write("Content payload number two");
  cw.write("Content payload number three");
  ASSERT_EQ(ptr->string(),
            "Content-Length: 26\r\n\r\nContent payload number one"
            "Content-Length: 26\r\n\r\nContent payload number two"
            "Content-Length: 28\r\n\r\nContent payload number three");
}

TEST(ContentStreamTest, Read) {
  auto sb = dap::StringBuffer::create();
  sb->write("Content-Length: 26\r\n\r\nContent payload number one");
  sb->write("some unrecognised garbage");
  sb->write("Content-Length: 26\r\n\r\nContent payload number two");
  sb->write("some more unrecognised garbage");
  sb->write("Content-Length: 28\r\n\r\nContent payload number three");
  dap::ContentReader cs(std::move(sb));
  ASSERT_EQ(cs.read(), "Content payload number one");
  ASSERT_EQ(cs.read(), "Content payload number two");
  ASSERT_EQ(cs.read(), "Content payload number three");
  ASSERT_EQ(cs.read(), "");
}

TEST(ContentStreamTest, ShortRead) {
  auto sb = dap::StringBuffer::create();
  sb->write("Content-Length: 26\r\n\r\nContent payload number one");
  sb->write("some unrecognised garbage");
  sb->write("Content-Length: 26\r\n\r\nContent payload number two");
  sb->write("some more unrecognised garbage");
  sb->write("Content-Length: 28\r\n\r\nContent payload number three");
  dap::ContentReader cs(
      std::unique_ptr<SingleByteReader>(new SingleByteReader(std::move(sb))));
  ASSERT_EQ(cs.read(), "Content payload number one");
  ASSERT_EQ(cs.read(), "Content payload number two");
  ASSERT_EQ(cs.read(), "Content payload number three");
  ASSERT_EQ(cs.read(), "");
}
