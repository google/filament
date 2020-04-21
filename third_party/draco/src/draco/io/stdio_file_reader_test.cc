#include "draco/io/stdio_file_reader.h"

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/io/file_reader_test_common.h"

namespace draco {
namespace {

TEST(StdioFileReaderTest, FailOpen) {
  EXPECT_EQ(StdioFileReader::Open(""), nullptr);
  EXPECT_EQ(StdioFileReader::Open("fake file"), nullptr);
}

TEST(StdioFileReaderTest, Open) {
  EXPECT_NE(StdioFileReader::Open(GetTestFileFullPath("car.drc")), nullptr);
  EXPECT_NE(StdioFileReader::Open(GetTestFileFullPath("cube_pc.drc")), nullptr);
}

TEST(StdioFileReaderTest, FailRead) {
  auto reader = StdioFileReader::Open(GetTestFileFullPath("car.drc"));
  ASSERT_NE(reader, nullptr);
  std::vector<char> *buffer = nullptr;
  EXPECT_FALSE(reader->ReadFileToBuffer(buffer));
}

TEST(StdioFileReaderTest, ReadFile) {
  std::vector<char> buffer;

  auto reader = StdioFileReader::Open(GetTestFileFullPath("car.drc"));
  ASSERT_NE(reader, nullptr);
  EXPECT_TRUE(reader->ReadFileToBuffer(&buffer));
  EXPECT_EQ(buffer.size(), kFileSizeCarDrc);

  reader = StdioFileReader::Open(GetTestFileFullPath("cube_pc.drc"));
  ASSERT_NE(reader, nullptr);
  EXPECT_TRUE(reader->ReadFileToBuffer(&buffer));
  EXPECT_EQ(buffer.size(), kFileSizeCubePcDrc);
}

TEST(StdioFileReaderTest, GetFileSize) {
  auto reader = StdioFileReader::Open(GetTestFileFullPath("car.drc"));
  ASSERT_EQ(reader->GetFileSize(), kFileSizeCarDrc);
  reader = StdioFileReader::Open(GetTestFileFullPath("cube_pc.drc"));
  ASSERT_EQ(reader->GetFileSize(), kFileSizeCubePcDrc);
}

}  // namespace
}  // namespace draco
