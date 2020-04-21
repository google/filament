#include "draco/io/stdio_file_writer.h"

#include <cstdio>
#include <cstring>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace draco {
namespace {

TEST(StdioFileWriterTest, FailOpen) {
  EXPECT_EQ(StdioFileWriter::Open(""), nullptr);
}

TEST(StdioFileWriterTest, BasicWrite) {
  const char kWriteBuffer[] = "Hello";
  const size_t kWriteLength = 5;
  const std::string kTempFilePath = GetTestTempFileFullPath("hello");
  auto writer = StdioFileWriter::Open(kTempFilePath);
  ASSERT_NE(writer, nullptr);
  ASSERT_TRUE(writer->Write(kWriteBuffer, kWriteLength));
  writer.reset();
  std::unique_ptr<FILE, decltype(&fclose)> file(
      fopen(kTempFilePath.c_str(), "rb"), fclose);
  ASSERT_NE(file, nullptr);
  char read_buffer[kWriteLength + 1] = {0};
  ASSERT_EQ(
      fread(reinterpret_cast<void *>(read_buffer), 1, kWriteLength, file.get()),
      kWriteLength);
  ASSERT_STREQ(read_buffer, kWriteBuffer);
}

}  // namespace
}  // namespace draco
