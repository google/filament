#include "draco/io/file_writer_utils.h"

#include <string>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace draco {
namespace {

TEST(FileWriterUtilsTest, SplitPathPrivateNonWindows) {
  const std::string test_path = "/path/to/file";
  std::string directory;
  std::string file;
  SplitPathPrivate(test_path, &directory, &file);
  ASSERT_EQ(directory, "/path/to");
  ASSERT_EQ(file, "file");
}

TEST(FileWriterUtilsTest, SplitPathPrivateWindows) {
  const std::string test_path = "C:\\path\\to\\file";
  std::string directory;
  std::string file;
  SplitPathPrivate(test_path, &directory, &file);
  ASSERT_EQ(directory, "C:\\path\\to");
  ASSERT_EQ(file, "file");
}

TEST(FileWriterUtilsTest, DirectoryExistsTest) {
  ASSERT_TRUE(DirectoryExists(GetTestTempDir()));
  ASSERT_FALSE(DirectoryExists("fake/test/subdir"));
}

#ifdef DRACO_TRANSCODER_SUPPORTED
TEST(FileWriterUtilsTest, CheckAndCreatePathForFileTest) {
  const std::string fake_file = "fake.file";
  const std::string fake_file_subdir = "a/few/dirs/down";
  const std::string test_temp_dir = GetTestTempDir();
  const std::string fake_file_directory =
      test_temp_dir + "/" + fake_file_subdir;
  const std::string fake_full_path =
      test_temp_dir + "/" + fake_file_subdir + "/" + fake_file;
  ASSERT_TRUE(CheckAndCreatePathForFile(fake_full_path));
  ASSERT_TRUE(DirectoryExists(fake_file_directory));
}
#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
}  // namespace draco
