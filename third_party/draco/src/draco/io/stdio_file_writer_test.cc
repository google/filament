#include "draco/io/stdio_file_writer.h"

#include <cstdio>
#include <cstring>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace draco {
namespace {

void CheckFileWriter(const std::string &data, const std::string &filename) {
  auto writer = StdioFileWriter::Open(filename);
  ASSERT_NE(writer, nullptr);
  ASSERT_TRUE(writer->Write(data.data(), data.size()));
  writer.reset();
  std::unique_ptr<FILE, decltype(&fclose)> file(fopen(filename.c_str(), "r"),
                                                fclose);
  ASSERT_NE(file, nullptr);
  std::string read_buffer(data.size(), ' ');
  ASSERT_EQ(fread(reinterpret_cast<void *>(&read_buffer[0]), 1, data.size(),
                  file.get()),
            data.size());
  ASSERT_EQ(read_buffer, data);
}

TEST(StdioFileWriterTest, FailOpen) {
  EXPECT_EQ(StdioFileWriter::Open(""), nullptr);
}

TEST(StdioFileWriterTest, BasicWrite) {
  const std::string kWriteString = "Hello";
  const std::string kTempFilePath = GetTestTempFileFullPath("hello");
  CheckFileWriter(kWriteString, kTempFilePath);
}

}  // namespace
}  // namespace draco
