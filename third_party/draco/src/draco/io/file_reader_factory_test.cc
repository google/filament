#include "draco/io/file_reader_factory.h"

#include <memory>
#include <string>
#include <vector>

#include "draco/core/draco_test_base.h"
#include "draco/io/file_reader_interface.h"

namespace draco {
namespace {

class AlwaysFailFileReader : public FileReaderInterface {
 public:
  static std::unique_ptr<FileReaderInterface> Open(
      const std::string & /*file_name*/) {
    return nullptr;
  }

  AlwaysFailFileReader() = delete;
  AlwaysFailFileReader(const AlwaysFailFileReader &) = delete;
  AlwaysFailFileReader &operator=(const AlwaysFailFileReader &) = delete;
  // Note this isn't overridden as the class can never be instantiated. This
  // avoids an unused function warning.
  // ~AlwaysFailFileReader() override = default;

  bool ReadFileToBuffer(std::vector<char> * /*buffer*/) override {
    return false;
  }

  bool ReadFileToBuffer(std::vector<uint8_t> * /*buffer*/) override {
    return false;
  }

  size_t GetFileSize() override { return 0; }

 private:
  static bool is_registered_;
};

class AlwaysOkFileReader : public FileReaderInterface {
 public:
  static std::unique_ptr<FileReaderInterface> Open(
      const std::string & /*file_name*/) {
    return std::unique_ptr<AlwaysOkFileReader>(new AlwaysOkFileReader());
  }

  AlwaysOkFileReader(const AlwaysOkFileReader &) = delete;
  AlwaysOkFileReader &operator=(const AlwaysOkFileReader &) = delete;
  ~AlwaysOkFileReader() override = default;

  bool ReadFileToBuffer(std::vector<char> * /*buffer*/) override {
    return true;
  }

  bool ReadFileToBuffer(std::vector<uint8_t> * /*buffer*/) override {
    return true;
  }

  size_t GetFileSize() override { return 0; }

 private:
  AlwaysOkFileReader() = default;
  static bool is_registered_;
};

bool AlwaysFailFileReader::is_registered_ =
    FileReaderFactory::RegisterReader(AlwaysFailFileReader::Open);

bool AlwaysOkFileReader::is_registered_ =
    FileReaderFactory::RegisterReader(AlwaysOkFileReader::Open);

TEST(FileReaderFactoryTest, RegistrationFail) {
  EXPECT_FALSE(FileReaderFactory::RegisterReader(nullptr));
}

TEST(FileReaderFactoryTest, OpenReader) {
  auto reader = FileReaderFactory::OpenReader("fake file");
  EXPECT_NE(reader, nullptr);
  std::vector<char> *buffer = nullptr;
  EXPECT_TRUE(reader->ReadFileToBuffer(buffer));
}

}  // namespace
}  // namespace draco
